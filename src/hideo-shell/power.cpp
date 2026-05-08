export module Hideo.Shell:power;

import Mdi;
import Karm.Ui;
import Karm.Kira;
import Karm.Core;

using namespace Karm;
using namespace Karm::Literals;

namespace Hideo::Shell {

export Ui::Child powerDialog() {
    return Kr::dialogContent({
        Kr::dialogTitleBar("Power Options"s),
        Kr::dialogBody({
            Ui::button(Ui::closeDialog, Ui::ButtonStyle::subtle(), Mdi::LOGOUT, "Logout"),
            Ui::button(Ui::closeDialog, Ui::ButtonStyle::subtle(), Mdi::SLEEP, "Sleep"),
            Ui::button(Ui::closeDialog, Ui::ButtonStyle::subtle(), Mdi::RESTART, "Restart"),
            Ui::button(Ui::closeDialog, Ui::ButtonStyle::subtle(), Mdi::POWER_STANDBY, "Shutdown"),
        }) |
            Ui::minSize({260, Ui::UNCONSTRAINED}),
    });
}

} // namespace Hideo::Shell
