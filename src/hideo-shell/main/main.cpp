#include <karm/entry>

import Hideo.Shell;
import Karm.App;
import Karm.Image;
import Karm.Ui;
import Karm.Sys;
import Karm.Gfx;
import Mdi;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    Hideo::Shell::State state = {
        .dateTime = Sys::dateTime(),
        .background = co_try$(Image::loadOrFallback("bundle://hideo-shell/wallpapers/abstract.qoi"_url)),
        .noti = {},
        .launchers = {
            makeRc<Hideo::Shell::MockLauncher>(Mdi::INFORMATION_OUTLINE, "About"s, Gfx::BLUE_RAMP),
            makeRc<Hideo::Shell::MockLauncher>(Mdi::CALCULATOR, "Calculator"s, Gfx::ORANGE_RAMP),
            makeRc<Hideo::Shell::MockLauncher>(Mdi::PALETTE_SWATCH, "Color Picker"s, Gfx::RED_RAMP),
            makeRc<Hideo::Shell::MockLauncher>(Mdi::COUNTER, "Counter"s, Gfx::GREEN_RAMP),
            makeRc<Hideo::Shell::MockLauncher>(Mdi::DUCK, "Demos"s, Gfx::YELLOW_RAMP),
            makeRc<Hideo::Shell::MockLauncher>(Mdi::FILE, "Files"s, Gfx::ORANGE_RAMP),
            makeRc<Hideo::Shell::MockLauncher>(Mdi::FORMAT_FONT, "Fonts"s, Gfx::BLUE_RAMP),
            makeRc<Hideo::Shell::MockLauncher>(Mdi::EMOTICON, "Hello World"s, Gfx::RED_RAMP),
            makeRc<Hideo::Shell::MockLauncher>(Mdi::IMAGE, "Icons"s, Gfx::GREEN_RAMP),
            makeRc<Hideo::Shell::MockLauncher>(Mdi::IMAGE, "Image Viewer"s, Gfx::YELLOW_RAMP),
            makeRc<Hideo::Shell::MockLauncher>(Mdi::COG, "Settings"s, Gfx::ZINC_RAMP),
            makeRc<Hideo::Shell::MockLauncher>(Mdi::TABLE, "Spreadsheet"s, Gfx::GREEN_RAMP),
        },
        .filtered = {},
        .windows = {}
    };

    co_return co_await Ui::runAsync(env, Hideo::Shell::app(std::move(state)), ct);
}
