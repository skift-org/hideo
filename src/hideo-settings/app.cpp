export module Hideo.Settings:app;

import Karm.Ui;

import Mdi;
import Karm.Kira;
import Karm.Core;
import :model;
import :pageHome;
import :pageAbout;
import :pagePackages;

namespace Hideo::Settings {

Ui::Child sidebar(State const& state) {
    Ui::Children items = {
        Kr::searchbar(""s) | Ui::insets({6, 0}),
        Kr::sidenavItem(state.page() == Page::ACCOUNT, Some(Model::bind<GoTo>(Page::ACCOUNT)), Mdi::ACCOUNT, "Accounts"s),
        Kr::sidenavItem(state.page() == Page::PERSONALIZATION, Some(Model::bind<GoTo>(Page::PERSONALIZATION)), Mdi::PALETTE, "Personalization"s),
        Kr::sidenavItem(state.page() == Page::PACKAGES, Some(Model::bind<GoTo>(Page::PACKAGES)), Mdi::WIDGETS_OUTLINE, "Packages"s),

        Kr::sidenavItem(state.page() == Page::SYSTEM, Some(Model::bind<GoTo>(Page::SYSTEM)), Mdi::LAPTOP, "System"s),
        Kr::sidenavItem(state.page() == Page::NETWORK, Some(Model::bind<GoTo>(Page::NETWORK)), Mdi::WIFI, "Network"s),
        Kr::sidenavItem(state.page() == Page::SECURITY, Some(Model::bind<GoTo>(Page::SECURITY)), Mdi::SECURITY, "Security & Privacy"s),

        Kr::sidenavItem(state.page() == Page::UPDATES, Some(Model::bind<GoTo>(Page::UPDATES)), Mdi::UPDATE, "Updates"s),
        Kr::sidenavItem(state.page() == Page::ABOUT, Some(Model::bind<GoTo>(Page::ABOUT)), Mdi::INFORMATION_OUTLINE, "About"s),
    };

    return Kr::sidenavContent(items);
}

// MARK: Pages -----------------------------------------------------------------

Ui::Child pageContent(State const& state) {
    switch (state.page()) {
    case Page::HOME:
        return pageHome(state);

    case Page::PACKAGES:
        return pagePackages(state);

    case Page::ABOUT:
        return pageAbout(state);

    default:
        return Ui::text("Content") |
               Ui::center() |
               Ui::grow();
    }
}

// MARK: Body ------------------------------------------------------------------

export Ui::Child app() {
    return Ui::reducer<Model>(
        {
            Sys::Bundle::installed(),
        },
        [](State const& s) {
            return Kr::scaffold({
                .icon = Mdi::COG,
                .title = "Settings"s,
                .startTools = Some([&] -> Ui::Children {
                    return {
                        Ui::button(Model::bindIf<GoBack>(s.canGoBack()), Ui::ButtonStyle::subtle(), Mdi::ARROW_LEFT),
                        Ui::button(Model::bindIf<GoForward>(s.canGoForward()), Ui::ButtonStyle::subtle(), Mdi::ARROW_RIGHT),
                        Ui::button(Some(Model::bind<GoTo>(Page::HOME)), Ui::ButtonStyle::subtle(), Mdi::HOME),
                    };
                }),
                .sidebar = Some([&] {
                    return sidebar(s) | Kr::resizable(Kr::ResizeHandlePosition::END);
                }),
                .body = [&] {
                    return pageContent(s) | Ui::bound() | Kr::scaffoldContent() | Ui::grow();
                },
            });
        }
    );
}

} // namespace Hideo::Settings
