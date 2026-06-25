export module Hideo.Counter;

import Karm.Ui;
import Karm.Kira;
import Karm.Core;
import Karm.Math;
import Mdi;
import :model;

using namespace Karm::Literals;

namespace Hideo::Counter {

export Ui::Child app() {
    return Ui::reducer<Model>([](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::COUNTER,
            .title = "Counter"s,
            .body = [&] {
                auto decBtn = Ui::button(
                    Model::bind<DecrementAction>(),
                    Ui::ButtonStyle::regular().withRadii(999),
                    Mdi::MINUS_THICK
                );

                auto incBtn = Ui::button(
                    Model::bind<IncrementAction>(),
                    Ui::ButtonStyle::regular().withRadii(999),
                    Mdi::PLUS_THICK
                );

                auto resetBtn = Ui::button(
                    Model::bindIf<ResetAction>(not s.initial),
                    Ui::ButtonStyle::subtle().withRadii(999),
                    Mdi::REFRESH, "Reset"
                );

                return Ui::vflow(
                           32,
                           Math::Align::CENTER,
                           Ui::text(
                               Ui::TextStyles::codeLarge()
                                   .withFontSize(48),
                               "{}", s.counter
                           ) | Ui::grow(),
                           Ui::hflow(16, decBtn, incBtn),
                           resetBtn
                       ) |
                       Ui::insets(32) | Kr::scaffoldContent();
            },
            .size = 420,
        });
    });
}

} // namespace Hideo::Counter
