#include <karm/entry>

import Hideo.Text;
import Karm.Ui;
import Karm.Sys;

using namespace Karm;

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    auto& args = env.args();
    Opt<Ref::Url> url;
    Res<String> text = Ok(""s);
    if (args.len()) {
        url = Ref::parseUrlOrPath(args[0], env.cwd());
        text = Sys::readAllUtf8(*url);
    }
    co_return co_await Ui::runAsync(env, Hideo::Text::app(url, text), ct);
}
