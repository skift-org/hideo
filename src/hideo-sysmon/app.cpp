module;

#include <karm/macros>

export module Hideo.Sysmon;

import Mdi;
import Karm.Ui;
import Karm.Kira;
import Karm.Core;
import Karm.Gfx;

import :model;

using namespace Karm::Literals;

namespace Hideo::Sysmon {

Ui::Child processListItem(State const& s, Rc<Sys::ProcessStat> const& process) {
    return Ui::button(
        Some(Model::bind<SelectProcess>(process->id)),
        s.selected == process->id ? Ui::ButtonStyle::regular() : Ui::ButtonStyle::subtle(),
        Mdi::COG,
        process->name
    );
}

Ui::Child processListContent(State const& s) {
    return Ui::vflow(
               8,
               iter(s.processes) |
                   Select([&](auto& p) {
                       return processListItem(s, p);
                   }) |
                   Collect<Ui::Children>()
           ) |
           Ui::insets(16) |
           Ui::vscroll();
}

export Ui::Child app() {
    return Ui::reducer<Model>({}, [](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::VIEW_DASHBOARD,
            .title = "System Monitor"s,
            .body = [&] {
                return processListContent(s) | Kr::scaffoldContent();
            },
        });
    });
}

export Async::Task<> refreshTask(Ui::Child app, Async::CancellationToken ct) {
    while (not ct.cancelled()) {
        Model::event<Refresh>(*app);
        co_trya$(Sys::globalSched().sleepAsync(Sys::instant() + Duration::fromSecs(3), ct));
    }
    co_return Ok();
}

} // namespace Hideo::Sysmon
