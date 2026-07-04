#include <karm/entry>

import Karm.Ui;
import Hideo.Books;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    co_return co_await Ui::runAsync(env, Hideo::Books::app(), ct);
}
