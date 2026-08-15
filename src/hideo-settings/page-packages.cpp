module;

#include <karm/macros>

export module Hideo.Settings:pagePackages;

import Mdi;
import Karm.Kira;
import Karm.Ui;
import Karm.Sys;
import Karm.Gfx;
import Karm.Ref;

import :model;
import :common;

using namespace Karm::Literals;
using namespace Karm::Fmt::Literals;

namespace Hideo::Settings {

Ui::Child packageDetails(Sys::Bundle const& b) {
    return Ui::vflow(
        Kr::rowContent(
            NONE,
            "URL"s,
            NONE,
            Some(Ui::labelMedium(b.url().str()))
        ),
        Kr::rowContent(
            NONE,
            "Actions"s,
            NONE,
            Some(
                Ui::hflow(
                    6,
                    Ui::button(
                        Some(Ui::SINK<>),
                        Ui::ButtonStyle::destructive(),
                        "Uninstall"
                    ),
                    Ui::button(
                        Some(Ui::SINK<>),
                        Ui::ButtonStyle::text(),
                        "Explore Files"
                    ),
                    Ui::button(
                        Some(Ui::SINK<>),
                        Ui::ButtonStyle::regular(),
                        "Launch"
                    )
                )
            )
        )
    );
}

Ui::Child packageItem(Sys::Bundle const& b) {
    return Kr::treeRow(
               Some([] {
                   return Ui::icon(Mdi::WIDGETS_OUTLINE);
               }),
               b.id,
               NONE,
               Ui::Slot{[&] -> Ui::Child {
                   return packageDetails(b);
               }}
           ) |
           Kr::card();
}

Ui::Child packageList(State const& s) {
    if (not s.packages)
        return Kr::errorPage(Mdi::WIDGETS_OUTLINE, "Could not list packages"s, s.packages.none().msg());

    return Ui::vflow(
        6,
        iter(s.packages.unwrap()) |
            Select(packageItem) |
            Collect<Ui::Children>()
    );
}

export Ui::Child pagePackages(State const& s) {
    return Ui::vflow(
               8,
               Kr::titleRow("Packages"s),
               packageList(s)
           ) |
           pageScaffold;
}

} // namespace Hideo::Settings
