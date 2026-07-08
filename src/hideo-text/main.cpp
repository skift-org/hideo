#include <karm/entry>

import Hideo.Text;
import Karm.Ui;
import Karm.Sys;

using namespace Karm;
using namespace Karm::Literals;

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    Opt<Ref::Url> url;
    Res<String> text = Ok(""s);
    if (env.argsLen()) {
        url = Ref::parseUrlOrPath(env[0], env.cwd());
        text = Sys::readAllText<Utf8>(*url);
    }
    co_return co_await Ui::runAsync(env, Hideo::Text::app(url, text), ct);
}
