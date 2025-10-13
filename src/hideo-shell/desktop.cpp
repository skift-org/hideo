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

auto panel(Math::Vec2i size = {500, 400}) {
    return [=](Ui::Child child) {
        return child |
               Ui::pinSize(size) |
               Ui::box({
                   .padding = 8,
                   .borderRadii = 8,
                   .borderWidth = 1,
                   .borderFill = Ui::GRAY800,
                   .backgroundFill = Ui::GRAY950,
               });
    };
}

Ui::Child appStack(State const& state) {
    Ui::Children apps;
    bool topLevel = true;
    for (auto& i : iterRev(state.instances)) {
        apps.pushBack(
            makeRc<Viewport>(i, 8) |
            Ui::box({
                .borderRadii = 8,
                .borderWidth = 1,
                .borderFill = Ui::GRAY800,
                .shadowStyle = Gfx::BoxShadow::elevated(topLevel ? 16 : 4),
            }) |
            Ui::placed(i->bound)
        );
        topLevel = false;
    }

    return Ui::stack(apps);
}

Ui::Child applicationsPanel(State const& s) {
    return apps(s) | panel();
}

Ui::Child notificationPanel(State const& state) {
    return Ui::vflow(
               8,
               Ui::labelMedium("Notifications") |
                   Ui::insets({6, 0, 0, 12}),
               notifications(state) | Ui::grow()
           ) |
           panel({500, 400});
}

Ui::Child settingsPanel(State const& state) {
    return expendedQuickSettings(state) |
           panel({320, Ui::UNCONSTRAINED});
}

Ui::Child desktopPanels(State const& s) {
    return Ui::stack(
               s.activePanel == Panel::APPS
                   ? applicationsPanel(s) |
                         Ui::align(Math::Align::START | Math::Align::TOP) |
                         Ui::slideIn(Ui::SlideFrom::TOP)
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
           Ui::insets({38, 8});
}

Ui::Child desktop(State const& state) {
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
            taskbar(state) | Ui::slideIn(Ui::SlideFrom::TOP)
        ),
        appStack(state) |
            Ui::grow()
    );
}

} // namespace Hideo::Shell
