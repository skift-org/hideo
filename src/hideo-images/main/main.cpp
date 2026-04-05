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

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    auto& args = env.args();
    Res<Rc<Gfx::Surface>> image = Error::invalidInput("No image provided");

    if (args.len()) {
        auto url = Ref::parseUrlOrPath(args[0], env.cwd());
        image = Image::load(url);

        if (not image) {
            logError("Failed to load image: {}", image.none());
        }
    }

    co_return co_await Ui::runAsync(env, Hideo::Images::app(image.unwrap()), ct);
}
