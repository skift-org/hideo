export module Hideo.Zoo:model;

import Karm.Core;
import Karm.Ui;
import Karm.Gfx;

using namespace Karm;
using namespace Karm::Literals;

namespace Hideo::Zoo {

struct Page {
    Gfx::Icon icon;
    Str name;
    Str description;
    Ui::Slot build;
};

struct State {
    Page const* selectedPage;
    String searchQuery = ""s;
};

export struct UpdateSearch {
    String query;
};

struct Switch {
    Page const* page;
};

export using Action = Union<UpdateSearch, Switch>;

Ui::Task<Action> reduce(State& s, Action a) {
    a.visit(
        [&](UpdateSearch updateSearch) {
            s.searchQuery = updateSearch.query;
        },
        [&](Switch action) {
            s.selectedPage = action.page;
        }
    );

    return NONE;
}

export using Model = Ui::Model<State, Action, reduce>;

} // namespace Hideo::Zoo
