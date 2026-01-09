export module Hideo.Shell:mock;

import Karm.Kira;
import Karm.Ui;
import Karm.Gfx;
import :model;

namespace Hideo::Shell {

export struct MockInstance : Window {
    Gfx::Icon icon;
    String name;
    Gfx::ColorRamp ramp;

    MockInstance(Gfx::Icon icon, String name, Gfx::ColorRamp ramp)
        : icon(icon), name(name), ramp(ramp) {}

    Rc<Gfx::Surface> surface() const override {
        return Gfx::Surface::fallback();
    }

    void event(App::Event&) override {}
};

export struct MockLauncher : Launcher {
    using Launcher::Launcher;

    void launch(State& s) override {
        auto instance = makeRc<MockInstance>(
            icon,
            name,
            ramp
        );
        s.windows.emplaceFront(instance);
    }
};

} // namespace Hideo::Shell
