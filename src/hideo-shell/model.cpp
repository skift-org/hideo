export module Hideo.Shell:model;

import Mdi;
import Karm.Ui;
import Karm.Core;
import Karm.App;
import Karm.Image;
import Karm.Gfx;
import Karm.Math;
import Karm.Glob;

using namespace Karm;
using namespace Karm::Literals;

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

export inline constexpr Math::Vec2i MIN_WINDOW_SIZE = {250, 150};

export struct Window {
    Math::Recti _activeBound = {100, 100, 600, 400};
    Math::Recti _floatingBound = {100, 100, 600, 400};

    bool dragged = false;
    bool resizing = false;
    Math::Vec2i resizeDir = {};
    App::CursorStyle cursor = App::CursorStyle::DEFAULT;
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
    String searchQuery = ""s;
    usize searchIndex = 0;

    DateTime dateTime;

    Rc<Gfx::Surface> background;
    Vec<Noti> noti;
    Vec<Rc<Launcher>> launchers;
    Vec<Rc<Launcher>> filtered;
    Vec<Rc<Window>> windows;

    void filter() {
        Vec<Tuple<Rc<Launcher>, int>> matches;
        for (auto l : launchers) {
            if (not searchQuery) {
                matches.pushBack({l, {}});
                continue;
            }

            auto match = Glob::matchFuzzy(l->name, searchQuery);
            if (not match)
                continue;
            matches.pushBack({l, match->score});
        }

        if (searchQuery)
            sort(matches, [](auto& a, auto& b) {
                return b.v1 <=> a.v1;
            });

        filtered = iter(matches) |
                   Select([](auto& m) {
                       return m.v0;
                   }) |
                   Collect<Vec<Rc<Launcher>>>();
    }

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

export struct UpdateSearch {
    String query;
};

export struct SelectSearch {
    int offset = 0;
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
    Rc<Launcher> launcher;
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

export struct StartDragWindow {
    Rc<Window> window;
};

export struct EndDragWindow {
    Rc<Window> window;
};

export struct StartResizeWindow {
    Rc<Window> window;
    Math::Vec2i dir;
};

export struct ResizeWindow {
    Rc<Window> window;
    Math::Vec2i off;
};

export struct EndResizeWindow {
    Rc<Window> window;
};

export struct FocusWindow {
    Rc<Window> window;
};

export struct ToggleSysPanel {};

export struct ActivatePanel {
    Panel panel;
};

export using Action = Union<
    UpdateSearch,
    SelectSearch,
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
    StartDragWindow,
    ResizeWindow,
    EndResizeWindow,
    StartResizeWindow,
    FocusWindow,
    ActivatePanel,
    ToggleSysPanel>;

Ui::Task<Action> reduce(State& s, Action a) {
    a.visit(
        [&](UpdateSearch u) {
            s.searchQuery = u.query;
            s.searchIndex = 0;
            s.filter();
        },
        [&](SelectSearch u) {
            if (s.searchIndex != 0 or u.offset != -1)
                s.searchIndex += u.offset;
            s.searchIndex = clampIndex(s.searchIndex, s.filtered.len());
        },
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
            start.launcher->launch(s);
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
        [&](StartDragWindow s) {
            s.window->dragged = true;
        },
        [&](EndDragWindow s) {
            s.window->dragged = false;
        },
        [&](StartResizeWindow s) {
            s.window->resizing = true;
            s.window->resizeDir = s.dir;
        },
        [&](ResizeWindow resize) {
            s.activePanel = Panel::NIL;
            auto dir = resize.window->resizeDir;
            auto bound = resize.window->_floatingBound;

            if (dir.x < 0)
                bound.start(min(bound.start() + resize.off.x, bound.end() - MIN_WINDOW_SIZE.x));
            else if (dir.x > 0)
                bound.end(max(bound.end() + resize.off.x, bound.start() + MIN_WINDOW_SIZE.x));

            if (dir.y < 0)
                bound.top(min(bound.top() + resize.off.y, bound.bottom() - MIN_WINDOW_SIZE.y));
            else if (dir.y > 0)
                bound.bottom(max(bound.bottom() + resize.off.y, bound.top() + MIN_WINDOW_SIZE.y));

            resize.window->_floatingBound = bound;
        },
        [&](EndResizeWindow s) {
            s.window->resizing = false;
        },
        [&](FocusWindow focus) {
            s.windows.removeAll(focus.window);
            s.windows.pushFront(focus.window);
            s.activePanel = Panel::NIL;
            s.updateFocus();
        },
        [&](ActivatePanel panel) {
            s.searchQuery = ""s;
            s.searchIndex = 0;
            s.filtered = s.launchers;
            s.activePanel = s.activePanel != panel.panel ? panel.panel : Panel::NIL;
        },
        [&](ToggleSysPanel) {
            s.isSysPanelColapsed = not s.isSysPanelColapsed;
        }
    );

    return NONE;
}

export using Model = Ui::Model<State, Action, reduce>;

export struct WindowFlipEvent {
    Rc<Window> window;
    Math::Recti region;
};

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
            g.blit(_bound.cast<isize>(), surface);
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

    Math::Vec2i _quadrantDir(Math::Vec2i pos) const {
        return {
            pos.x < _bound.center().x ? -1 : 1,
            pos.y < _bound.center().y ? -1 : 1,
        };
    }

    static App::CursorStyle _resizeCursor(Math::Vec2i dir) {
        if (dir.x and dir.y)
            return dir.x == dir.y
                       ? App::CursorStyle::RESIZE_NWSE
                       : App::CursorStyle::RESIZE_NESW;
        if (dir.x)
            return App::CursorStyle::RESIZE_EW;
        return App::CursorStyle::RESIZE_NS;
    }

    void event(App::Event& e) override {
        if (e.accepted())
            return;

        if (auto it = e.is<WindowFlipEvent>(); it and it->window == _window) {
            Ui::shouldRepaint(*this, it->region.offset(bound().topStart()));
        } else if (auto it = e.is<App::MouseEvent>(); it) {
            if (it->type == App::MouseEvent::RELEASE and _window->dragged) {
                Model::bubble<EndDragWindow>(*this, {_window});
                e.accept();
                return;
            }

            if (it->type == App::MouseEvent::MOVE and _window->dragged) {
                Model::bubble<DragWindow>(*this, {_window, it->delta});
                e.accept();
                return;
            }

            if (it->type == App::MouseEvent::RELEASE and _window->resizing) {
                Model::bubble<EndResizeWindow>(*this, {_window});
                e.accept();
                return;
            }

            if (it->type == App::MouseEvent::MOVE and _window->resizing) {
                Ui::bubble<App::RequestCursorEvent>(*this, _resizeCursor(_window->resizeDir));
                Model::bubble<ResizeWindow>(*this, {_window, it->delta});
                e.accept();
                return;
            }

            if (bound().contains(it->pos)) {
                // The client application decides where a resize can be
                // initiated and requests the matching cursor as the pointer
                // hovers these regions.
                if (it->type == App::MouseEvent::MOVE and _window->cursor != App::CursorStyle::DEFAULT)
                    Ui::bubble<App::RequestCursorEvent>(*this, _window->cursor);

                if (it->type == App::MouseEvent::PRESS and not _window->focused) {
                    Model::bubble<FocusWindow>(*this, {_window});
                }

                if (it->type == App::MouseEvent::PRESS and it->button == App::MouseButton::LEFT and App::match(it->mods, App::KeyMod::SUPER)) {
                    Model::bubble<StartDragWindow>(*this, {_window});
                    e.accept();
                }

                if (it->type == App::MouseEvent::PRESS and it->button == App::MouseButton::RIGHT and App::match(it->mods, App::KeyMod::SUPER) and _snap == App::Snap::NONE and not e.accepted()) {
                    Model::bubble<StartResizeWindow>(*this, {_window, _quadrantDir(it->pos)});
                    e.accept();
                }

                if (e.accepted())
                    return;

                auto transformedEvent = *it;
                transformedEvent.pos = transformedEvent.pos - bound().xy;
                auto ee = App::makeEvent<App::MouseEvent>(transformedEvent);
                _window->event(*ee);
                e.accept();
            }
        } else if (_window->focused) {
            if (e.is<App::KeyboardEvent>())
                _window->event(e);
        }
    }
};

// MARK: Mock ------------------------------------------------------------------

export struct MockWindow : Window {
    Gfx::Icon icon;
    String name;
    Gfx::ColorRamp ramp;

    MockWindow(Gfx::Icon icon, String name, Gfx::ColorRamp ramp)
        : icon(icon), name(name), ramp(ramp) {}

    Rc<Gfx::Surface> surface() const override {
        return Gfx::Surface::fallback();
    }

    void event(App::Event&) override {}
};

export struct MockLauncher : Launcher {
    using Launcher::Launcher;

    void launch(State& s) override {
        auto instance = makeRc<MockWindow>(
            icon,
            name,
            ramp
        );
        s.windows.emplaceFront(instance);
    }
};

} // namespace Hideo::Shell
