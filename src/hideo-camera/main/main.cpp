#include <karm-sys/entry.h>

import Hideo.Camera;
import Karm.Ui;
import Karm.Av;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Context& ctx) {
    auto cam = co_try$(Av::Camera::openDefault());
    auto cap = co_try$(cam->startCapture());

    co_return co_await Ui::runAsync(
        ctx,
        Hideo::Camera::app(cam, cap)
    );
}
