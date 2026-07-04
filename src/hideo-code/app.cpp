export module Hideo.Code;

import Mdi;
import Karm.Core;
import Karm.Kira;
import Karm.Ui;

using namespace Karm;
using namespace Karm::Literals;

namespace Hideo::Code {

export Ui::Child app() {
    return Kr::scaffold({
        .icon = Mdi::CODE_BRACES,
        .title = "Code"s,
        .sidebar = [] {
            return Ui::empty(128) | Kr::scaffoldContent() | Kr::resizable(Kr::ResizeHandlePosition::END);
        },
        .body = [] {
            return Ui::vflow(Ui::empty() | Kr::scaffoldContent() | Ui::grow(), Ui::empty(128) | Kr::scaffoldContent() | Kr::resizable(Kr::ResizeHandlePosition::TOP));
        },
    });
}

} // namespace Hideo::Code
