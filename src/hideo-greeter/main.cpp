#include <karm/entry>

import Hideo.Greeter;
import Karm.App;
import Karm.Ui;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    auto app = co_trya$(App::Application::createAsync(env, {.options = {App::ApplicationOptions::ROOT}}, ct));
    co_return co_await Ui::runAsync(
        app,
        Hideo::Greeter::app(),
        ct
    );
}
