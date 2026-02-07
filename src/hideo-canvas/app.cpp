export module Hideo.Canvas;

import Mdi;
import Karm.Core;
import Karm.Ui;
import Karm.Kira;
import Karm.Gfx;
import Karm.Math;
import Karm.Kira;

using namespace Karm;

namespace Hideo::Canvas {

Ui::Child canvas() {
    return Ui::vflow(
               2,
               Ui::labelSmall(Gfx::GRAY400, "Frame 1"),
               Ui::empty({320, 240}) |
                   Ui::box({
                       .backgroundFill = Gfx::WHITE,
                   })
           ) |
           Ui::center() | Ui::bound() | Kr::selectionArea();
}

Ui::Child panelBottom() {
    return Ui::hflow(
               4,
               Ui::button(Ui::SINK<>, Ui::ButtonStyle::subtle().withForegroundFill(Ui::ACCENT500), Mdi::CURSOR_DEFAULT),
               Ui::button(Ui::SINK<>, Ui::ButtonStyle::subtle(), Mdi::ARTBOARD),
               Ui::button(Ui::SINK<>, Ui::ButtonStyle::subtle(), Mdi::FORMAT_TEXT),
               Ui::button(Ui::SINK<>, Ui::ButtonStyle::subtle(), Mdi::RECTANGLE_OUTLINE),
               Ui::button(Ui::SINK<>, Ui::ButtonStyle::subtle(), Mdi::CHEVRON_UP)
           ) |
           Ui::box({
               .margin = 16,
               .padding = 2,
               .borderRadii = 4,
               .backgroundFill = Ui::GRAY900,
               .shadowStyle = Gfx::BoxShadow::elevated(4),
           });
}

Ui::Child panelLeft() {
    return Ui::vflow(
               Kr::labelRow("Layers"s)
           ) |
           Ui::box({
               .backgroundFill = Ui::GRAY900,
           }) |
           Ui::minSize({240, Ui::UNCONSTRAINED});
}

Ui::Child panelRight() {
    return Ui::vflow(
               Kr::labelRow("Postion"s),
               Kr::separator(),
               Kr::labelRow("Appearance"s),
               Kr::separator(),
               Kr::labelRow("Fill"s),
               Kr::separator(),
               Kr::labelRow("Stroke"s),
               Kr::separator(),
               Kr::labelRow("Effects"s),
               Kr::separator(),
               Kr::labelRow("Export"s),
               Kr::separator()
           ) |
           Ui::box({
               .backgroundFill = Ui::GRAY900,
           }) |
           Ui::minSize({240, Ui::UNCONSTRAINED});
}

export Ui::Child app() {
    return Kr::scaffold({
        .icon = Mdi::DRAW,
        .title = "Canvas"s,
        .body = [] {
            return Ui::hflow(
                panelLeft(),
                Kr::separator(),
                Ui::stack(canvas(), panelBottom() | Ui::align(Math::Align::BOTTOM | Math::Align::HCENTER)) | Ui::grow(),
                Kr::separator(),
                panelRight()
            );
        },
    });
}

} // namespace Hideo::Canvas
