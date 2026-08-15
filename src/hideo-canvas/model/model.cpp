module;

#include <karm/macros>

export module Hideo.Canvas.Model:model;

import Karm.Core;
import Karm.Math;
import Karm.Ui;
import Karm.App;

import Hideo.Canvas.Freehand;

export import :bound;
export import :gizmo;
export import :node;
export import :selection;
export import :tree;

using namespace Karm;

namespace Hideo::Canvas {

export enum struct Tool {
    SELECT,
    FREEHAND,
    FRAME,
    TEXT,
    RECT,
};

export struct SelectTool {
    Tool tool;
};

export struct SelectAll {};

export struct CopySelection {};

export struct CutSelection {};

export struct PasteSelection {};

export struct DeleteSelection {};

export struct FrameSelection {};

export struct ChooseFreeHandColor {
    Gfx::Color color;
};

export struct CanvasPress {
    Math::Vec2f pos;
    bool resize = false;
};

export struct CanvasRelease {
    Math::Vec2f pos;
};

export struct CanvasDrag {
    Math::Vec2f pos;
    Flags<App::KeyMod> mods;
};

export struct ToggleProperties {};

export using Action = Union<
    ToggleProperties,
    SelectTool,
    SelectAll,
    CopySelection,
    CutSelection,
    PasteSelection,
    FrameSelection,
    DeleteSelection,
    ChooseFreeHandColor,
    CanvasPress,
    CanvasRelease,
    CanvasDrag>;

export struct State;

struct DragMode {
    virtual ~DragMode() = default;

    virtual Opt<Rc<DragMode>> reduce(State& s, Action a) = 0;

    virtual Opt<Gizmo> gizmo(State const&) const {
        return NONE;
    }

    virtual Opt<Math::Rectf> selectionRect() const {
        return NONE;
    }
};

export struct State {
    Tool currentTool = Tool::SELECT;
    Opt<Rc<DragMode>> dragMode;
    Tree tree;
    Opt<Tree> clipboard = NONE;
    Selection selection;
    Gfx::Color freehandColor = Gfx::WHITE;
    bool propertiesVisible = false;

    Opt<Gizmo> gizmo() const {
        if (not dragMode)
            return NONE;

        return dragMode.unwrap()->gizmo(*this);
    }

    Opt<Math::Rectf> selectionRect() const {
        if (not dragMode)
            return NONE;

        return dragMode.unwrap()->selectionRect();
    }
};

Rc<DragMode> makeIdleDragMode();

struct PlacingDragMode final : DragMode {
    Ref _ref;
    Math::Vec2f _start;
    Math::Vec2f _end;

    PlacingDragMode(Ref ref, Math::Vec2f start, Math::Vec2f end)
        : _ref(ref), _start(start), _end(end) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            auto& n = s.tree.byRef(_ref);
            if (App::match(drag->mods, App::KeyMod::SHIFT)) {
                auto delta = drag->pos - _start;
                n.bound = Obb{Math::Rectf::fromTwoPoint(_start, _start + delta.snapToDiagonal())};
            } else {
                n.bound = Obb{Math::Rectf::fromTwoPoint(_start, drag->pos)};
            }
            return Some(makeRc<PlacingDragMode>(_ref, _start, drag->pos));
        }

        if (auto release = a.is<CanvasRelease>()) {
            if (_start.dist(_end) < 2) {
                auto& n = s.tree.byRef(_ref);
                n.bound = Obb{Math::Rectf::fromTwoPoint(release->pos, release->pos).grow(50)};
            }

            s.currentTool = Tool::SELECT;
            return Some(makeIdleDragMode());
        }

        return Some(makeRc<PlacingDragMode>(_ref, _start, _end));
    }
};

struct FreehandDragMode final : DragMode {
    Ref _ref;
    Math::Vec2f _min;
    Math::Vec2f _max;

    FreehandDragMode(Ref ref, Math::Vec2f start)
        : _ref(ref), _min(start), _max(start) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            auto& n = s.tree.byRef(_ref);
            _min = _min.min(drag->pos);
            _max = _max.max(drag->pos);
            n.freehand.pushBack({drag->pos});
            n.bound = Obb{Math::Rectf::fromTwoPoint(_min, _max)};
        }

        if (a.is<CanvasRelease>())
            return Some(makeIdleDragMode());

        return NONE;
    }
};

struct ResizingDragMode final : DragMode {
    Gizmo _gizmo;
    GizmoHandle _handle;
    Math::Vec2f _pivot;

    ResizingDragMode(Gizmo gizmo, GizmoHandle handle)
        : _gizmo(gizmo), _handle(handle), _pivot(gizmo.oppositePivot(handle)) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            auto scale = _gizmo.resizeScale(_handle, drag->pos, App::match(drag->mods, App::KeyMod::SHIFT));
            s.selection.resize(s.tree, _gizmo.bound, _pivot, scale);
            return Some(makeRc<ResizingDragMode>(_gizmo, _handle));
        }

        if (a.is<CanvasRelease>()) {
            s.selection.commitTransform(s.tree);
            s.currentTool = Tool::SELECT;
            return Some(makeIdleDragMode());
        }

        return Some(makeRc<ResizingDragMode>(_gizmo, _handle));
    }

    Opt<Gizmo> gizmo(State const& s) const override {
        return s.selection.createGizmo(s.tree);
    }
};

struct RotatingSelectionDragMode final : DragMode {
    Math::Vec2f center;
    f64 startAngle;

    RotatingSelectionDragMode(Math::Vec2f center, f64 startAngle)
        : center(center), startAngle(startAngle) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            auto angle = Math::atan2(drag->pos.y - center.y, drag->pos.x - center.x);
            auto delta = angle - startAngle;

            s.selection.rotate(s.tree, center, delta);
            return NONE;
        }

        if (a.is<CanvasRelease>()) {
            s.selection.commitTransform(s.tree);
            s.currentTool = Tool::SELECT;
            return Some(makeIdleDragMode());
        }

        return NONE;
    }

    Opt<Gizmo> gizmo(State const& s) const override {
        return s.selection.createGizmo(s.tree);
    }
};

struct SelectingDragMode final : DragMode {
    Math::Vec2f start;
    Math::Vec2f end;

    SelectingDragMode(Math::Vec2f start)
        : start(start), end(start) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            end = drag->pos;
            s.selection.set(s.tree, s.tree.objectAt(Math::Rectf::fromTwoPoint(start, drag->pos), start));
            return NONE;
        }

        if (a.is<CanvasRelease>()) {
            s.currentTool = Tool::SELECT;
            return Some(makeIdleDragMode());
        }

        return NONE;
    }

    Opt<Math::Rectf> selectionRect() const override {
        return Some(Math::Rectf::fromTwoPoint(start, end));
    }
};

struct MovingSelectionDragMode final : DragMode {
    Math::Vec2f start;
    Opt<Ref> parent;

    MovingSelectionDragMode(Math::Vec2f start, Opt<Ref> parent = NONE)
        : start(start), parent(parent) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            s.selection.move(s.tree, drag->pos - start);

            auto ignored = s.tree.descendantsOf(s.selection.roots());
            Opt<Ref> newParent = s.tree.frameAt(drag->pos, ignored);
            if (newParent != parent) {
                s.tree.reparentRoots(s.selection.roots(), newParent);
                parent = newParent;
            }
        }

        if (a.is<CanvasRelease>()) {
            s.selection.commitTransform(s.tree);
            s.currentTool = Tool::SELECT;
            return Some(makeIdleDragMode());
        }

        return Some(makeRc<MovingSelectionDragMode>(start, parent));
    }
};

struct IdleDragMode final : DragMode {
    static Kind _toolToKind(Tool tool) {
        switch (tool) {
        case Tool::FRAME:
            return Kind::FRAME;
        case Tool::FREEHAND:
            return Kind::FREEHAND;
        case Tool::TEXT:
            return Kind::TEXT;
        case Tool::RECT:
            return Kind::RECT;
        case Tool::SELECT:
            return Kind::GROUP;
        default:
            unreachable();
        }
    }

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto press = a.is<CanvasPress>()) {
            if (s.currentTool != Tool::SELECT) {
                auto parent = s.tree.frameAt(press->pos);
                auto ref = s.tree.insert(
                    _toolToKind(s.currentTool),
                    Obb{Math::Rectf::fromTwoPoint(press->pos, press->pos)},
                    parent
                );

                if (s.currentTool == Tool::FREEHAND) {
                    s.tree.byRef(ref).freehandColor = s.freehandColor;
                    return Some(makeRc<FreehandDragMode>(ref, press->pos));
                } else {
                    s.selection.set(s.tree, {ref});
                    return Some(makeRc<PlacingDragMode>(ref, press->pos, press->pos));
                }
            }

            if (auto gizmo = s.selection.createGizmo(s.tree); gizmo) {
                auto [hitHandle, rotate] = gizmo->hitHandle(press->pos);

                if (rotate) {
                    s.selection.beginTransform(s.tree);
                    auto startAngle = Math::atan2(press->pos.y - gizmo->bound.center.y, press->pos.x - gizmo->bound.center.x);
                    return Some(makeRc<RotatingSelectionDragMode>(gizmo->bound.center, startAngle));
                }

                if (hitHandle != GizmoHandle::NONE) {
                    s.selection.beginTransform(s.tree);
                    return Some(makeRc<ResizingDragMode>(*gizmo, hitHandle));
                }
            }

            if (auto ref = s.tree.objectAt(press->pos); ref) {
                auto const pressedRef = ref.unwrap();
                if (not s.selection.selected(pressedRef)) {
                    s.selection.set(s.tree, {pressedRef});
                }

                if (press->resize) {
                    if (auto gizmo = s.selection.createGizmo(s.tree); gizmo) {
                        s.selection.beginTransform(s.tree);
                        return Some(makeRc<ResizingDragMode>(*gizmo, GizmoHandle::SE));
                    }
                }

                s.selection.beginTransform(s.tree);
                return Some(makeRc<MovingSelectionDragMode>(press->pos));
            }

            if (auto frame = s.tree.frameAt(press->pos); frame and s.selection.selected(frame.unwrap())) {
                s.selection.beginTransform(s.tree);
                return Some(makeRc<MovingSelectionDragMode>(press->pos));
            }

            s.selection.unselectAll();
            return Some(makeRc<SelectingDragMode>(press->pos));
        }

        return Some(makeIdleDragMode());
    }

    Opt<Gizmo> gizmo(State const& s) const override {
        return s.selection.createGizmo(s.tree);
    }
};

Rc<DragMode> makeIdleDragMode() {
    return makeRc<IdleDragMode>();
}

Ui::Task<Action> reduce(State& s, Action action) {
    if (action.is<ToggleProperties>()) {
        s.propertiesVisible = not s.propertiesVisible;
    } else if (auto a = action.is<SelectTool>()) {
        s.currentTool = a->tool;
    } else if (action.is<SelectAll>()) {
        s.selection.selectAll(s.tree);
    } else if (auto a = action.is<CopySelection>()) {
        if (not s.selection.empty())
            s.clipboard = Some(s.selection.copy(s.tree));
    } else if (auto a = action.is<CutSelection>()) {
        if (not s.selection.empty()) {
            s.clipboard = Some(s.selection.cut(s.tree));
            s.dragMode = Some(makeIdleDragMode());
        }
    } else if (auto a = action.is<PasteSelection>()) {
        if (s.clipboard) {
            s.selection.paste(s.tree, s.clipboard.unwrap());
            s.dragMode = Some(makeIdleDragMode());
        }
    } else if (auto a = action.is<FrameSelection>()) {
        if (not s.selection.empty()) {
            auto obb = s.selection.obb(s.tree);
            auto parentRef = s.tree.insert(Kind::FRAME, obb);
            s.tree.reparentRoots(s.selection.roots(), Some(parentRef));
            s.selection.unselectAll();
            s.selection.select(s.tree, parentRef);
        }
    } else if (auto a = action.is<DeleteSelection>()) {
        s.selection.remove(s.tree);
        s.dragMode = Some(makeIdleDragMode());
    } else if (auto c = action.is<ChooseFreeHandColor>()) {
        s.freehandColor = c->color;
    } else if (
        action.is<CanvasPress>() or
        action.is<CanvasDrag>() or
        action.is<CanvasRelease>()
    ) {
        if (not s.dragMode)
            s.dragMode = Some(makeIdleDragMode());
        auto nextMode = s.dragMode.unwrap()->reduce(s, action);
        if (nextMode)
            s.dragMode = nextMode;
    }

    return NONE;
};

export using Model = Ui::Model<State, Action, reduce>;

} // namespace Hideo::Canvas
