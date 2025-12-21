#include <karm-sys/entry.h>

import Karm.Ui;
import Hideo.Settings;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Context& ctx, Async::CancellationToken ct) {
    co_return co_await Ui::runAsync(ctx, Hideo::Settings::app(), ct);
}
