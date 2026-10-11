module;

#include <karm/macros>

export module Hideo.Sysmon:model;

import Karm.Ui;
import Karm.Core;
import Karm.Sys;
import Karm.Chart;

using namespace Karm;

namespace Hideo::Resources {

export enum struct Page {
    PROCESSES,
    PROCESSORS,
    MEMORY,
};

export struct State {
    Page page = Page::PROCESSES;
    Opt<usize> selected = NONE;
    Vec<Rc<Sys::ProcessStat>> processes;
    bool details = false;

    Vec<Sys::Cpu> hartInfo;
    Chart::Serie cpuData;
    Vec<Chart::Serie> hartData;
    Sys::MemInfo memoryInfo;
    Chart::Serie memoryData;
    Chart::Serie swapData;

    State() {
        memoryData.values.resize(32);
        swapData.values.resize(32);
        hartInfo = Sys::Cpu::list().expect();

        cpuData.values.resize(32);
        hartData.resize(hartInfo.len());
        for (auto& c : hartData)
            c.values.resize(32);
    }

    Opt<Rc<Sys::ProcessStat>> selectedProcess() const {
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

            // cpu

            auto newCpuInfo = Sys::Cpu::list().expect();

            f64 totalUsage = 0.;
            for (auto& [c, i] : iter(newCpuInfo) | Index()) {
                auto totalTime = s.hartInfo[i].totalTime();
                auto idleTime = s.hartInfo[i].idleTime;
                auto usage = 1 - ((c.idleTime - idleTime).value() / (f64)(c.totalTime() - totalTime).value());
                s.hartData[i].values.pushBack(usage);
                s.hartData[i].values.popFront();
                totalUsage += usage;
            }
            s.cpuData.values.pushBack(totalUsage / newCpuInfo.len());
            s.cpuData.values.popFront();

            s.hartInfo = newCpuInfo;

            // memory
            s.memoryInfo = Sys::meminfo().expect();
            s.memoryData.values.pushBack(s.memoryInfo.physicalUsed() / (f64)s.memoryInfo.physicalTotal);
            s.memoryData.values.popFront();

            s.swapData.values.pushBack(s.memoryInfo.swapUsed() / (f64)s.memoryInfo.swapTotal);
            s.swapData.values.popFront();
        }
    );
    return NONE;
}

export using Model = Ui::Model<State, Action, reduce>;

export Async::Task<> refreshTask(Ui::Child app, Async::CancellationToken ct) {
    while (not ct.cancelled()) {
        Model::event<Refresh>(*app);
        co_trya$(Sys::globalSched().sleepAsync(Sys::instant() + Duration::fromSecs(1), ct));
    }
    co_return Ok();
}

} // namespace Hideo::Resources
