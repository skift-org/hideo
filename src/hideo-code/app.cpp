export module Hideo.Code;

import Mdi;
import Karm.Core;
import Karm.Kira;
import Karm.Ui;

using namespace Karm;
using namespace Karm::Literals;

namespace Hideo::Code {

export struct Document {};

export struct State {
    bool terminalPanel = false;
    Vec<Document> _documents;
    usize _active;
};

struct ToggleTerminalPanel {};

export using Action = Union<
    ToggleTerminalPanel>;

Ui::Task<Action> reduce(State& s, Action a) {
    a.visit(
        [&](ToggleTerminalPanel) {
            s.terminalPanel = not s.terminalPanel;
        }
    );

    return NONE;
}

export using Model = Ui::Model<State, Action, reduce>;

export Ui::Child app() {
    return Ui::reducer<Model>({}, [](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::CODE_BRACES,
            .title = "Code"s,
            .endTools = [&] -> Ui::Children {
                return {
                    Ui::button(Model::bind<ToggleTerminalPanel>(), Ui::ButtonStyle::subtle(), Mdi::CONSOLE),
                };
            },
            .sidebar = [&] {
                return Ui::empty(128) |
                       Kr::scaffoldContent() |
                       Kr::resizable(Kr::ResizeHandlePosition::END);
            },
            .body = [&] {
                return Ui::vflow(
                    Ui::empty() |
                        Kr::scaffoldContent() | Ui::grow(),
                    Ui::empty(128) |
                        Kr::scaffoldContent() |
                        Kr::resizable(Kr::ResizeHandlePosition::TOP) |
                        Ui::cond(s.terminalPanel)
                );
            },
        });
    });
}

} // namespace Hideo::Code
