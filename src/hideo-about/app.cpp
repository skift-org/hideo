export module Hideo.About;

import Karm.Core;
import Karm.Image;
import Karm.Kira;
import Karm.Ref;
import Karm.Sys;
import Karm.Ui;
import Karm.Math;

import Mdi;

using namespace Karm;

namespace Hideo::About {

export Ui::Child app() {
    return Kr::scaffold({
        .icon = Mdi::INFORMATION,
        .title = "About"s,
        .body =
            [] {
                auto titleText = Ui::headlineMedium("skiftOS");

                auto bodyText =
                    Ui::bodySmall(
                        "Copyright © 2018-2025 The skiftOS Developers\n"
                        "\n"
                        "All rights reserved."
                    );

                auto inspireMe = Ui::state(Sys::now().val(), [](auto v, auto bind) {
                    auto body = Ui::hflow(
                        8, Math::Align::CENTER,
                        Ui::image("bundle://hideo-about/pride.qoi"_url, 4) | Ui::sizing(Ui::UNCONSTRAINED, 24),
                        Ui::bodySmall(wholesome(v))
                    );

                    return body | Ui::insets({6, 16, 6, 12}) |
                           Ui::minSize({Ui::UNCONSTRAINED, 36}) |
                           Ui::button(bind(v + 1), Ui::ButtonStyle::subtle());
                });

                auto licenseBtn = Ui::button(
                    [](auto& n) {
                        Ui::showDialog(n, Kr::licenseDialog());
                    },
                    Ui::ButtonStyle::outline(), Mdi::LICENSE, "License"
                );

                return Ui::vflow(
                           8,
                           Ui::hflow(
                               8, titleText,
                               Kr::versionBadge() | Ui::center()
                           ),
                           Ui::empty(),
                           bodyText,
                           Ui::grow(NONE),
                           Ui::hflow(
                               8,
                               inspireMe | Ui::vcenter() | Ui::grow(),
                               licenseBtn
                           )
                       ) |
                       Ui::insets(16);
            },
        .size = {460, 320},
    });
}

} // namespace Hideo::About
