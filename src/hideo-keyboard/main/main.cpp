#include <karm/entry>

import Karm.Ui;
import Hideo.Keyboard;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    co_return co_await Ui::runAsync(env, Hideo::Keyboard::flyout(), ct);
}
