export module Hideo.Shell:mobile;

import Karm.Core;
import Karm.Ui;

import :model;
import :navbar;
import :background;
import :applications;
import :settings;
import :statusbar;

using namespace Karm;

namespace Hideo::Shell {

Ui::Child mobilePanels(State const& s) {
    return Ui::stack(
        s.activePanel == Panel::APPS
            ? appsFlyout(s)
            : Ui::empty(),
        s.activePanel == Panel::SYS
            ? sysFlyout(s)
            : Ui::empty()
    );
}

Ui::Child mobileAppHost(State const& s) {
    if (isEmpty(s.windows))
        return Ui::grow(NONE);

    return makeRc<Viewport>(first(s.windows), true, App::Snap::FULL, 0);
}

Ui::Child mobileScreen(State const& s) {
    return Ui::stack(
        s.windows.len() == 0
            ? background(s)
            : Ui::empty(),
        Ui::vflow(
            statusbarButton(s) | Ui::slideIn(Ui::SlideFrom::TOP),
            Ui::stack(mobileAppHost(s), mobilePanels(s)) | Ui::grow(),
            s.keyboard ? Keyboard::flyout() : Ui::empty(),
            navbar(s)
        )
    );
}

} // namespace Hideo::Shell
