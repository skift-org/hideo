#include <karm-sys/entry.h>

import Hideo.Code;
import Karm.Ui;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Context& ctx, Async::CancellationToken ct) {
    co_return co_await Ui::runAsync(
        ctx,
        Hideo::Code::app(),
        ct
    );
}
