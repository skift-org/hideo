#include <karm-gfx/colors.h>
#include <karm-math/align.h>
#include <karm-sys/entry.h>
#include <karm-sys/time.h>

#include "karm-gfx/icon.h"

import Karm.App;
import Karm.Kira;
import Karm.Ui;
import Karm.Image;
import Mdi;

namespace Ottercat::Shell {

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

Ui::Child logo(Str text) {
    return Ui::titleLarge(text) | Ui::hcenter();
}

Ui::Child buttonHint(Str button, Str description, Gfx::Color color) {
    return Ui::hflow(
               12,
               Ui::labelLarge(button) | Ui::center() | Ui::minSize(22) |
                   Ui::box({
                       .borderRadii = 999,
                       .backgroundFill = color,
                       .foregroundFill = Ui::GRAY50,
                   }),
               Ui::labelLarge(description) | Ui::center()
           ) |
           Ui::box({
               .padding = {8, 16, 8, 8},
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
               .backgroundFill = Ui::GRAY900,
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
                   12,
                   tileContent(tileGameCover("bundle://hideo-handheld/tiles/celeste.qoi"_url)),
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
                       Kr::titleRow("Quick Settings"s),
                       Kr::treeRow(
                           NONE,
                           "General"s,
                           NONE,
                           Ui::Slots{[] -> Ui::Children {
                               return {
                                   Kr::sliderRow(0.5, Ui::SINK<f64>, "Brightness"s),
                                   Kr::sliderRow(0.5, Ui::SINK<f64>, "Volume"s),
                               };
                           }}
                       ),

                       Kr::toggleRow(true, Ui::SINK<bool>, "NFC"s), Kr::toggleRow(true, Ui::SINK<bool>, "Spotpass"s)
                   ) | Ui::vscroll() |
                       Ui::box({.backgroundFill = Ui::GRAY900}) | Ui::grow()
               ) | Ui::minSize(280) |
                   Ui::slideIn(Ui::SlideFrom::END)
           ) |
           Ui::box({.backgroundFill = Gfx::BLACK.withOpacity(0.4)});
}

Ui::Child topBar() {
    return Ui::hflow(
               6,
               logo("Ottercat"),
               Ui::grow(NONE),
               statusWidget(Mdi::SUN_WIRELESS),
               statusWidget(Mdi::BATTERY_50, "50%"),
               statusWidget("11:29")
           ) |
           Ui::insets(12);
}

Ui::Child bottomBar() {
    return Ui::hflow(
               6,
               buttonHint("  MENU  ", "Options", Ui::ACCENT700),
               Ui::grow(NONE),
               buttonHint("A", "Select", Ui::ACCENT700),
               buttonHint("B", "Back", Ui::GRAY700)
           ) |
           Ui::insets(8);
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
                       bottomBar() |
                           Ui::box({
                               .backgroundFill = Ui::GRAY950.withOpacity(0.7),
                           }) |
                           Ui::backgroundFilter(Gfx::BlurFilter{16})
                   )
               ) |
               Ui::keyboardShortcut(App::Key::M, Model::bind<State::QuickMenuToggle>());
    });
}

} // namespace Ottercat::Shell

Async::Task<> entryPointAsync(Sys::Context& ctx) {

    co_return co_await Ui::runAsync(
        ctx,
        Ottercat::Shell::app() | Ui::pinSize({640, 480})
    );
}
