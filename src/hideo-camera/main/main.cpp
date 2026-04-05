#include <karm/entry>

import Hideo.Camera;
import Karm.Ui;
import Karm.Av;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    auto cam = co_try$(Av::Camera::openDefault());
    auto cap = co_try$(cam->startCapture());

    co_return co_await Ui::runAsync(
        env,
        Hideo::Camera::app(cam, cap),
        ct
    );
}
