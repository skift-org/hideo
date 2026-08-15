export module Hideo.Shell:navbar;

import Karm.Ui;
import Karm.Kira;
import Hideo.Keyboard;
import :model;

using namespace Karm;

namespace Hideo::Shell {

export Ui::Child navbar(State const& s) {
    return Ui::stack(
               Kr::buttonHandle(Some(Model::bind<ActivatePanel>(Panel::APPS))),
               Ui::hflow(
                   Ui::grow(NONE),
                   Ui::button(Some(Model::bind<ToggleKeyboard>()), Ui::ButtonStyle::subtle(), Mdi::KEYBOARD)
               )
           ) |
           Ui::box({.backgroundFill = Some(s.keyboard ? Ui::GRAY950 : Gfx::ALPHA)}) |
           Ui::slideIn(Ui::SlideFrom::BOTTOM);
}

} // namespace Hideo::Shell
