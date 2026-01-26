#include <karm/entry>

import Karm.Core;
import Karm.Ui;
import Hideo.Clock;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Context& ctx, Async::CancellationToken ct) {
    auto app = Hideo::Clock::app();
    Async::detach(Hideo::Clock::timerTask(app, ct));
    co_return co_await Ui::runAsync(ctx, app, ct);
}
