#include <karm-gfx/colors.h>
#include <karm-math/align.h>
#include <karm-sys/entry.h>
#include <karm-sys/time.h>

#include "karm-gfx/icon.h"

import Karm.App;
import Karm.Kira;
import Karm.Ui;
import Mdi;

Ui::Child logo(Str text) {
    return Ui::titleLarge(text);
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
               .backgroundFill = Ui::GRAY900,
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
               .backgroundFill = Ui::GRAY900,
           });
}

Ui::Child statusWidget(Str description) {
    return Ui::labelLarge(description) | Ui::center() |
           Ui::box({
               .padding = {8},
               .borderRadii = 999,
               .backgroundFill = Ui::GRAY900,
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
               .backgroundFill = Ui::GRAY900,
           });
}

Ui::Child gameTile() {
    return Ui::empty(192) |
           Ui::box({
               .borderRadii = 6,
               .borderWidth = 1,
               .borderFill = Ui::GRAY800,
               .backgroundFill = Ui::GRAY900,
           }) |
           Ui::align(Math::Align::BOTTOM | Math::Align::START);
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

Async::Task<> entryPointAsync(Sys::Context& ctx) {
    auto app = Ui::vflow(

        Ui::hflow(
            12,
            gameTile(),
            gameTile(),
            gameTile(),
            gameTile(),
            gameTile()
        ) |
        Ui::insets(48) |
        Ui::hscroll() |
        Ui::grow()
    );

    app = Ui::vflow(
        topBar(),
        Kr::separator(),
        Ui::stack(app, false ? quickSettings() : Ui::empty()) | Ui::grow(),
        Kr::separator(),
        bottomBar()
    );

    co_return co_await Ui::runAsync(
        ctx,
        app | Ui::pinSize({640, 480})
    );
}
