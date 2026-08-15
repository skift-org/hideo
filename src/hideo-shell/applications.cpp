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
               .borderFill = Some(ramp[5]),
               .backgroundFill = Some(ramp[6]),
               .foregroundFill = ramp[1],
           });
}

Ui::Child appRow(Rc<Launcher> launcher, bool selected) {
    auto child = Ui::hflow(
                     12,
                     Math::Align::START | Math::Align::VCENTER,
                     appIcon(launcher->icon, launcher->ramp, 18),
                     Ui::labelMedium(launcher->name)
                 ) |
                 Ui::insets(6) |
                 Ui::button(
                     Some(Model::bind<StartApplication>(launcher)),
                     selected ? Ui::ButtonStyle::regular() : Ui::ButtonStyle::subtle()
                 );

    if (selected) {
        child |= Ui::keyboardShortcut(App::Key::ENTER);
        child |= Ui::scrollToMe(8);
    }

    return child;
}

Ui::Child appsList(State const& state) {
    if (state.filtered.len() == 0)
        return Kr::errorPageContent({
            Kr::errorPageSubTitle("No result found matching your query."s),
        });

    return Ui::vflow(
        6,
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
               iter(state.windows) |
                   Select([](auto& instance) {
                       return runningApp(instance);
                   }) |
                   Collect<Ui::Children>()
           ) |
           Ui::center() |
           Ui::insets({64, 0, 16, 0});
}

export Ui::Child appsSearchbar(State const& s) {
    return Ui::hflow(
        8,
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
                   Ui::insets({18, 18}),
               Kr::separator(),
               appsList(s) |
                   Ui::insets(12) | Ui::vscroll() | Ui::grow()
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
           Ui::pinSize({500, 400}) | Ui::focusable({.visual = false, .steal = true});
}

export Ui::Child appsFlyout(State const& state) {
    return Ui::vflow(
        runningApps(state),
        Ui::vflow(
            Kr::dragHandle(),
            appsContent(state) | Ui::grow()
        ) |
            Ui::box({
                .margin = 8,
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
