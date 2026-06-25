export module Hideo.Shell:lock;

import Mdi;
import Karm.Ui;
import Karm.Font;
import Karm.Ref;
import Karm.Gfx;
import Karm.Math;

import :model;
import :background;

using namespace Karm::Ref::Literals;

namespace Hideo::Shell {

static Opt<Rc<Gfx::Fontface>> _blackFontface = NONE;

static Rc<Gfx::Fontface> blackFontface() {
    if (not _blackFontface) {
        _blackFontface = Font::loadFontfaceOrFallback("bundle://fonts.inter/fonts/Inter-Bold.ttf"_url).unwrap();
    }
    return *_blackFontface;
}

Ui::Child lockScreen(State const& state) {
    auto [date, time] = state.dateTime;
    auto dateTime = Io::format(
        // Mon, 28 Jul
        "{}, {} {}",
        Io::toCapitalCase(date.dayOfWeek().str()),
        date.dayOfMonth() + 1,
        Io::toCapitalCase(date.month.str())
    );

    auto clock = Ui::vflow(
        0,
        Math::Align::CENTER,
        Ui::text(Gfx::ProseProps{blackFontface()}.withFontSize(16), dateTime),
        Ui::text(
            Gfx::ProseProps{blackFontface()}.withFontSize(72),
            "{02}:{02}",
            time.hour, time.minute
        )
    );

    auto hintText = Ui::vflow(
        Ui::center(Ui::icon(Mdi::CHEVRON_UP, 48)),
        Ui::center(Ui::labelLarge(App::formFactor == App::FormFactor::MOBILE ? "Swipe up to unlock" : "Swipe up or press the space key to unlock"))
    );

    return Ui::stack(
        background(state),
        Ui::vflow(clock, Ui::grow(NONE), hintText | Ui::slideIn(Ui::SlideFrom::BOTTOM)) |
            Ui::insets(App::formFactor == App::FormFactor::MOBILE ? 64 : 128) |
            Ui::dragRegion() |
            Ui::keyboardShortcut(App::Key::SPACE, Model::bind<Unlock>()) |
            Ui::dismisable(Model::bind<Unlock>(), Ui::DismisDir::TOP, 0.3) |
            Ui::align(Math::Align::VFILL | Math::Align::HCENTER)
    );
}

} // namespace Hideo::Shell
