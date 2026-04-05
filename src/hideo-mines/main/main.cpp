#include <karm/entry>

import Karm.Ui;
import Hideo.Mines;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    co_return co_await Ui::runAsync(env, Hideo::Mines::app(), ct);
}
