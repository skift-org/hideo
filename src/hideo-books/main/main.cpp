#include <karm/entry>

import Karm.Ui;
import Hideo.Books;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Context& ctx, Async::CancellationToken ct) {
    co_return co_await Ui::runAsync(ctx, Hideo::Books::app(), ct);
}
