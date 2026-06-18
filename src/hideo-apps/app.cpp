export module Hideo.Apps;

import Mdi;
import Karm.Core;
import Karm.Ui;
import Karm.Kira;

using namespace Karm;
using namespace Karm::Literals;

namespace Hideo::Apps {

Ui::Child sidebar() {
    return Kr::sidenavContent({
        Kr::searchbar(""s) | Ui::insets({6, 0}),
        Kr::sidenavItem(true, Ui::SINK<>, Mdi::STAR_OUTLINE, "Discover"s),
        Kr::sidenavItem(false, Ui::SINK<>, Mdi::GAMEPAD_OUTLINE, "Play"s),
        Kr::sidenavItem(false, Ui::SINK<>, Mdi::BRUSH_OUTLINE, "Create"s),
        Kr::sidenavItem(false, Ui::SINK<>, Mdi::BRIEFCASE_OUTLINE, "Work"s),
        Kr::sidenavItem(false, Ui::SINK<>, Mdi::SHAPE, "Other"s),
        Kr::sidenavItem(false, Ui::SINK<>, Mdi::DOWNLOAD_BOX_OUTLINE, "Updates"s),
    });
}

Ui::Child pageContent() {
    return Ui::empty();
}

export Ui::Child app() {
    return Kr::scaffold({
        .icon = Mdi::BASKET,
        .title = "Apps"s,
        .sidebar = [] {
            return sidebar() | Kr::resizable(Kr::ResizeHandlePosition::END);
        },
        .body = [] {
            return pageContent() | Kr::scaffoldContent() | Ui::grow();
        },
    });
}

} // namespace Hideo::Apps
