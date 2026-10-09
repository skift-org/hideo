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
using namespace Karm::Literals;
using namespace Karm::Ref::Literals;
using namespace Karm::Math::Literals;

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
                        "Copyright © 2018-2026 The skiftOS Developers\n"
                        "\n"
                        "All rights reserved."
                    );

                auto inspireMe = Ui::state(Sys::instant().val(), [](auto v, auto bind) {
                    auto body = Ui::hflow(
                        8_au, Math::Align::CENTER,
                        Ui::image("bundle://hideo-about/progress.qoi"_url, Some(4)) | Ui::sizing(Ui::UNCONSTRAINED, 24_au),
                        Ui::bodySmall(wholesome(v))
                    );

                    return body | Ui::insets({6_au, 16_au, 6_au, 12_au}) |
                           Ui::minSize({Ui::UNCONSTRAINED, 32_au}) |
                           Ui::button(Some(bind(v + 1)), Ui::ButtonStyle::subtle());
                });

                auto licenseBtn = Ui::button(
                    Some([](auto& n) {
                        Ui::showDialog(n, Kr::licenseDialog());
                    }),
                    Ui::ButtonStyle::outline(), Mdi::LICENSE, "License"
                );

                return Ui::vflow(
                           8_au,
                           Ui::hflow(
                               8_au, titleText,
                               Kr::versionBadge() | Ui::center()
                           ),
                           Ui::empty(),
                           bodyText,
                           Ui::grow(NONE),
                           Ui::hflow(
                               8_au,
                               inspireMe | Ui::vcenter() | Ui::grow(),
                               licenseBtn
                           )
                       ) |
                       Ui::insets(8_au) | Kr::scaffoldContent();
            },
        .size = {460_au, 320_au},
    });
}

} // namespace Hideo::About
