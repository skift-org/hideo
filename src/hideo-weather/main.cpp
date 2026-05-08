#include <karm/entry>

import Mdi;
import Karm.Ui;
import Karm.Kira;
import Karm.Image;

using namespace Karm;
using namespace Karm::Literals;
using namespace Karm::Ref::Literals;

namespace Hideo::Weather {

Ui::Child app() {
    return Kr::scaffold({
        .icon = Mdi::WEATHER_PARTLY_CLOUDY,
        .title = "Weather"s,
        .body = [&] {
            auto image = Image::load("bundle://hideo-weather/images/weather-few-clouds.jpg"_url).unwrap();
            return Ui::image(image) | Ui::cover() | Ui::vhclip();
        },
    });
}

} // namespace Hideo::Weather

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    co_return co_await Ui::runAsync(env, Hideo::Weather::app(), ct);
}
