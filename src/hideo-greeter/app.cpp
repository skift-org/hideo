export module Hideo.Greeter;

import Mdi;
import Karm.Core;
import Karm.Ui;
import Karm.Kira;
import Karm.App;
import Karm.Ref;
import Karm.Gfx;
import Karm.Math;
import Karm.Font;
import Karm.Image;

using namespace Karm;
using namespace Karm::Literals;
using namespace Karm::Ref::Literals;
using namespace Karm::Math::Literals;

namespace Hideo::Greeter {

struct Account {
    Opt<Rc<Gfx::Image>> avatar;
    String name;
};

struct State {
    Vec<Account> accounts;
    Opt<Account> selected = NONE;
    DateTime dateTime;
};

struct Back {};

struct Select {
    Account account;
};

struct Login {};

using Action = Union<Back, Select, Login>;

Ui::Task<Action> reduce(State& s, Action a) {
    a.visit(
        [&](Back) {
            s.selected = NONE;
        },
        [&](Select const& select) {
            s.selected = Some(select.account);
        },
        [&](Login) {

        }
    );

    return NONE;
}

using Model = Ui::Model<State, Action, reduce>;

Ui::Child userLogin() {
    return Ui::vflow(
               32_au,
               Kr::avatar(Karm::Image::loadOrFallback("bundle://hideo-images/images/geraldine.png"_url).expect(), 160_au),
               Ui::headlineLarge("Geraldine") | Ui::center(),
               Ui::hflow(
                   4_au,
                   Kr::input(Mdi::LOCK, "Password"s, ""s, Ui::SINK<String>) | Ui::box({.backgroundFill = Some(Ui::GRAY950)}) | Ui::grow(),
                   Ui::button(Some(Ui::SINK<>), Ui::ButtonStyle::regular(), Mdi::CHEVRON_RIGHT)
               ) | Ui::pinSize({260_au, Ui::UNCONSTRAINED})
           ) |
           Ui::insets(32_au);
}

Ui::Child appContent(State const&) {
    return Ui::stack(
        Ui::image("bundle://hideo-shell/wallpapers/abstract.qoi"_url) |
            Ui::cover() |
            Ui::grow() | Ui::foregroundFilter(Gfx::BlurFilter{16}),
        userLogin() | Ui::center()
    );
}

export Ui::Child app() {
    return Ui::reducer<Model>({}, [](State const& s) {
        return appContent(s) |
               Ui::pinSize(
                   App::formFactor == App::FormFactor::MOBILE
                       ? Math::Vec2Au{411_au, 731_au}
                       : Math::Vec2Au{1280_au, 720_au}
               ) |
               Ui::dialogLayer();
        ;
    });
}

} // namespace Hideo::Greeter
