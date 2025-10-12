export module Hideo.Shell:app;

import Mdi;
import Karm.Ui;
import Karm.Kira;
import Karm.App;
import Karm.Gfx;
import Hideo.Keyboard;
import Karm.Math;

import :model;
import :mobile;
import :desktop;
import :lock;

using namespace Karm;

namespace Hideo::Shell {

// MARK: Shells ----------------------------------------------------------------

export Ui::Child app(State state) {
    return Ui::reducer<Model>(
        std::move(state),
        [](auto const& state) {
            auto content =
                Ui::stack(
                    state.locked
                        ? lock(state)
                        : (App::formFactor == App::FormFactor::MOBILE ? mobile(state)
                                          : desktop(state)),

                    App::formFactor == App::FormFactor::MOBILE
                        ? Ui::empty()
                        : desktopPanels(state)
                ) |
                Ui::dialogLayer() |
                Ui::popoverLayer() |
                Ui::pinSize(
                    App::formFactor == App::FormFactor::MOBILE 
                        ? Math::Vec2i{411, 731}
                        : Math::Vec2i{1280, 720}
                );

            if (state.nightLight) {
                content = Ui::foregroundFilter(Gfx::SepiaFilter{0.7}, content);
            }

            content = Ui::foregroundFilter(
                Gfx::BrightnessFilter{state.brightness},
                content
            );

            return content;
        }
    );
}

} // namespace Hideo::Shell
