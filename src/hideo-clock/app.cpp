module;

#include <karm/macros>

export module Hideo.Clock:app;

import Mdi;
import Karm.Core;
import Karm.Kira;
import Karm.Ui;
import Karm.Sys;
import Karm.Math;

import :model;

using namespace Karm::Literals;

namespace Hideo::Clock {

// MARK: Alarm Page ------------------------------------------------------------

Ui::Child alarmCard(Time alarm, bool enabled) {
    return Ui::hflow(
               24,
               Math::Align::CENTER,
               Ui::displayMedium("{:02}:{:02}", alarm.hour, alarm.minute),
               Ui::grow(NONE),
               Kr::toggle(enabled, Ui::SINK<bool>)
           ) |
           Ui::box({
               .padding = 12,
               .borderRadii = 12,
               .backgroundFill = Some(Ui::GRAY900),
           });
}

Ui::Child alarmPage() {
    return Ui::vflow(
               12,
               alarmCard({0, 0, 8}, true),
               alarmCard({0, 0, 12}, false),
               alarmCard({0, 0, 18}, true),
               alarmCard({0, 0, 22}, false)
           ) |
           Ui::insets(12);
}

// MARK: Clock Page ------------------------------------------------------------

Ui::Child clockPage(State const& s) {
    auto time = s.dateTime.time;

    return Ui::vflow(
               12,
               Kr::clock(time) | Ui::pinSize({200, 200}),
               Ui::displayMedium("{:02}:{:02}:{:02}", time.hour, time.minute, time.second) | Ui::center()
           ) |
           Ui::insets(12);
}

// MARK: Timer Page ------------------------------------------------------------

Ui::Child timerPage() {
    return Ui::labelLarge("Timer");
}

// MARK: Stopwatch Page --------------------------------------------------------

Ui::Child stopwatchPage() {
    return Ui::labelLarge("Stopwatch");
}

// MARK: App -------------------------------------------------------------------

Ui::Child appContent(State const& s) {
    switch (s.page) {
    case Page::ALARM:
        return alarmPage();
    case Page::CLOCK:
        return clockPage(s);
    case Page::TIMER:
        return timerPage();
    case Page::STOPWATCH:
        return stopwatchPage();
    }
}

export Ui::Child app() {
    return Ui::reducer<Model>(
        [](State const& s) {
            return Kr::scaffold({
                .icon = Mdi::CLOCK,
                .title = "Clock"s,
                .middleTools = Some([&] -> Ui::Children {
                    return {
                        Kr::tabbarContent({
                            Kr::tabbarItem(s.page == Page::ALARM, Model::bind(Page::ALARM), Kr::tabarItemLabel(Some(Mdi::ALARM), "Alarm"s)),
                            Kr::tabbarItem(s.page == Page::CLOCK, Model::bind(Page::CLOCK), Kr::tabarItemLabel(Some(Mdi::CLOCK_OUTLINE), "Clock"s)),
                            Kr::tabbarItem(s.page == Page::TIMER, Model::bind(Page::TIMER), Kr::tabarItemLabel(Some(Mdi::TIMER_SAND), "Timer"s)),
                            Kr::tabbarItem(s.page == Page::STOPWATCH, Model::bind(Page::STOPWATCH), Kr::tabarItemLabel(Some(Mdi::TIMER_OUTLINE), "Stopwatch"s)),
                        }) |
                        Ui::center() | Ui::grow()
                    };
                }),
                .body = [&] {
                    return Ui::vflow(
                               Ui::hflow(
                                   0,
                                   Math::Align::CENTER,
                                   Ui::titleLarge(toStr(s.page)),
                                   Ui::grow(NONE),
                                   Ui::button(
                                       Some(Ui::SINK<>),
                                       Ui::ButtonStyle::subtle(),
                                       Mdi::DOTS_HORIZONTAL
                                   )
                               ) |
                                   Ui::insets(12),
                               appContent(s) | Ui::vscroll() | Ui::grow()
                           ) |
                           Kr::scaffoldContent() |
                           Ui::grow();
                },
            });
        }
    );
}

export Async::Task<> timerTask(Ui::Child app, Async::CancellationToken ct) {
    while (not ct.cancelled()) {
        Model::event<TimeTick>(*app);
        co_trya$(Sys::globalSched().sleepAsync(Sys::instant() + Duration::fromSecs(1), ct));
    }
    co_return Ok();
}

} // namespace Hideo::Clock
