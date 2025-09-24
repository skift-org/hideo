export module Hideo.Avplayer:model;

import Karm.Core;
import Karm.Av;
import Karm.Ui;

using namespace Karm;

namespace Hideo::Avplayer {

struct State {
    Rc<Av::Player> player;
    Res<Rc<Av::Audio>> audio;
};

export struct Update {};

export struct TogglePause {};

export struct ToggleMute {};

export struct Previous {};

export struct ChangeVolume {
    f64 value;
};

export struct Scrub {
    Duration duration;
};

using Action = Union<
    Update,
    TogglePause,
    ToggleMute,
    Previous,
    ChangeVolume,
    Scrub>;

Ui::Task<Action> reduce(State& s, Action a) {
    a.visit(Visitor{
        [&](Update) {
        },
        [&](TogglePause) {
            s.player->pause(not s.player->pause());
        },
        [&](ToggleMute) {
            s.player->mute(not s.player->mute());
        },
        [&](Previous) {
            s.player->seek(Duration::fromSecs(0));
        },
        [&](ChangeVolume c) {
            s.player->volume(c.value);
        },
        [&](Scrub m) {
            s.player->seek(m.duration);
        },
    });

    return NONE;
}

export using Model = Ui::Model<State, Action, reduce>;

} // namespace Hideo::Avplayer
