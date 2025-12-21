#include <karm-sys/entry.h>

import Karm.Ui;
import Hideo.Sysmon;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Context& ctx, Async::CancellationToken ct) {
    co_return co_await Ui::runAsync(ctx, Hideo::Sysmon::app(), ct);
}
