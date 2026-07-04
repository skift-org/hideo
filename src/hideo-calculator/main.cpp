#include <karm/entry>

import Hideo.Calculator;
import Karm.Ui;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    co_return co_await Ui::runAsync(env, Hideo::Calculator::app(), ct);
}
