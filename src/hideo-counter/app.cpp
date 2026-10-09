export module Hideo.Counter;

import Karm.Ui;
import Karm.Kira;
import Karm.Core;
import Karm.Math;
import Karm.Signals;
import Mdi;
import :model;

using namespace Karm::Literals;
using namespace Karm::Math::Literals;

namespace Hideo::Counter {

export Ui::Child app() {
    return Ui::reducer<Model>([](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::COUNTER,
            .title = "Counter"s,
            .body = [&] {
                auto decBtn = Ui::button(
                    Some(Model::bind<DecrementAction>()),
                    Ui::ButtonStyle::regular().withRadii(999),
                    Mdi::MINUS_THICK
                );

                auto incBtn = Ui::button(
                    Some(Model::bind<IncrementAction>()),
                    Ui::ButtonStyle::regular().withRadii(999),
                    Mdi::PLUS_THICK
                );

                auto resetBtn = Ui::button(
                    Model::bindIf<ResetAction>(not s.initial),
                    Ui::ButtonStyle::subtle().withRadii(999),
                    Mdi::REFRESH, "Reset"
                );

                return Ui::vflow(
                           32_au,
                           Math::Align::CENTER,
                           Ui::text(
                               Ui::TextStyles::codeLarge()
                                   .withFontSize(48),
                               "{}", s.counter
                           ) | Ui::grow(),
                           Ui::hflow(16_au, decBtn, incBtn),
                           resetBtn
                       ) |
                       Ui::insets(32_au) | Kr::scaffoldContent();
            },
            .size = 420_au,
        });
    });
}

} // namespace Hideo::Counter
