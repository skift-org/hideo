#include <karm/entry>

import Hideo.Shell;
import Karm.App;
import Karm.Image;
import Karm.Ui;
import Karm.Sys;
import Karm.Gfx;
import Mdi;

using namespace Karm;
using namespace Karm::Literals;
using namespace Karm::Ref::Literals;

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    Hideo::Shell::State state = {
        .background = co_try$(Image::loadOrFallback("bundle://hideo-shell/wallpapers/abstract.qoi"_url)),
    };
    auto app = Hideo::Shell::app(std::move(state));
    Async::detach(Hideo::Shell::timerTask(*app, ct));
    co_return co_await Ui::runAsync(env, app, ct);
}
