module;

#include <karm/macros>

export module Hideo.Sysmon:model;

import Karm.Ui;
import Karm.Core;
import Karm.Sys;

using namespace Karm;

namespace Hideo::Sysmon {

export enum struct Page {
    PROCESSES,

    PROCESSORS,
    MEMORY,
    DRIVES,
    NETWORK,
    GRAPHICS,
};

export struct State {
    Page page = Page::PROCESSES;
    Opt<usize> selected = NONE;
    Vec<Rc<Sys::ProcessStat>> processes;
    bool details = false;

    Opt<Rc<Sys::ProcessStat>>
    selectedProcess() const {
        return iter(processes) |
               FindFirst([&](Rc<Sys::ProcessStat> const& p) {
                   return p->id == selected;
               });
    }
};

export struct SelectProcess {
    usize id;
};

export struct KillProcess {};

export struct DetailProcess {};

export struct Refresh {};

export using Action = Union<
    Page,
    SelectProcess,
    KillProcess,
    DetailProcess,
    Refresh>;

Ui::Task<Action> reduce(State& s, Action action) {
    action.visit(
        [&](Page a) {
            s.page = a;
        },
        [&](SelectProcess a) {
            s.selected = Some(a.id);
        },
        [&](KillProcess) {
            if (auto& [id] = s.selected) {
                auto process = Sys::Process::open(id).ok();
                if (auto& [proc] = process)
                    (void)proc->kill();
            }
        },
        [&](DetailProcess) {
            s.details = not s.details;
        },
        [&](Refresh) {
            s.processes.clear();
            for (auto p : Sys::Process::list().expect()) {
                if (auto const& [stat] = p->stat(Sys::ProcessStat::ALL).ok())
                    s.processes.pushBack(stat);
            }
            reverse(mutSub(s.processes));
        }
    );
    return NONE;
}

export using Model = Ui::Model<State, Action, reduce>;

export Async::Task<> refreshTask(Ui::Child app, Async::CancellationToken ct) {
    while (not ct.cancelled()) {
        Model::event<Refresh>(*app);
        co_trya$(Sys::globalSched().sleepAsync(Sys::instant() + Duration::fromSecs(3), ct));
    }
    co_return Ok();
}

} // namespace Hideo::Sysmon
