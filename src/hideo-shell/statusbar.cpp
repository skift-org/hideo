export module Hideo.Shell:statusbar;

import Mdi;
import Karm.Ui;
import Karm.Kira;
import Karm.Gfx;

import :model;

using namespace Karm;

namespace Hideo::Shell {

Ui::Child statusbarClock(State const& s) {
    auto [_, time] = s.dateTime;
    return Ui::labelLarge("{:02}:{:02}", time.hour, time.minute) | Ui::center();
};

Ui::Child statusbarIndicator(Gfx::Icon icon) {
    return Ui::icon(icon) |
           Ui::center() |
           Ui::insets(4);
}

Ui::Child statusbarBatteryIndicator() {
    return Ui::hflow(
        statusbarIndicator(Mdi::BATTERY),
        Ui::labelLarge("100%") | Ui::center()
    );
}

export Ui::Child statusbar(State const& s) {
    return Ui::hflow(
               statusbarClock(s),
               Ui::grow(NONE),
               statusbarIndicator(Mdi::WIFI_STRENGTH_4),
               statusbarIndicator(Mdi::NETWORK_STRENGTH_4),
               statusbarBatteryIndicator()
           ) |
           Ui::minSize({
               Ui::UNCONSTRAINED,
               32,
           }) |
           Ui::box({
               .padding = {0, 12},
               .backgroundFill = Some(s.hasFullWindow() ? Ui::GRAY950 : Ui::GRAY950.withOpacity(0.6)),
           });
}

export Ui::Child statusbarButton(State const& s) {
    return statusbar(s) | 
        Ui::button(
            Some(Model::bind<ActivatePanel>(Panel::SYS)),
            Ui::ButtonStyle::none()
        );
}

} // namespace Hideo::Shell
