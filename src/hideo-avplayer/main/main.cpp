#include <karm-sys/entry.h>

import Hideo.Avplayer;
import Karm.Ui;
import Karm.Av;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Context& ctx) {
    auto url = "bundle://hideo-avplayer/audio/free-software.wav"_url;
    auto device = co_try$(Av::Device::create());
    auto player = makeRc<Av::Player>();
    auto audio = co_try$(Av::load(url));
    player->play(audio);
    device->play(player);
    device->pause(false);

    auto [cancelation, token] = Async::Cancellation::create();
    auto app = Hideo::Avplayer::app(player);
    Async::detach(Hideo::Avplayer::updatePlayback(app, token));

    co_return co_await Ui::runAsync(ctx, app);
}
