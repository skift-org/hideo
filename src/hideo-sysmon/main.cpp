#include <karm/entry>

import Karm.Ui;
import Hideo.Sysmon;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    auto app = Hideo::Sysmon::app();
    Async::detach(Hideo::Sysmon::refreshTask(app, ct));
    co_return co_await Ui::runAsync(env, app, ct);
}
