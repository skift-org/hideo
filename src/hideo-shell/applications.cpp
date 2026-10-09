export module Hideo.Shell:applications;

import Mdi;
import Karm.Kira;
import Karm.Ui;
import Hideo.Keyboard;
import Karm.Gfx;
import Karm.Math;

import :model;

using namespace Karm;
using namespace Karm::Math::Literals;

namespace Hideo::Shell {

Ui::Child appIcon(Gfx::Icon const& icon, Gfx::ColorRamp ramp, Math::Au size = 22_au) {
    return Ui::icon(icon, size) |
           Ui::insets(size / 2.75) |
           Ui::center() |
           Ui::box({
               .borderRadii = size.cast<f64>() * 0.25,
               .borderWidth = 1,
               .borderFill = Some(ramp[5]),
               .backgroundFill = Some(ramp[6]),
               .foregroundFill = ramp[1],
           });
}

Ui::Child appRow(Rc<Launcher> launcher, bool selected) {
    auto child = Ui::hflow(
                     12_au,
                     Math::Align::START | Math::Align::VCENTER,
                     appIcon(launcher->icon, launcher->ramp, 18_au),
                     Ui::labelMedium(launcher->name)
                 ) |
                 Ui::insets(6_au) |
                 Ui::button(
                     Some(Model::bind<StartApplication>(launcher)),
                     selected ? Ui::ButtonStyle::regular() : Ui::ButtonStyle::subtle()
                 );

    if (selected) {
        child |= Ui::keyboardShortcut(App::Key::ENTER);
        child |= Ui::scrollToMe(8_au);
    }

    return child;
}

Ui::Child appsList(State const& state) {
    if (state.filtered.len() == 0)
        return Kr::emptyContent({
            Kr::emptySubTitle("No result found matching your query."s),
        });

    return Ui::vflow(
        6_au,
        iter(state.filtered) |
            Selecti([&](auto& man, usize i) {
                return appRow(man, i == state.searchIndex);
            }) |
            Collect<Ui::Children>()
    );
}

Ui::Child runningApp(Rc<Window> instance) {
    return Ui::stack(
               Ui::image(instance->surface()) |
                   Ui::box({
                       .borderWidth = 1,
                       .borderFill = Some(Ui::GRAY800),
                   }) |
                   Ui::button(Some(Model::bind<FocusWindow>(instance))),
               Ui::button(
                   Some(Model::bind<RemoveWindow>(instance)),
                   Ui::ButtonStyle::secondary(),
                   Mdi::CLOSE
               ) |
                   Ui::align(Math::Align::TOP_END) |
                   Ui::insets({6_au, 6_au, 0_au, 0_au})
           ) |
           Ui::pinSize({120_au, 192_au});
}

Ui::Child runningApps(State const& state) {
    if (state.keyboard)
        return Ui::empty();

    if (state.windows.len() == 0)
        return Ui::empty(64_au);

    return Ui::hflow(
               8_au,
               iter(state.windows) |
                   Select([](auto& instance) {
                       return runningApp(instance);
                   }) |
                   Collect<Ui::Children>()
           ) |
           Ui::center() |
           Ui::insets({64_au, 0_au, 16_au, 0_au});
}

export Ui::Child appsSearchbar(State const& s) {
    return Ui::hflow(
        8_au,
        Math::Align::VCENTER | Math::Align::START,
        Ui::stack(
            s.searchQuery ? Ui::empty() : Ui::labelLarge(Ui::GRAY500, "Search for anything…"),
            Ui::input(Ui::TextStyles::labelLarge(), s.searchQuery, Model::map<UpdateSearch>())
        ) | Ui::grow(),
        Ui::icon(Mdi::MAGNIFY)
    );
}

export Ui::Child appsContent(State const& s) {
    return Ui::vflow(
               appsSearchbar(s) |
                   Ui::insets({18_au, 18_au}),
               Kr::separator(),
               appsList(s) |
                   Ui::insets(12_au) | Ui::vscroll() | Ui::grow()
           ) |
           Ui::keyboardShortcut(App::Key::UP, {}, [](auto& n) {
               Model::bubble<SelectSearch>(n, {-1});
           }) |
           Ui::keyboardShortcut(App::Key::DOWN, {}, [](auto& n) {
               Model::bubble<SelectSearch>(n, {1});
           });
}

export Ui::Child appsLauncher(State const& state) {
    return appsContent(state) | Ui::bound() |
           Ui::box({
               .borderRadii = 12,
               .borderWidth = 1,
               .borderFill = Some(Ui::GRAY800),
               .backgroundFill = Some(Ui::GRAY900),
               .shadowStyle = Some(Gfx::BoxShadow::elevated(16)),
           }) |
           Ui::pinSize({500_au, 400_au}) | Ui::focusable({.visual = false, .steal = true});
}

export Ui::Child appsFlyout(State const& state) {
    return Ui::vflow(
        runningApps(state),
        Ui::vflow(
            Kr::dragHandle(),
            appsContent(state) | Ui::grow()
        ) |
            Ui::box({
                .margin = 8_au,
                .borderRadii = 8,
                .borderWidth = 1,
                .borderFill = Some(Ui::GRAY800),
                .backgroundFill = Some(Ui::GRAY950),
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
