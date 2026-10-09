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

using namespace Karm::Math::Literals;

namespace Hideo::Shell {

auto desktopPanel(Math::Vec2Au size = {500_au, 400_au}) {
    return [=](Ui::Child child) {
        return child |
               Ui::pinSize(size) |
               Ui::box({
                   .padding = 8_au,
                   .borderRadii = 12,
                   .borderWidth = 1,
                   .borderFill = Some(Ui::GRAY800),
                   .backgroundFill = Some(Ui::GRAY950),
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
                       .borderFill = Some(Ui::GRAY800),
                       .shadowStyle = Some(Gfx::BoxShadow::elevated(topLevel ? 16 : 4).withSkipOccluded(true)),
                   }) |
                   Ui::placed(window->_floatingBound.cast<Math::Au>());
        }

        apps.pushFront(node);
        topLevel = false;
    }

    return Ui::stack(apps);
}

Ui::Child notificationPanel(State const& state) {
    return Ui::vflow(
               8_au,
               Ui::labelMedium("Notifications") |
                   Ui::insets({6_au, 0_au, 0_au, 12_au}),
               notifications(state) | Ui::grow()
           ) |
           desktopPanel({500_au, 400_au});
}

Ui::Child settingsPanel(State const& state) {
    return expendedQuickSettings(state) |
           desktopPanel({320_au, Ui::UNCONSTRAINED});
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
           Ui::insets(4_au);
}

Ui::Child desktopScreen(State const& state) {
    return Ui::stack(
        background(state) |
            Kr::contextMenu([] {
                return Kr::contextMenuContent({
                    Kr::contextMenuItem(Some(Ui::SINK<>), Some(Mdi::PALETTE), "Personalize..."),
                    Kr::separator(),
                    Kr::contextMenuItem(Some(Ui::SINK<>), Some(Mdi::COG), "Settings"),
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
