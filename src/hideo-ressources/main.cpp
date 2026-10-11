#include <karm/entry>

import Karm.Ui;
import Hideo.Sysmon;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    auto app = Hideo::Resources::app();
    Async::detach(Hideo::Resources::refreshTask(app, ct));
    co_return co_await Ui::runAsync(env, app, ct);
}
