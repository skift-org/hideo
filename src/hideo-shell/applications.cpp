export module Hideo.Shell:applications;

import Mdi;
import Karm.Kira;
import Karm.Ui;
import Hideo.Keyboard;
import Karm.Gfx;
import Karm.Math;

import :model;

using namespace Karm;

namespace Hideo::Shell {

Ui::Child appIcon(Gfx::Icon const& icon, Gfx::ColorRamp ramp, isize size = 22) {
    return Ui::icon(icon, size) |
           Ui::insets(size / 2.75) |
           Ui::center() |
           Ui::box({
               .borderRadii = size * 0.25,
               .borderWidth = 1,
               .borderFill = ramp[5],
               .backgroundFill = ramp[6],
               .foregroundFill = ramp[1],
           });
}

Ui::Child appRow(Launcher const& manifest, usize i) {
    return Ui::ButtonStyle::subtle(),
           Ui::hflow(
               12,
               Math::Align::START | Math::Align::VCENTER,
               appIcon(manifest.icon, manifest.ramp),
               Ui::labelLarge(manifest.name)
           ) |
               Ui::insets(6) |
               Ui::button(Model::bind<StartApplication>(i), Ui::ButtonStyle::subtle());
}

Ui::Child appsList(State const& state) {
    return Ui::vflow(
        iter(state.launchers)
            .mapi([](auto& man, usize i) {
                return appRow(*man, i);
            })
            .collect<Ui::Children>()
    );
}

Ui::Child runningApp(Rc<Window> instance) {
    return Ui::stack(
               Ui::image(instance->surface()) |
                   Ui::box({
                       .borderWidth = 1,
                       .borderFill = Ui::GRAY800,
                   }) |
                   Ui::button(Model::bind<FocusWindow>(instance)),
               Ui::button(Model::bind<RemoveWindow>(instance), Ui::ButtonStyle::secondary(), Mdi::CLOSE) |
                   Ui::align(Math::Align::TOP_END) |
                   Ui::insets({6, 6, 0, 0})
           ) |
           Ui::pinSize({120, 192});
}

Ui::Child runningApps(State const& state) {
    if (state.keyboard)
        return Ui::empty();

    if (state.windows.len() == 0)
        return Ui::empty(64);

    return Ui::hflow(
               8,
               iter(state.windows)
                   .map([](auto& instance) {
                       return runningApp(instance);
                   })
                   .collect<Ui::Children>()
           ) |
           Ui::center() | Ui::insets({64, 0, 16, 0});
}

export Ui::Child apps(State const& state) {
    return Ui::vflow(
        Kr::searchbar(""s),
        appsList(state) |
            Ui::insets({12, 0}) | Ui::vscroll() | Ui::grow()
    );
}

export Ui::Child appsFlyout(State const& state) {
    return Ui::vflow(
               runningApps(state),
               Ui::vflow(
                   Kr::dragHandle(),
                   apps(state) | Ui::grow()
               ) |
                   Ui::box({
                       .margin = 8,
                       .padding = {0, 12},
                       .borderRadii = 8,
                       .borderWidth = 1,
                       .borderFill = Ui::GRAY800,
                       .backgroundFill = Ui::GRAY950,
                   }) |
                   Ui::bound() |
                   Ui::dismisable(
                       Model::bind<ActivatePanel>(Panel::NIL),
                       Ui::DismisDir::DOWN,
                       0.3
                   ) |
                   Ui::slideIn(Ui::SlideFrom::BOTTOM) | Ui::grow()
           );
}

} // namespace Hideo::Shell
