export module Hideo.Shell:taskbar;

import Mdi;
import Karm.Ui;
import Karm.Core;
import Karm.Kira;
import Karm.Gfx;
import Karm.Math;

import :model;

using namespace Karm;

namespace Hideo::Shell {

Ui::Child taskbarSearchButton() {
    return Ui::button(
               Model::bind<ActivatePanel>(Panel::APPS),
               Ui::ButtonStyle::subtle().withRadii(99),
               Mdi::MAGNIFY,
               "Search…"
           ) |
           Ui::minSize({180, Ui::UNCONSTRAINED});
}

Ui::Child taskbarCalendarButton(State const& s) {
    auto [date, time] = s.dateTime;

    auto dateTime = Io::format(
        "{}. {} {}, {:02}:{:02}",
        Io::toCapitalCase(date.month.abbr()),
        date.dayOfMonth() + 1,
        date.year.val(),
        time.hour,
        time.minute
    );

    return Ui::button(
        Model::bind<ActivatePanel>(Panel::NOTIS),
        Ui::ButtonStyle::subtle().withRadii(99),
        dateTime
    );
}

Ui::Child taskbarStatusButton() {
    return Ui::button(
        Model::bind<ActivatePanel>(Panel::SYS),
        Ui::ButtonStyle::subtle().withRadii(99),
        Ui::hflow(
            6,
            Math::Align::CENTER,
            Ui::icon(Mdi::WIFI_STRENGTH_4),
            Ui::icon(Mdi::VOLUME_HIGH),
            Ui::icon(Mdi::BATTERY),
            Ui::labelMedium("100%")
        ) |

            Ui::center() |
            Ui::insets({0, 12}) |
            Ui::bound()
    );
}

Ui::Child taskbar(State const& s) {
    return Ui::stack(
               Ui::hflow(6, taskbarSearchButton(), Ui::grow(NONE), taskbarStatusButton()), taskbarCalendarButton(s) | Ui::center()
           ) |
           Ui::box({
               .padding = 4,
               .backgroundFill = Gfx::BLACK,
           });
}

} // namespace Hideo::Shell
