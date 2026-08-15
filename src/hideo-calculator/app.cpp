export module Hideo.Calculator:app;

import Mdi;
import Karm.Core;
import Karm.Kira;
import Karm.Ui;
import Karm.Math;

import :model;

namespace Hideo::Calculator {

Ui::Child textButton(Opt<Ui::Send<>> onPress, Ui::ButtonStyle style, String t) {
    return Ui::text(Ui::TextStyles::labelLarge().withFontSize(18), t) |
           Ui::center() |
           Ui::bound() |
           Ui::button(onPress, style);
}

Ui::Child textButton(Opt<Ui::Send<>> onPress, String t) {
    return textButton(std::move(onPress), Ui::ButtonStyle::regular(), t);
}

Ui::Child keypad(State const& state) {
    return Ui::insets(
        8,
        Ui::grid(
            Ui::GridStyle::simpleGrow(7, 4, 4),

            Ui::cell(
                {0, 0},
                {3, 0},
                Ui::hflow(
                    4,
                    Ui::button(Model::bindIf<MemClearAction>(state.hasMem), Ui::ButtonStyle::subtle(), "MC"),
                    Ui::button(Model::bindIf<MemRecallAction>(state.hasMem), Ui::ButtonStyle::subtle(), "MR"),
                    Ui::button(Some(Model::bind<MemAddAction>()), Ui::ButtonStyle::subtle(), "M+"),
                    Ui::button(Some(Model::bind<MemSubAction>()), Ui::ButtonStyle::subtle(), "M-"),
                    Ui::button(Some(Model::bind<MemStoreAction>()), Ui::ButtonStyle::subtle(), "MS")
                )
            ),

            textButton(Some(Model::bind<ClearAllAction>()), Ui::ButtonStyle::secondary(), "CE"s),
            textButton(Some(Model::bind<ClearAction>()), Ui::ButtonStyle::secondary(), "C"s),
            Ui::button(Some(Model::bind(Operator::TO_PERCENT)), Ui::ButtonStyle::secondary(), Mdi::PERCENT),
            Ui::button(Some(Model::bind<BackspaceAction>()), Ui::ButtonStyle::secondary(), Mdi::BACKSPACE_OUTLINE),

            textButton(Some(Model::bind(Operator::RESIPROCAL)), Ui::ButtonStyle::secondary(), "1/x"s),
            textButton(Some(Model::bind(Operator::SQUARE)), Ui::ButtonStyle::secondary(), "x²"s),
            textButton(Some(Model::bind(Operator::SQRT)), Ui::ButtonStyle::secondary(), "√x"s),
            Ui::button(Some(Model::bind(Operator::DIV)), Ui::ButtonStyle::secondary(), Mdi::DIVISION),

            textButton(Some(Model::bind<Number>(7)), "7"s),
            textButton(Some(Model::bind<Number>(8)), "8"s),
            textButton(Some(Model::bind<Number>(9)), "9"s),
            Ui::button(Some(Model::bind(Operator::MULT)), Ui::ButtonStyle::secondary(), Mdi::MULTIPLICATION),

            textButton(Some(Model::bind<Number>(4)), "4"s),
            textButton(Some(Model::bind<Number>(5)), "5"s),
            textButton(Some(Model::bind<Number>(6)), "6"s),
            Ui::button(Some(Model::bind(Operator::SUB)), Ui::ButtonStyle::secondary(), Mdi::MINUS),

            textButton(Some(Model::bind<Number>(1)), "1"s),
            textButton(Some(Model::bind<Number>(2)), "2"s),
            textButton(Some(Model::bind<Number>(3)), "3"s),
            Ui::button(Some(Model::bind(Operator::ADD)), Ui::ButtonStyle::secondary(), Mdi::PLUS),

            textButton(Some(Model::bind(Operator::INVERT_SIGN)), "+/-"s),
            textButton(Some(Model::bind<Number>(0)), "0"s),
            Ui::button(Some(Model::bind<EnterDecimalAction>()), Mdi::CIRCLE_SMALL),
            Ui::button(Some(Model::bind<EqualAction>()), Ui::ButtonStyle::primary(), Mdi::EQUAL)
        )
    );
}

Ui::Child screen(State const& state) {
    // auto debugExpr = Ui::text("op: {}, lhs: {}, rhs: {}", toFmt(state.op), state.lhs, state.rhs);

    auto currExpr =
        (state.op == Operator::NONE ? Ui::text("") : Ui::text(toFmt(state.op), state.lhs)) |
        Ui::align(Math::Align::VCENTER | Math::Align::END);

    auto result =
        (state.error ? Ui::headlineMedium(*state.error)
                     : Ui::headlineMedium("{}", state.hasRhs ? state.rhs : state.lhs)) |
        Ui::align(Math::Align::VCENTER | Math::Align::END);

    return Ui::vflow(8, /* debugExpr, */ currExpr, result) |
           Ui::insets({8, 16}) |
           Ui::focusable();
}

export Ui::Child app() {
    return Ui::reducer<Model>([](State const& state) {
        return Kr::scaffold({
            .icon = Mdi::CALCULATOR,
            .title = "Calculator"s,
            .body = [&] {
                return Ui::vflow(
                           screen(state) | Ui::dragRegion(),
                           keypad(state) | Ui::grow()
                       ) |
                       Kr::scaffoldContent();
            },
            .size = {280, 440},
        });
    });
}

} // namespace Hideo::Calculator
