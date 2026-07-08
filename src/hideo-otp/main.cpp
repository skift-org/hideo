#include <karm/entry>

import Hideo.Otp;
import Karm.Ui;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    auto app = Hideo::Otp::app();
    Async::detach(Hideo::Otp::updateTask(app, ct));
    co_return co_await Ui::runAsync(env, app, ct);
}
