module;

#include <karm-math/align.h>

export module Hideo.Avplayer;

import Mdi;
import Karm.Ui;
import Karm.Kira;
import Karm.Image;
import Karm.Ref;
import Karm.Av;
import Karm.App;
import Karm.Sys;
import Karm.Gfx;

export import :model;

using namespace Karm;

namespace Hideo::Avplayer {

Ui::Child videoContent() {
    return Ui::image("bundle://hideo-avplayer/images/bunny.qoi"_url) |
           Ui::cover() |
           Ui::vhclip();
}

Ui::Child audioContent() {
    auto image = Image::load("bundle://hideo-avplayer/images/missing.png"_url).unwrap();
    auto background = Ui::image(image) |
                      Ui::foregroundFilter(Gfx::BlurFilter{8}) |
                      Ui::foregroundFilter(Gfx::BrightnessFilter{0.2}) |
                      Ui::cover();

    auto cover = Ui::image(
                     image,
                     8
                 ) |
                 Ui::box({
                     .borderRadii = 8,
                     .borderWidth = 2,
                     .borderFill = Ui::GRAY100.withOpacity(0.1),
                 }) |
                 Ui::pinSize(256);

    return Ui::stack(
               background,
               Ui::vflow(
                   4,
                   Math::Align::CENTER,
                   cover,
                   Ui::empty(4),
                   Ui::titleLarge("Unknown Title"),
                   Ui::labelMedium("Unknown Artist")
               ) | Ui::center()
           ) |
           Ui::vhclip();
}

Ui::Child nomedia(Error err) {
    return Kr::errorPage(Mdi::ALERT_CIRCLE_OUTLINE, "Could not start media playback"s, Str{err.msg()});
}

Ui::Child transportControls(State const& s) {
    return Ui::hflow(
               Ui::button(Model::bind<Previous>(), Ui::ButtonStyle::subtle(), Mdi::SKIP_PREVIOUS),
               Kr::separator(),
               Ui::button(Model::bind<TogglePause>(), Ui::ButtonStyle::primary(), s.player->status() == Av::Player::PLAYING ? Mdi::PAUSE : Mdi::PLAY),
               Kr::separator(),
               Ui::button(Ui::SINK<>, Ui::ButtonStyle::subtle(), Mdi::SKIP_NEXT)
           ) |
           Ui::box({
               .borderRadii = 4,
               .backgroundFill = Ui::GRAY800,
           });
}

Ui::Child duration(Duration dur) {
    return Ui::text(Ui::TextStyles::codeSmall().withColor(Ui::GRAY300), "{:02}:{:02}", dur.toMinutes(), dur.toSecs() % 60);
}

Ui::Child scrubberControls(State const& s) {
    return Ui::hflow(
        6,
        Math::Align::VCENTER | Math::Align::HFILL | Math::Align::TOP_START,
        duration(s.player->tell()),
        Kr::slider(
            s.player->tell().toMSecs() / static_cast<f64>(s.player->duration().toMSecs()),
            [&](auto& n, f64 v) {
                auto durr = Duration::fromMSecs(s.player->duration().toMSecs() * v);
                Model::bubble<Scrub>(n, Scrub{durr});
            }
        ) |
            Ui::grow(),
        duration(s.player->duration())
    );
}

Ui::Child volumeControls(State const& s) {
    return Ui::hflow(
               0,
               Math::Align::VCENTER | Math::Align::HFILL | Math::Align::TOP_START,
               Ui::button(
                   Model::bind<ToggleMute>(),
                   Ui::ButtonStyle::subtle(),
                   s.player->mute() ? Mdi::VOLUME_MUTE : Mdi::VOLUME_HIGH
               ),
               Kr::slider(s.player->volume(), [](auto& n, f64 v) {
                   Model::bubble<ChangeVolume>(n, ChangeVolume{v});
               })
           ) |
           Ui::box({
               .padding = {0, 6, 0, 0},
               .borderRadii = 4,
               .backgroundFill = Ui::GRAY800,
           });
}

Ui::Child mediaControls(State const& s) {
    return Ui::hflow(
               6,
               Math::Align::VCENTER | Math::Align::HFILL | Math::Align::TOP_START,
               transportControls(s),
               Ui::empty(4),
               scrubberControls(s) | Ui::grow(),
               Ui::empty(4),
               volumeControls(s),
               Ui::button(Ui::bindBubble<App::RequestMaximizeEvent>(), Ui::ButtonStyle::regular(), Mdi::FULLSCREEN)
           ) |
           Ui::insets(8);
}

Ui::Child player(State const& s) {
    return Ui::stack(
               s.audio ? audioContent() : nomedia(s.audio.none()),
               Ui::vflow(
                   Ui::grow(NONE),
                   mediaControls(s)
               )
           ) |
           Ui::grow();
}

export Ui::Child app(Rc<Av::Player> p, Res<Rc<Av::Audio>> a) {
    return Ui::reducer<Model>(State{p, a}, [](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::PLAY_CIRCLE,
            .title = "Media Player"s,
            .body = [&] {
                return player(s) |
                       Ui::keyboardShortcut(App::Key::SPACE, Model::bind<TogglePause>()) |
                       Ui::keyboardShortcut(App::Key::LEFT, [&](auto& n) {
                           auto curr = s.player->tell();
                           Model::bubble<Scrub>(n, Scrub{s.player->tell() - Duration::fromSecs(min(5uz, curr.toSecs()))});
                       }) |
                       Ui::keyboardShortcut(App::Key::RIGHT, [&](auto& n) {
                           Model::bubble<Scrub>(n, Scrub{s.player->tell() + Duration::fromSecs(5)});
                       }) |
                       Ui::keyboardShortcut(App::Key::DOWN, [&](auto& n) {
                           Model::bubble<ChangeVolume>(n, ChangeVolume{s.player->volume() - 0.1});
                       }) |
                       Ui::keyboardShortcut(App::Key::UP, [&](auto& n) {
                           Model::bubble<ChangeVolume>(n, ChangeVolume{s.player->volume() + 0.1});
                       });
            },
        });
    });
}

export Async::Task<> updatePlayback(Ui::Child app, Async::Ct ct) {
    while (not ct.canceled()) {
        Model::event<Update>(*app);
        co_trya$(Sys::globalSched().sleepAsync(Sys::instant() + Duration::fromSecs(1)));
    }
    co_return Ok();
}

} // namespace Hideo::Avplayer
