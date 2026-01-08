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

export struct Window {
    Math::Recti bound = {100, 100, 600, 400};
    bool dragged = false;
    bool focused = false;

    Window() = default;

    virtual ~Window() = default;

    virtual Rc<Gfx::Surface> surface() const = 0;

    virtual void event(App::Event&) = 0;

    virtual void resize(Math::Vec2i size) {
        bound.wh = size;
    }

    bool operator==(Window const& other) const {
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
    bool keyboard = false;
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
    Vec<Rc<Window>> instances;

    void updateFocus() {
        if (not instances)
            return;

        for (auto& i : instances)
            i->focused = false;

        first(instances)->focused = true;
    }
};

export struct ToggleTablet {};

export struct ToggleKeyboard {};

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
    Rc<Window> instance;
};

export struct RemoveInstance {
    Rc<Window> instance;
};

export struct DragInstance {
    Rc<Window> window;
    Math::Vec2i off;
};

export struct InstanceDragEnd {};

export struct FocusInstance {
    Rc<Window> window;
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
    ToggleKeyboard,
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
    FocusInstance,
    Activate,
    ToggleSysPanel,
    ToggleAppThumbnail>;

Ui::Task<Action> reduce(State& s, Action a) {
    a.visit(Visitor{
        [&](ToggleTablet) {
            if (App::formFactor == App::FormFactor::MOBILE) {
                App::formFactor = App::FormFactor::DESKTOP;
            } else {
                App::formFactor = App::FormFactor::MOBILE;
            }
            s.activePanel = Panel::NIL;
            s.isSysPanelColapsed = true;
        },
        [&](ToggleNightLight) {
            s.nightLight = not s.nightLight;
        },
        [&](ToggleKeyboard) {
            s.keyboard = not s.keyboard;
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
            auto bound = move.window->bound;
            bound.xy = bound.xy + move.off;
            move.window->bound = bound;
        },
        [&](InstanceDragEnd) {
            first(s.instances)->dragged = false;
        },
        [&](FocusInstance focus) {
            s.instances.removeAll(focus.window);
            s.instances.pushFront(focus.window);
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

export struct Viewport : Ui::View<Viewport> {
    Rc<Window> _window;
    bool _primary;
    Math::Radiif _radii;

    Viewport(Rc<Window> window, bool primary, Math::Radiif radii)
        : _window(window), _primary(primary), _radii(radii) {}

    void reconcile(Viewport& o) override {
        _window = o._window;
        _radii = o._radii;
    }

    void paint(Gfx::Canvas& g, Math::Recti) override {
        auto surface = _window->surface();
        g.push();
        if (not _radii.zero()) {
            g.fillStyle(surface->pixels());
            g.fill(bound(), _radii);
        } else {
            g.blit(_bound.cast<isize>(), surface->pixels());
        }
        g.pop();
    }

    void layout(Math::Recti rect) override {
        if (_primary)
            _window->resize(rect.wh);
        View::layout(rect);
    }

    Math::Vec2i size(Math::Vec2i, Ui::Hint) override {
        return _window->bound.size();
    }

    void event(App::Event& e) override {
        if (auto c = e.is<App::RequestCloseEvent>()) {
            e.accept();
            Model::bubble<RemoveInstance>(*this, {_window});
        } else if (auto it = e.is<App::MouseEvent>(); it) {
            if (_window->dragged) {
                if (it->type == App::MouseEvent::RELEASE) {
                    Model::bubble<InstanceDragEnd>(*this);
                } else if (it->type == App::MouseEvent::MOVE) {
                    Model::bubble<DragInstance>(*this, {_window, it->delta});
                    e.accept();
                    return;
                }
            }

            if (bound().contains(it->pos)) {
                if (_window->focused) {
                    auto transformedEvent = *it;
                    transformedEvent.pos = transformedEvent.pos - bound().xy;
                    auto ee = App::makeEvent<App::MouseEvent>(transformedEvent);
                    _window->event(ee);
                } else if (it->type == App::MouseEvent::PRESS and not _window->focused) {
                    Model::bubble<FocusInstance>(*this, {_window});
                }
                e.accept();
            }
        }
    }
};

} // namespace Hideo::Shell
