export module Hideo.Shell:model;

import Mdi;
import Karm.Ui;
import Karm.Core;
import Karm.App;
import Karm.Image;
import Karm.Gfx;
import Karm.Math;

using namespace Karm;

namespace Hideo::Shell {

export struct State;

// MARK: Notification ----------------------------------------------------------

export struct Noti {
    usize id;
    String title;
    String body;
    Vec<String> actions{};
};

// MARK: Manifest & Instance ---------------------------------------------------

export struct Launcher {
    Gfx::Icon icon = Mdi::APPLICATION;
    String name;
    Gfx::ColorRamp ramp;

    Launcher(Gfx::Icon icon, String name, Gfx::ColorRamp ramp)
        : icon(icon), name(name), ramp(ramp) {}

    virtual ~Launcher() = default;

    virtual void launch(State&) = 0;
};

export struct Instance {
    Math::Recti bound = {100, 100, 600, 400};
    bool dragged = false;
    bool focused = false;

    Instance() = default;

    virtual ~Instance() = default;

    virtual Ui::Child build() const = 0;

    virtual Rc<Gfx::Surface> thumbnail() const = 0;

    bool operator==(Instance const& other) const {
        return this == &other;
    }
};

// MARK: Model -----------------------------------------------------------------

export enum struct Panel {
    NIL,
    APPS,
    NOTIS,
    SYS,
};

export struct State : Meta::NoCopy {
    bool locked = true;
    bool isMobile = true;
    bool nightLight = false;
    f64 brightness = 1;
    f64 volume = 0.5;
    Panel activePanel = Panel::NIL;
    bool isSysPanelColapsed = true;
    bool isAppPanelThumbnails = false;

    DateTime dateTime;

    Rc<Gfx::Surface> background;
    Vec<Noti> noti;
    Vec<Rc<Launcher>> launchers;
    Vec<Rc<Instance>> instances;

    void updateFocus() {
        if (not instances)
            return;

        for (auto& i : instances)
            i->focused = false;

        first(instances)->focused = true;
    }
};

export struct ToggleTablet {};

export struct ToggleNightLight {};

export struct ChangeBrightness {
    f64 value;
};

export struct ChangeVolume {
    f64 value;
};

export struct Lock {};

export struct Unlock {};

export struct DimisNoti {
    usize index;
};

export struct StartInstance {
    usize index;
};

export struct AddInstance {
    Rc<Instance> instance;
};

export struct RemoveInstance {
    Rc<Instance> instance;
};

export struct DragInstance {
    usize index;
    Math::Vec2i off;
};

export struct InstanceDragEnd {};

export struct CloseInstance {
    usize index;
};

export struct FocusInstance {
    usize index;
};

export struct ToggleSysPanel {};

export struct ToggleAppThumbnail {
    bool value;
};

export struct Activate {
    Panel panel;
};

export using Action = Union<
    ToggleTablet,
    ToggleNightLight,
    ChangeBrightness,
    ChangeVolume,
    Lock,
    Unlock,
    DimisNoti,
    StartInstance,
    AddInstance,
    RemoveInstance,
    DragInstance,
    InstanceDragEnd,
    CloseInstance,
    FocusInstance,
    Activate,
    ToggleSysPanel,
    ToggleAppThumbnail>;

Ui::Task<Action> reduce(State& s, Action a) {
    a.visit(Visitor{
        [&](ToggleTablet) {
            s.isMobile = not s.isMobile;
            s.activePanel = Panel::NIL;
            s.isSysPanelColapsed = true;
        },
        [&](ToggleNightLight) {
            s.nightLight = not s.nightLight;
        },
        [&](ChangeBrightness m) {
            s.brightness = m.value;
        },
        [&](ChangeVolume m) {
            s.volume = m.value;
        },
        [&](Lock) {
            s.locked = true;
            s.activePanel = Panel::NIL;
        },
        [&](Unlock) {
            s.locked = false;
        },
        [&](DimisNoti dismis) {
            s.noti.removeAt(dismis.index);
        },
        [&](StartInstance start) {
            s.launchers[start.index]->launch(s);
            s.activePanel = Panel::NIL;
        },
        [&](AddInstance add) {
            s.instances.pushFront(add.instance);
            s.updateFocus();
        },
        [&](RemoveInstance rem) {
            s.instances.removeAll(rem.instance);
            s.updateFocus();
        },
        [&](DragInstance move) {
            s.activePanel = Panel::NIL;
            auto bound = s.instances[move.index]->bound;
            bound.xy = bound.xy + move.off;
            s.instances[move.index]->bound = bound;
        },
        [&](InstanceDragEnd) {
            first(s.instances)->dragged = false;
        },
        [&](CloseInstance close) {
            s.instances.removeAt(close.index);
            s.updateFocus();
        },
        [&](FocusInstance focus) {
            auto instance = s.instances.removeAt(focus.index);
            s.instances.pushFront(instance);
            s.activePanel = Panel::NIL;
            s.updateFocus();
        },
        [&](Activate panel) {
            if (s.activePanel != panel.panel) {
                s.activePanel = panel.panel;
            } else {
                s.activePanel = Panel::NIL;
            }
        },
        [&](ToggleSysPanel) {
            s.isSysPanelColapsed = not s.isSysPanelColapsed;
        },
        [&](ToggleAppThumbnail a) {
            s.isAppPanelThumbnails = a.value;
        },
    });

    return NONE;
}

export using Model = Ui::Model<State, Action, reduce>;

} // namespace Hideo::Shell
