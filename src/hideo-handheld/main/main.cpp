#include <karm-gfx/colors.h>
#include <karm-gfx/icon.h>
#include <karm-math/align.h>
#include <karm-sys/entry.h>
#include <karm-sys/time.h>
#include <karm-text/font.h>

#include "karm-text/loader.h"
#include "karm-text/prose.h"

import Karm.App;
import Karm.Kira;
import Karm.Ui;
import Karm.Image;
import Mdi;

namespace Hideo::Handheld {

struct State {
    bool quickMenuVisible;

    struct QuickMenuToggle {};

    using Action = Union<QuickMenuToggle>;

    Ui::Task<Action> reduce(Action const& action) {
        return action.visit(Visitor{
            [&](QuickMenuToggle) {
                quickMenuVisible = not quickMenuVisible;
                return NONE;
            },
        });
    }
};

using Model = Ui::Model<State, State::Action>;

static Opt<Rc<Text::Fontface>> _iputFontface = NONE;

Rc<Text::Fontface> inputFontface() {
    if (not _iputFontface) {
        _iputFontface = Text::loadFontfaceOrFallback("bundle://hideo-handheld/fonts/BPreplayBold.ttf"_url).unwrap();
    }
    return *_iputFontface;
}

Text::ProseStyle inputMedium() {
    return {
        .font = Text::Font{
            inputFontface(),
            16,
        },
    };
}

Ui::Child logo(Str text) {
    return Ui::titleMedium(text) | Ui::vcenter();
}

Ui::Child buttonHint(Str button, Str description, Gfx::Color color) {
    return Ui::hflow(
               Ui::text(inputMedium(), button) | Ui::center() | Ui::minSize(26) |
                   Ui::box({
                       .margin = 4,
                       .borderRadii = 999,
                       .backgroundFill = color,
                       .foregroundFill = color.luminance() > 0.5 ? Ui::GRAY800 : Ui::GRAY50,
                   }),
               Ui::labelLarge(description) | Ui::center() | Ui::insets({0, 12, 0, 2})
           ) |
           Ui::box({
               .borderRadii = 999,
               .backgroundFill = Ui::GRAY800,
           });
}

Ui::Child statusWidget(Gfx::Icon icon) {
    return Ui::hflow(
               12,
               Ui::icon(icon) | Ui::center() | Ui::minSize(22)
           ) |
           Ui::box({
               .padding = 8,
               .borderRadii = 999,
               .backgroundFill = Ui::GRAY800,
           });
}

Ui::Child statusWidget(Str description) {
    return Ui::labelLarge(description) | Ui::center() |
           Ui::box({
               .padding = {8, 16},
               .borderRadii = 999,
               .backgroundFill = Ui::GRAY800,
           });
}

Ui::Child statusWidget(Gfx::Icon icon, Str description) {
    return Ui::hflow(
               6,
               Ui::icon(icon) | Ui::center() | Ui::minSize(22), Ui::labelLarge(description) | Ui::center()
           ) |
           Ui::box({
               .padding = {8, 16, 8, 8},
               .borderRadii = 999,
               .backgroundFill = Ui::GRAY800,
           });
}

Ui::Child tileGameCover(Mime::Url image) {
    return Ui::image(image, 6);
}

Ui::Child tileAppCover(Gfx::Icon icon, Gfx::ColorRamp ramp) {
    return Ui::icon(icon, 96) |
           Ui::center() | Ui::bound() |
           Ui::box({
               .borderRadii = 6,
               .backgroundFill = ramp[6],
               .foregroundFill = ramp[1],
           });
}

Ui::Child tileContent(Ui::Child child) {
    return child |
           Ui::pinSize(192) |
           Ui::box({
               .borderRadii = 6,
               .borderWidth = 1,
               .borderFill = Ui::GRAY50.withOpacity(0.4),
           }) |
           Ui::focusable() |
           Ui::align(Math::Align::BOTTOM | Math::Align::START);
}

Ui::Child tileList() {
    return Ui::vflow(
               Ui::grow(NONE),
               Ui::headlineMedium("Celeste") |
                   Ui::insets({0, 48, 16}),
               Ui::hflow(
                   26,
                   tileContent(tileGameCover("bundle://hideo-handheld/tiles/celeste.qoi"_url)) | Ui::scaleIn(),
                   tileContent(tileGameCover("bundle://hideo-handheld/tiles/doom.qoi"_url)),
                   tileContent(tileGameCover("bundle://hideo-handheld/tiles/minicraft.qoi"_url)),
                   tileContent(tileGameCover("bundle://hideo-handheld/tiles/vvvvvv.qoi"_url)),
                   tileContent(tileAppCover(Mdi::FOLDER, Gfx::EMERALD_RAMP)),
                   Kr::separator(),
                   tileContent(tileAppCover(Mdi::APPS, Gfx::ZINC_RAMP))
               ) | Ui::insets({0, 48, 48}) |
                   Ui::hscroll()
           ) |
           Ui::grow();
}

Ui::Child quickSettings() {
    return Ui::hflow(
               Ui::grow(NONE),
               Ui::hflow(
                   Kr::separator(),
                   Ui::vflow(
                       Kr::titleRow("Sound"s),
                       Kr::sliderRow(0.5, Ui::SINK<f64>, "Volume"s),

                       Kr::separator(),
                       Kr::titleRow("Display"s),
                       Kr::sliderRow(0.5, Ui::SINK<f64>, "Brightness"s),
                       Kr::toggleRow(true, Ui::SINK<bool>, "Night Mode"s),

                       Kr::separator(),
                       Kr::titleRow("Wireless"s),
                       Kr::toggleRow(true, Ui::SINK<bool>, "NFC"s),
                       Kr::toggleRow(true, Ui::SINK<bool>, "RaftShare™"s),
                       Kr::rowContent(Ui::text(Ui::TextStyles::bodyMedium().withColor(Ui::GRAY400), "RaftShare™ keeps consoles together like otters in a raft, letting you share games and apps with the ones around you, wirelessly.")),

                       Kr::separator(),
                       Kr::titleRow("Appearance"s),
                       Kr::toggleRow(true, Ui::SINK<bool>, "Dark Mode"s),

                       Kr::separator(),
                       Kr::titleRow("About"s),
                       Kr::rowContent(
                           Ui::button(
                               [](auto& n) {
                                   Ui::showDialog(n, Kr::aboutDialog("Ottercat"s));
                               },
                               "About Ottercat"
                           )
                       )
                   ) | Ui::vscroll() |
                       Ui::box({.backgroundFill = Ui::GRAY900}) | Ui::grow()
               ) | Ui::pinSize(280) |
                   Ui::slideIn(Ui::SlideFrom::END)
           ) |
           Ui::box({.backgroundFill = Gfx::BLACK.withOpacity(0.4)});
}

Ui::Child topBar() {
    return Ui::hflow(
               6,
               logo("Applications"),
               Ui::grow(NONE),
               statusWidget(Mdi::BROADCAST),
               statusWidget(Mdi::BATTERY_50, "50%"),
               statusWidget("11:29")
           ) |
           Ui::insets(12);
}

Ui::Child bottomBar(State const& s) {
    return Ui::hflow(
               6,
               buttonHint("  MENU  ", "SETTINGS", Ui::ACCENT700),
               Ui::grow(NONE),
               buttonHint("Y", "OPTIONS", Gfx::WHITE) | Ui::cond(not s.quickMenuVisible),
               buttonHint("A", "SELECT", Gfx::WHITE),
               buttonHint("B", "BACK", Gfx::WHITE)
           ) |
           Ui::insets(8);
}

Ui::Child appMenu() {
    return Kr::contextMenuContent({
        Kr::dialogHeader({
            Ui::hflow(
                12,
                Ui::image("bundle://hideo-handheld/tiles/celeste.qoi"_url, 6) |
                    Ui::box({
                        .borderRadii = 6,
                        .borderWidth = 1,
                        .borderFill = Ui::GRAY50.withOpacity(0.4),
                    }) |
                    Ui::pinSize(64) | Ui::vcenter(),
                Ui::vflow(
                    Kr::dialogTitle("Celeste"s),
                    Kr::dialogDescription("Version: v0.1"s),
                    Kr::dialogDescription("Size: 10.5MiB"s)
                )
            ),
        }),
        Kr::separator(),
        Kr::contextMenuItem(Ui::closeDialog, Mdi::PLAY, "Open"s),
        Kr::contextMenuItem(Ui::closeDialog, Mdi::PLAYLIST_REMOVE, "Remove from recents"s),
        Kr::contextMenuItem(Ui::closeDialog, Mdi::DELETE_FOREVER, "Uninstall"s),
        Kr::separator(),
        Kr::contextMenuItem(Ui::closeDialog, Mdi::CANCEL, "Cancel"s),
    });
}

Ui::Child app() {
    return Ui::reducer<Model>({}, [](State const& s) {
        return Ui::stack(
                   Ui::image(Image::loadOrFallback("bundle://hideo-handheld/covers/celeste.qoi"_url).take()) | Ui::foregroundFilter(Gfx::OverlayFilter{Ui::GRAY950.withOpacity(0.6)}) | Ui::grow() | Ui::cover(),
                   Ui::vflow(
                       topBar() |
                           Ui::box({
                               .backgroundFill = Ui::GRAY950.withOpacity(0.7),
                           }) |
                           Ui::backgroundFilter(Gfx::BlurFilter{16}),
                       Kr::separator(),
                       Ui::stack(tileList(), s.quickMenuVisible ? quickSettings() : Ui::empty()) | Ui::grow(),
                       Kr::separator(),
                       bottomBar(s) |
                           Ui::box({
                               .backgroundFill = Ui::GRAY950.withOpacity(0.7),
                           }) |
                           Ui::backgroundFilter(Gfx::BlurFilter{16})
                   )
               ) |
               Ui::keyboardShortcut(App::Key::M, Model::bind<State::QuickMenuToggle>()) |
               Ui::keyboardShortcut(App::Key::Y, [&](auto& n) {
                   if (not s.quickMenuVisible)
                       Ui::showDialog(n, appMenu() | Ui::center());
               }) |
               Ui::dialogLayer();
    });
}

} // namespace Hideo::Handheld

Async::Task<> entryPointAsync(Sys::Context& ctx) {
    App::formFactor = App::FormFactor::MOBILE;

    co_return co_await Ui::runAsync(
        ctx,
        Hideo::Handheld::app() | Ui::pinSize({640, 480})
    );
}
