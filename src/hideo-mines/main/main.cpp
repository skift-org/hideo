#include <karm/entry>

import Karm.Ui;
import Hideo.Mines;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Context& ctx, Async::CancellationToken ct) {
    co_return co_await Ui::runAsync(ctx, Hideo::Mines::app(), ct);
}
