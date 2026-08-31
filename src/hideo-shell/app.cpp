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

export Ui::Child shellContent(State const& state) {
    if (state.locked)
        return lockScreen(state);

    if (App::formFactor == App::FormFactor::MOBILE)
        return mobileScreen(state);

    return desktopScreen(state);
}

export Ui::Child app(State state) {
    return Ui::reducer<Model>(
        std::move(state),
        [](State const& state) {
            auto content =
                shellContent(state) |
                Ui::dialogLayer() |
                Ui::popoverLayer() |
                Ui::pinSize(
                    App::formFactor == App::FormFactor::MOBILE
                        ? Math::Vec2i{411, 731}
                        : Math::Vec2i{1280, 720}
                ) |
                Ui::keyboardShortcut(App::Key::ESC, {}, [&](auto& n) {
                    if (state.activePanel != Panel::NIL)
                        Model::bubble<ActivatePanel>(n, {Panel::NIL});
                }) |
                Ui::keyboardShortcut(App::Key::SPACE, {App::KeyMod::SUPER}, [&](auto& n) {
                    Model::bubble<ActivatePanel>(n, {Panel::APPS});
                }) |
                Ui::keyboardShortcut(App::Key::L, {App::KeyMod::SUPER}, [&](auto& n) {
                    Model::bubble<Lock>(n);
                }) |
                Ui::keyboardShortcut(App::Key::V, {App::KeyMod::SUPER}, [&](auto& n) {
                    Model::bubble<ActivatePanel>(n, {Panel::NOTIS});
                });

            auto colorMatrix = Gfx::ColorMatrix::identity();
            colorMatrix *= Gfx::ColorMatrix::sepia(state.nightLight ? 0.7 : 0);
            colorMatrix *= Gfx::ColorMatrix::brightness(state.brightness);
            content = Ui::foregroundFilter(Gfx::ColorMatrixFilter{colorMatrix}, content);

            return content;
        }
    );
}

} // namespace Hideo::Shell
