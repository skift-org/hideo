export module Hideo.Images:viewer;

import Mdi;
import Karm.Core;
import Karm.Kira;
import Karm.Ui;

import :model;

using namespace Karm::Literals;

namespace Hideo::Images {

Ui::Child viewerPreview(State const& state) {
    return Ui::image(state.mode.unwrap<Viewer>().image) |
           Ui::box({
               .borderWidth = 1,
               .borderFill = Some(Ui::GRAY50.withOpacity(0.1)),
               .backgroundFill = Some(Ui::GRAY50),
           }) |
           Ui::insets(8) |
           Ui::fit();
}

Ui::Child viewerControls(State const&) {
    return Ui::hflow(
               Ui::button(Some(Ui::SINK<>), Ui::ButtonStyle::subtle(), Mdi::ARROW_LEFT),
               Ui::button(Some(Ui::SINK<>), Ui::ButtonStyle::subtle(), Mdi::ARROW_RIGHT)
           ) |
           Ui::insets({0, 0, 8, 0}) |
           Ui::center();
}

export Ui::Child viewerApp(State const& state) {
    return Kr::scaffold({
        .icon = Mdi::IMAGE,
        .title = "Images"s,
        .startTools = Some([&] -> Ui::Children {
            return {
                Ui::button(Some(Ui::SINK<>), Ui::ButtonStyle::subtle(), Mdi::MAGNIFY_PLUS),
                Ui::button(Some(Ui::SINK<>), Ui::ButtonStyle::subtle(), Mdi::MAGNIFY_MINUS),
                Ui::button(Some(Ui::SINK<>), Ui::ButtonStyle::subtle(), Mdi::FULLSCREEN),
            };
        }),
        .endTools = Some([&] -> Ui::Children {
            return {
                Ui::button(
                    Some(Model::bind<Edit>()),
                    Ui::ButtonStyle::subtle(),
                    Mdi::PENCIL,
                    "Edit"
                ),
            };
        }),
        .body = [&] {
            return Ui::vflow(
                viewerPreview(state) | Ui::bound() | Kr::scaffoldContent() | Ui::grow(),
                viewerControls(state)
            );
        },
    });
}

} // namespace Hideo::Images
