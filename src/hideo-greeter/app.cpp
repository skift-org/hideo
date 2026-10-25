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

struct Session {
    String name;
    Ref::Url url;
};

struct State {
    Rc<Gfx::Image> avatar;
    Rc<Gfx::Image> background;
    Vec<Session> sessions = {
        Session{"Desktop"s, "bundle://strata-shell"_url}
    };
    usize selectedSession = 0;
};

struct SelectSession {
    usize index;
};

struct Login {};

using Action = Union<SelectSession, Login>;

Ui::Task<Action> reduce(State& s, Action action) {
    action.visit(
        [&](SelectSession a) {
            s.selectedSession = a.index;
        },
        [&](Login) {

        }
    );

    return NONE;
}

using Model = Ui::Model<State, Action, reduce>;

Ui::Child userLogin(State const& s) {
    return Ui::vflow(
               32_au,
               Kr::avatar(s.avatar, 160_au),
               Ui::hflow(
                   4_au,
                   Kr::select(
                       Kr::selectValue("Desktop"s),
                       [] -> Ui::Children {
                           return {
                               Kr::selectGroup({
                                   Kr::selectItem(Some(Ui::SINK<>), "Desktop"s),
                                   Kr::selectItem(Some(Ui::SINK<>), "Mobile"s),
                                   Kr::selectItem(Some(Ui::SINK<>), "Gaming"s),
                                   Kr::selectItem(Some(Ui::SINK<>), "Wearable"s),
                               }),
                           };
                       }
                   ) | Ui::grow(),
                   Ui::button(Some(Ui::SINK<>), Ui::ButtonStyle::primary(), Mdi::CHEVRON_RIGHT)

               ) | Ui::pinSize({260_au, Ui::UNCONSTRAINED})
           ) |
           Ui::insets({16_au});
}

Ui::Child appContent(State const& s) {
    return Ui::stack(
        Ui::image(s.background) |
            Ui::cover() |
            Ui::foregroundFilter(Gfx::OverlayFilter{Ui::GRAY950.withOpacity(0.5)}) |
            Ui::grow(),
        userLogin(s) | Ui::center()
    );
}

export Ui::Child app() {
    return Ui::reducer<Model>(
               {
                   Image::loadOrFallback("bundle://hideo-greeter/public/profiles/avocado.png"_url).expect(),
                   Image::loadOrFallback("bundle://hideo-shell/wallpapers/abstract.qoi"_url).expect(),
               },
               [](State const& s) {
                   return appContent(s) |
                          Ui::pinSize(
                              App::formFactor == App::FormFactor::MOBILE
                                  ? Math::Vec2Au{411_au, 731_au}
                                  : Math::Vec2Au{1280_au, 720_au}
                          ) |
                          Ui::dialogLayer() |
                          Ui::popoverLayer();
               }
           ) |
           Kr::pointer();
}

} // namespace Hideo::Greeter
