#include <karm/entry>

import Karm.Logger;
import Karm.Ui;
import Karm.Av;

import Hideo.Avplayer;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    Res<Rc<Av::Audio>> audio = Error::invalidInput("No media provided");

    if (env.argsLen()) {
        auto url = Ref::parseUrlOrPath(env[0], env.cwd());
        audio = Av::load(url);

        if (not audio) {
            logError("Failed to load image: {}", audio.none());
        }
    }

    auto device = co_try$(Av::Device::create());
    auto player = makeRc<Av::Player>();
    if (audio)
        player->play(audio.unwrap());
    device->play(player);
    device->pause(false);

    auto app = Hideo::Avplayer::app(player, audio);
    Async::detach(Hideo::Avplayer::updatePlayback(app, ct));

    co_return co_await Ui::runAsync(env, app, ct);
}
