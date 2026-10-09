export module Hideo.Settings:common;

import Karm.Ui;
import Karm.Math;

using namespace Karm;
using namespace Karm::Math::Literals;

namespace Hideo::Settings {

export Ui::Child pageScaffold(Ui::Child inner) {
    return inner |
           Ui::insets({0_au, 0_au, 32_au, 0_au}) |
           Ui::maxSize({460_au, Ui::UNCONSTRAINED}) |
           Ui::grow() |
           Ui::hcenter() |
           Ui::vscroll() |
           Ui::grow();
}

} // namespace Hideo::Settings