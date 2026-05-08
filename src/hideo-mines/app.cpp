export module Hideo.Mines;

import Mdi;
import Karm.Core;
import Karm.Ui;
import Karm.Kira;

using namespace Karm;
using namespace Karm::Literals;

namespace Hideo::Mines {

export Ui::Child app() {
    return Kr::scaffold({
        .icon = Mdi::MINE,
        .title = "Mines"s,
        .body = [] {
            return Ui::empty() | Kr::scaffoldContent();
        },
    });
}

} // namespace Hideo::Mines
