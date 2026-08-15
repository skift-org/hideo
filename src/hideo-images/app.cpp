export module Hideo.Images;

import Karm.Gfx;
import Karm.Core;
import :editor;
import :viewer;

namespace Hideo::Images {

export Ui::Child app(Rc<Gfx::Image> initial) {
    return Ui::reducer<Model>(
        State{Viewer{initial}},
        [](State const& s) {
            return s.mode.visit(
                [&](Editor const&) {
                    return editorApp(s);
                },
                [&](Viewer const&) {
                    return viewerApp(s);
                }
            );
        }
    );
}

} // namespace Hideo::Images
