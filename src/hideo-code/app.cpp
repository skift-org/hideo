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
        .body = [] {
            return Ui::empty() | Kr::scaffoldContent();
        },
    });
}

} // namespace Hideo::Code
