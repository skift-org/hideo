#include <karm/entry>

import Hideo.Images;
import Karm.Image;
import Karm.Kira;
import Karm.Ui;
import Karm.Sys;
import Karm.Gfx;
import Karm.Logger;

import Mdi;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Context& ctx, Async::CancellationToken ct) {
    auto& args = useArgs(ctx);
    Res<Rc<Gfx::Surface>> image = Error::invalidInput("No image provided");

    if (args.len()) {
        auto url = Ref::parseUrlOrPath(args[0], co_try$(Sys::pwd()));
        image = Image::load(url);

        if (not image) {
            logError("Failed to load image: {}", image.none());
        }
    }

    co_return co_await Ui::runAsync(ctx, Hideo::Images::app(image.unwrap()), ct);
}
