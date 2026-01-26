#include <karm/entry>

import Hideo.Greeter;
import Karm.Ui;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Context& ctx, Async::CancellationToken ct) {
    co_return co_await Ui::runAsync(
        ctx,
        Hideo::Greeter::app(),
        ct
    );
}
