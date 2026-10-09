export module Hideo.Settings:pageHome;

import Mdi;
import Karm.Ui;
import Karm.Math;
import Karm.App;
import Karm.Sys;
import Karm.Gfx;

import :model;

using namespace Karm::Literals;
using namespace Karm::Math::Literals;

namespace Hideo::Settings {

Ui::Child tileButton(Ui::Send<> onPress, Gfx::Icon icon, String text) {
    return Ui::vflow(
               Ui::icon(icon, 32_au) | Ui::center() | Ui::grow(),
               Ui::text(text) | Ui::center()
           ) |
           Ui::insets(8_au) |
           Ui::bound() |
           Ui::minSize({96_au, 72_au}) |
           Ui::button(Some(onPress), Ui::ButtonStyle::secondary());
}

export Ui::Child pageHome(State const&) {
    Ui::Children items = {
        tileButton(Model::bind<GoTo>(Page::ACCOUNT), Mdi::ACCOUNT, "Accounts"s),
        tileButton(Model::bind<GoTo>(Page::PERSONALIZATION), Mdi::PALETTE, "Personalization"s),
        tileButton(Model::bind<GoTo>(Page::PACKAGES), Mdi::WIDGETS_OUTLINE, "Applications"s),

        tileButton(Model::bind<GoTo>(Page::SYSTEM), Mdi::LAPTOP, "System"s),
        tileButton(Model::bind<GoTo>(Page::NETWORK), Mdi::WIFI, "Network"s),
        tileButton(Model::bind<GoTo>(Page::SECURITY), Mdi::SECURITY, "Security & Privacy"s),

        tileButton(Model::bind<GoTo>(Page::UPDATES), Mdi::UPDATE, "Updates"s),
        tileButton(Model::bind<GoTo>(Page::ABOUT), Mdi::INFORMATION_OUTLINE, "About"s),
    };

    auto isMobile = App::formFactor == App::FormFactor::MOBILE;

    auto gridLayout = isMobile
                          ? Ui::GridStyle::simpleGrow(4, 2, 4_au)
                          : Ui::GridStyle::simpleFixed({3, 96_au}, {3, 120_au}, 8_au);

    auto grid = Ui::grid(gridLayout, items) |
                Ui::insets(8_au);

    if (isMobile)
        return grid;

    return grid | Ui::center() | Ui::bound() | Ui::grow();
}

} // namespace Hideo::Settings
