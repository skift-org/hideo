export module Hideo.Shell:desktop;

import Karm.Ui;
import Karm.App;
import Karm.Gfx;
import Karm.Math;

import :model;
import :applications;
import :settings;
import :background;
import :taskbar;

namespace Hideo::Shell {

auto desktopPanel(Math::Vec2i size = {500, 400}) {
    return [=](Ui::Child child) {
        return child |
               Ui::pinSize(size) |
               Ui::box({
                   .padding = 8,
                   .borderRadii = 12,
                   .borderWidth = 1,
                   .borderFill = Ui::GRAY800,
                   .backgroundFill = Ui::GRAY950,
               });
    };
}

Ui::Child desktopStack(State const& state) {
    Ui::Children apps;
    bool topLevel = true;
    for (auto& window : state.windows) {
        Ui::Child node = makeRc<Viewport>(window, true, window->preferSnap, state.hasFullWindow() ? 0 : 8);

        if (window->preferSnap == App::Snap::NONE) {
            node = node |
                   Ui::box({
                       .borderRadii = 8,
                       .borderWidth = 1.,
                       .borderFill = Ui::GRAY800,
                       .shadowStyle = Gfx::BoxShadow::elevated(topLevel ? 16 : 4).withFillCenter(false),
                   }) |
                   Ui::placed(window->_floatingBound);
        }

        apps.pushFront(node);
        topLevel = false;
    }

    return Ui::stack(apps);
}

Ui::Child notificationPanel(State const& state) {
    return Ui::vflow(
               8,
               Ui::labelMedium("Notifications") |
                   Ui::insets({6, 0, 0, 12}),
               notifications(state) | Ui::grow()
           ) |
           desktopPanel({500, 400});
}

Ui::Child settingsPanel(State const& state) {
    return expendedQuickSettings(state) |
           desktopPanel({320, Ui::UNCONSTRAINED});
}

Ui::Child desktopPanels(State const& s) {
    return Ui::stack(
               s.activePanel == Panel::APPS
                   ? appsLauncher(s) |
                         Ui::center()
                   : Ui::empty(),
               s.activePanel == Panel::NOTIS
                   ? notificationPanel(s) |
                         Ui::align(Math::Align::HCENTER | Math::Align::TOP) |
                         Ui::slideIn(Ui::SlideFrom::TOP)
                   : Ui::empty(),
               s.activePanel == Panel::SYS
                   ? settingsPanel(s) |
                         Ui::align(Math::Align::END | Math::Align::TOP) |
                         Ui::slideIn(Ui::SlideFrom::TOP)
                   : Ui::empty()
           ) |
           Ui::insets(4);
}

Ui::Child desktopScreen(State const& state) {
    return Ui::stack(
        background(state) |
            Kr::contextMenu([] {
                return Kr::contextMenuContent({
                    Kr::contextMenuItem(Ui::SINK<>, Mdi::PALETTE, "Personalize..."),
                    Kr::separator(),
                    Kr::contextMenuItem(Ui::SINK<>, Mdi::COG, "Settings"),
                });
            }),
        Ui::vflow(
            taskbar(state) | Ui::slideIn(Ui::SlideFrom::TOP),
            Ui::stack(desktopStack(state), desktopPanels(state)) | Ui::clip() |
                Ui::grow()
        )
    );
}

} // namespace Hideo::Shell
