module;

#include <karm-gfx/filters.h>
#include <karm-math/align.h>

export module Hideo.Avplayer;

import Mdi;
import Karm.Ui;
import Karm.Kira;
import Karm.Image;
import Karm.Ref;
import Karm.Av;

using namespace Karm;

namespace Hideo::Avplayer {

struct State {
    Rc<Av::Player> player;
};

export struct TogglePause {};

using Action = Union<TogglePause>;

Ui::Task<Action> reduce(State& s, Action a) {
    a.visit(Visitor{
        [&](TogglePause) {
            s.player->pause(not s.player->pause());
        },
    });

    return NONE;
}

export using Model = Ui::Model<State, Action, reduce>;

Ui::Child videoContent() {
    return Ui::image("bundle://hideo-avplayer/images/bunny.qoi"_url) |
           Ui::cover() |
           Ui::vhclip();
}

Ui::Child audioContent() {
    auto cover = Image::load("bundle://hideo-avplayer/images/cover.png"_url).unwrap();
    return Ui::stack(
               Ui::image(cover) |
                   Ui::foregroundFilter(Gfx::FilterChain{
                       .filters = {
                           makeBox<Gfx::Filter>(Gfx::BlurFilter{16}),
                           makeBox<Gfx::Filter>(Gfx::BrightnessFilter{0.5}),
                       },
                   }) |
                   Ui::cover(),
               Ui::image(cover, 8) | Ui::pinSize(256) | Ui::center()
           ) |
           Ui::vhclip();
}

Ui::Child player(State const& s) {
    auto mediaContent = audioContent();

    auto mediaControls =
        Ui::hflow(
            6,
            Math::Align::VCENTER | Math::Align::HFILL | Math::Align::TOP_START,
            Ui::hflow(
                Ui::button(Ui::SINK<>, Ui::ButtonStyle::subtle(), Mdi::SKIP_PREVIOUS),
                Kr::separator(),
                Ui::button(Model::bind<TogglePause>(), Ui::ButtonStyle::subtle(), s.player->pause() ? Mdi::PLAY : Mdi::PAUSE),
                Kr::separator(),
                Ui::button(Ui::SINK<>, Ui::ButtonStyle::subtle(), Mdi::SKIP_NEXT)
            ) | Ui::box({
                    .borderRadii = 4,
                    .borderFill = Ui::GRAY700,
                    .backgroundFill = Ui::GRAY800,
                }),
            Ui::empty(4),
            Ui::labelMedium("{}", s.player->tell()),
            Kr::slider(s.player->tell().toMSecs() / static_cast<f64>(s.player->duration().toMSecs()), NONE) |
                Ui::grow(),
            Ui::labelMedium("{}", s.player->duration()),
            Ui::empty(4),
            Ui::button(Ui::SINK<>, Ui::ButtonStyle::regular(), Mdi::VOLUME_HIGH),
            Ui::button(Ui::SINK<>, Ui::ButtonStyle::regular(), Mdi::FULLSCREEN),
            Ui::button(Ui::SINK<>, Ui::ButtonStyle::regular(), Mdi::COG)
        ) |
        Ui::insets(8) |
        Ui::box({
            .backgroundFill = Ui::GRAY900.withOpacity(0.6),
        }) |
        Ui::backgroundFilter(Gfx::BlurFilter{16});

    return Ui::stack(
               mediaContent,
               Ui::vflow(
                   Ui::grow(NONE),
                   mediaControls
               )
           ) |
           Ui::grow();
}

export Ui::Child app(Rc<Av::Player> p) {
    return Ui::reducer<Model>(State{p}, [](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::PLAY_CIRCLE,
            .title = "Media Player"s,
            .body = [&] {
                return player(s);
            },
        });
    });
}

} // namespace Hideo::Avplayer
