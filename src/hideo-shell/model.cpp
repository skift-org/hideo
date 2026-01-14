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
    Math::Recti _activeBound = {100, 100, 600, 400};
    Math::Recti _floatingBound = {100, 100, 600, 400};

    bool dragged = false;
    bool focused = false;
    App::Snap preferSnap = App::Snap::NONE;

    Window() = default;

    virtual ~Window() = default;

    virtual Rc<Gfx::Surface> surface() const = 0;

    virtual void event(App::Event&) = 0;

    Math::Recti bound(App::Snap snap) {
        return snap == App::Snap::NONE ? _floatingBound : _activeBound;
    }

    Math::Recti activeBound() {
        return _activeBound;
    }

    virtual void resize(App::Snap snap, Math::Vec2i size) {
        if (snap == App::Snap::NONE) {
            _floatingBound.wh = size;
        }
        _activeBound.wh = size;
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

    DateTime dateTime;

    Rc<Gfx::Surface> background;
    Vec<Noti> noti;
    Vec<Rc<Launcher>> launchers;
    Vec<Rc<Window>> windows;

    void updateFocus() {
        if (not windows)
            return;

        for (auto& i : windows)
            i->focused = false;

        first(windows)->focused = true;
    }

    bool hasFullWindow() const {
        if (App::formFactor == App::FormFactor::MOBILE)
            return windows.len() != 0;

        for (auto& w : windows) {
            if (w->preferSnap == App::Snap::FULL)
                return true;
        }
        return false;
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

export struct StartApplication {
    usize index;
};

export struct AddWindow {
    Rc<Window> window;
};

export struct RemoveWindow {
    Rc<Window> window;
};

export struct SnapWindow {
    Rc<Window> window;
    App::Snap snap;
};

export struct DragWindow {
    Rc<Window> window;
    Math::Vec2i off;
};

export struct EndDragWindow {};

export struct FocusWindow {
    Rc<Window> window;
};

export struct ToggleSysPanel {};

export struct ActivatePanel {
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
    StartApplication,
    AddWindow,
    RemoveWindow,
    SnapWindow,
    DragWindow,
    EndDragWindow,
    FocusWindow,
    ActivatePanel,
    ToggleSysPanel>;

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
        [&](StartApplication start) {
            s.launchers[start.index]->launch(s);
            s.activePanel = Panel::NIL;
        },
        [&](AddWindow add) {
            s.windows.pushFront(add.window);
            s.updateFocus();
        },
        [&](RemoveWindow rem) {
            s.windows.removeAll(rem.window);
            s.updateFocus();
        },
        [&](DragWindow move) {
            s.activePanel = Panel::NIL;
            auto bound = move.window->_floatingBound;
            bound.xy = bound.xy + move.off;
            move.window->_floatingBound = bound;
        },
        [&](SnapWindow s) {
            s.window->preferSnap = s.snap;
        },
        [&](EndDragWindow) {
            first(s.windows)->dragged = false;
        },
        [&](FocusWindow focus) {
            s.windows.removeAll(focus.window);
            s.windows.pushFront(focus.window);
            s.activePanel = Panel::NIL;
            s.updateFocus();
        },
        [&](ActivatePanel panel) {
            s.activePanel = s.activePanel != panel.panel ? panel.panel : Panel::NIL;
        },
        [&](ToggleSysPanel) {
            s.isSysPanelColapsed = not s.isSysPanelColapsed;
        },
    });

    return NONE;
}

export using Model = Ui::Model<State, Action, reduce>;

export struct Viewport : Ui::View<Viewport> {
    Rc<Window> _window;
    bool _primary;
    App::Snap _snap;
    Math::Radiif _radii;

    Viewport(Rc<Window> window, bool primary, App::Snap snap, Math::Radiif radii)
        : _window(window), _primary(primary), _snap(snap), _radii(radii) {}

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
            _window->resize(_snap, rect.wh);
        View::layout(rect);
    }

    Math::Vec2i size(Math::Vec2i, Ui::Hint) override {
        return _window->bound(_snap).size();
    }

    void event(App::Event& e) override {
        if (auto c = e.is<App::RequestCloseEvent>()) {
            e.accept();
            Model::bubble<RemoveWindow>(*this, {_window});
        } else if (auto it = e.is<App::MouseEvent>(); it) {
            if (_window->dragged) {
                if (it->type == App::MouseEvent::RELEASE) {
                    Model::bubble<EndDragWindow>(*this);
                } else if (it->type == App::MouseEvent::MOVE) {
                    Model::bubble<DragWindow>(*this, {_window, it->delta});
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
                    Model::bubble<FocusWindow>(*this, {_window});
                }
                e.accept();
            }
        } else if (_window->focused) {
            if (e.is<App::KeyboardEvent>() or e.is<App::TypeEvent>())
                _window->event(e);
        }
    }
};

} // namespace Hideo::Shell
