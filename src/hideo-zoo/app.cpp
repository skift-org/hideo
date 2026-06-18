export module Hideo.Zoo:app;

import Karm.Core;
import Karm.Kira;
import Karm.Ui;
import Karm.Glob;

import Mdi;

import :pages;
import :model;

namespace Hideo::Zoo {

export Ui::Child app() {
    return Ui::reducer<Model>({.selectedPage = PAGES[0]}, [](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::DUCK,
            .title = "Zoo"s,
            .sidebar = [&] {
                Vec<Tuple<Page const*, int>> pages;
                pages.ensure(PAGES.len());
                for (auto p : PAGES) {
                    if (not s.searchQuery) {
                        pages.pushBack({p, {}});
                        continue;
                    }

                    auto match = Glob::matchFuzzy(p->name, s.searchQuery);
                    if (not match)
                        continue;
                    pages.pushBack({p, match->score});
                }

                if (s.searchQuery)
                    sort(pages, [](auto& a, auto& b) {
                        return b.v1 <=> a.v1;
                    });

                auto items =
                    iter(pages) |
                    Select([&](Tuple<Page const*, int> item) {
                        auto page = item.v0;
                        return Kr::sidenavItem(
                            page == s.selectedPage,
                            Model::bind<Switch>(page),
                            item.v0->icon,
                            item.v0->name
                        );
                    }) |
                    Collect<Ui::Children>();

                return Kr::sidenavContent({
                           Kr::searchbar(s.searchQuery, Model::map<UpdateSearch>()),
                           Ui::vflow(8, items) | Ui::grow(),
                       }) |
                       Kr::resizable(Kr::ResizeHandlePosition::END);
            },
            .body = [&] {
                auto page = s.selectedPage;
                return Ui::vflow(
                           Ui::vflow(
                               Ui::titleMedium(page->name),
                               Ui::empty(4),
                               Ui::bodySmall(page->description)
                           ) | Ui::insets(16),
                           Kr::separator(),
                           page->build() | Ui::grow()
                       ) |
                       Kr::scaffoldContent();
            },
        });
    });
}

} // namespace Hideo::Zoo
