export module Hideo.Sysmon:model;

import Karm.Ui;
import Karm.Core;
import Karm.Sys;

using namespace Karm;

namespace Hideo::Sysmon {

export struct State {
    Opt<usize> selected = NONE;
    Vec<Rc<Sys::ProcessStat>> processes;
};

export struct SelectProcess {
    usize id;
};

export struct KillProcess {
};

export struct Refresh {
};

export using Action = Union<SelectProcess, Refresh>;

Ui::Task<Action> reduce(State& s, Action a) {
    if (auto const& [select] = a.is<SelectProcess>()) {
        s.selected = Some(select.id);
    } else if (a.is<Refresh>()) {
        s.processes.clear();
        for (auto p : Sys::Process::list().expect()) {
            if (auto const& [stat] = p->stat(Sys::ProcessStat::ALL).ok())
                s.processes.pushBack(stat);
        }
        reverse(mutSub(s.processes));
    }
    return NONE;
}

export using Model = Ui::Model<State, Action, reduce>;

} // namespace Hideo::Sysmon
