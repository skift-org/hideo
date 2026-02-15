module;

#include <karm/macros>

export module Hideo.Canvas:model;

export import :bound;
export import :gizmo;
export import :node;
export import :selection;
export import :tree;

import Karm.Core;
import Karm.Math;
import Karm.Ui;

using namespace Karm;

namespace Hideo::Canvas {
export enum struct Tool {
    SELECT,
    FRAME,
    TEXT,
    RECT,
};

export struct SelectTool {
    Tool tool;
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
    bool resize = false;
};

export using Action = Union<SelectTool, CanvasPress, CanvasRelease, CanvasDrag>;

struct DragMode;

Kind _toolToKind(Tool tool);

export struct State {
    Tool currentTool = Tool::SELECT;
    Opt<Rc<DragMode>> dragMode;
    Tree tree;
    Selection selection;

    Opt<Ref> objectAt(Math::Vec2f pos) const {
        return tree.objectAt(pos);
    }

    Vec<Ref> objectAt(Math::Rectf rect) const {
        return tree.objectAt(rect);
    }

    Opt<SelectionGizmo> gizmo() const;

    Opt<Math::Rectf> selectionRect() const;
};

struct DragMode {
    virtual ~DragMode() = default;

    virtual Opt<Rc<DragMode>> reduce(State& s, Action a) = 0;

    virtual Opt<SelectionGizmo> gizmo(State const&) const {
        return NONE;
    }
};

Rc<DragMode> makeIdleDragMode();

struct PlacingDragMode final : DragMode {
    Ref ref;
    Math::Vec2f start;
    Math::Vec2f end;

    PlacingDragMode(Ref ref, Math::Vec2f start, Math::Vec2f end)
        : ref(ref), start(start), end(end) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            auto& n = s.tree.byRef(ref);
            if (drag->resize) {
                auto delta = drag->pos - start;
                auto side = max(Math::abs(delta.x), Math::abs(delta.y));

                f64 sx = delta.x < 0 ? -1.0 : 1.0;
                f64 sy = delta.y < 0 ? -1.0 : 1.0;

                if (Math::abs(delta.x) < 1e-6)
                    sx = sy;
                if (Math::abs(delta.y) < 1e-6)
                    sy = sx;

                auto constrained = start + Math::Vec2f{sx * side, sy * side};
                n.bound = Bound{Math::Rectf::fromTwoPoint(start, constrained)};
            } else {
                n.bound = Bound{Math::Rectf::fromTwoPoint(start, drag->pos)};
            }
            s.selection.refresh(s.tree);
            return makeRc<PlacingDragMode>(ref, start, drag->pos);
        }

        if (auto release = a.is<CanvasRelease>()) {
            if (start.dist(end) < 2) {
                auto& n = s.tree.byRef(ref);
                n.bound = Bound{Math::Rectf::fromTwoPoint(release->pos, release->pos).grow(50)};
            }

            s.currentTool = Tool::SELECT;
            return makeIdleDragMode();
        }

        return makeRc<PlacingDragMode>(ref, start, end);
    }
};

struct ResizingDragMode final : DragMode {
    SelectionGizmo _gizmo;
    GizmoHandle handle;
    Math::Vec2f pivot;

    ResizingDragMode(SelectionGizmo gizmo, GizmoHandle handle)
        : _gizmo(gizmo), handle(handle), pivot(gizmo.oppositePivot(handle)) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            auto scale = _gizmo.resizeScale(handle, drag->pos, drag->resize);
            auto sx = scale.v0;
            auto sy = scale.v1;

            s.selection.resize(s.tree, _gizmo, pivot, sx, sy);
            return makeRc<ResizingDragMode>(_gizmo, handle);
        }

        if (a.is<CanvasRelease>()) {
            s.selection.commitTransform(s.tree);
            s.currentTool = Tool::SELECT;
            return makeIdleDragMode();
        }

        return makeRc<ResizingDragMode>(_gizmo, handle);
    }

    Opt<SelectionGizmo> gizmo(State const&) const override {
        return _gizmo;
    }
};

struct RotatingSelectionDragMode final : DragMode {
    Math::Vec2f center;
    f64 startAngle;

    RotatingSelectionDragMode(Math::Vec2f center, f64 startAngle)
        : center(center), startAngle(startAngle) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            auto currAngle = Math::atan2(drag->pos.y - center.y, drag->pos.x - center.x);
            auto delta = currAngle - startAngle;

            s.selection.rotate(s.tree, center, delta);
            return makeRc<RotatingSelectionDragMode>(center, startAngle);
        }

        if (a.is<CanvasRelease>()) {
            s.selection.commitTransform(s.tree);
            s.currentTool = Tool::SELECT;
            return makeIdleDragMode();
        }

        return makeRc<RotatingSelectionDragMode>(center, startAngle);
    }

    Opt<SelectionGizmo> gizmo(State const& s) const override {
        return s.selection.createGizmo(s.tree);
    }
};

struct SelectingDragMode final : DragMode {
    Math::Vec2f start;
    Math::Vec2f end;

    SelectingDragMode(Math::Vec2f start, Math::Vec2f end)
        : start(start), end(end) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            s.selection.set(s.tree, s.objectAt(Math::Rectf::fromTwoPoint(start, drag->pos)));
            return makeRc<SelectingDragMode>(start, drag->pos);
        }

        if (a.is<CanvasRelease>()) {
            s.currentTool = Tool::SELECT;
            return makeIdleDragMode();
        }

        return makeRc<SelectingDragMode>(start, end);
    }
};

struct MovingSelectionDragMode final : DragMode {
    Math::Vec2f start;

    MovingSelectionDragMode(Math::Vec2f start)
        : start(start) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            s.selection.move(s.tree, drag->pos - start);
            return makeRc<MovingSelectionDragMode>(start);
        }

        if (a.is<CanvasRelease>()) {
            s.selection.commitTransform(s.tree);
            s.currentTool = Tool::SELECT;
            return makeIdleDragMode();
        }

        return makeRc<MovingSelectionDragMode>(start);
    }
};

struct IdleDragMode final : DragMode {
    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto press = a.is<CanvasPress>()) {
            if (s.currentTool == Tool::SELECT) {
                if (auto gizmo = s.selection.createGizmo(s.tree); gizmo) {
                    auto hitHandle = gizmo->hitHandle(press->pos);

                    if (hitHandle == GizmoHandle::ROTATE) {
                        s.selection.beginTransform(s.tree);

                        auto startAngle = Math::atan2(press->pos.y - gizmo->bound.center.y, press->pos.x - gizmo->bound.center.x);
                        return makeRc<RotatingSelectionDragMode>(gizmo->bound.center, startAngle);
                    }

                    if (gizmo->isResizeHandle(hitHandle)) {
                        s.selection.beginTransform(s.tree);
                        return makeRc<ResizingDragMode>(*gizmo, hitHandle);
                    }
                }

                if (auto ref = s.objectAt(press->pos); ref) {
                    auto const pressedRef = ref.unwrap();
                    if (not s.selection.selected(pressedRef)) {
                        s.selection.set(s.tree, {pressedRef});
                    }

                    if (press->resize) {
                        if (auto gizmo = s.selection.createGizmo(s.tree); gizmo) {
                            s.selection.beginTransform(s.tree);
                            return makeRc<ResizingDragMode>(*gizmo, GizmoHandle::SE);
                        }
                    }

                    s.selection.beginTransform(s.tree);
                    return makeRc<MovingSelectionDragMode>(press->pos);
                }

                s.selection.unselectAll();
                return makeRc<SelectingDragMode>(press->pos, press->pos);
            }

            auto ref = s.tree.insert(
                _toolToKind(s.currentTool),
                Bound{Math::Rectf::fromTwoPoint(press->pos, press->pos)}
            );

            s.selection.set(s.tree, {ref});
            return makeRc<PlacingDragMode>(ref, press->pos, press->pos);
        }

        return makeIdleDragMode();
    }

    Opt<SelectionGizmo> gizmo(State const& s) const override {
        return s.selection.createGizmo(s.tree);
    }
};

Rc<DragMode> makeIdleDragMode() {
    return makeRc<IdleDragMode>();
}

Opt<SelectionGizmo> State::gizmo() const {
    if (not dragMode)
        return NONE;

    return dragMode.unwrap()->gizmo(*this);
}

Opt<Math::Rectf> State::selectionRect() const {
    if (not dragMode)
        return NONE;

    if (auto selecting = dragMode.unwrap().is<SelectingDragMode>()) {
        return Math::Rectf::fromTwoPoint(selecting->start, selecting->end);
    }

    return NONE;
}

Kind _toolToKind(Tool tool) {
    switch (tool) {
    case Tool::FRAME:
        return Kind::FRAME;
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

void _reduceDragAction(State& s, Action action) {
    if (not s.dragMode)
        s.dragMode = makeIdleDragMode();

    s.dragMode = s.dragMode.unwrap()->reduce(s, action);
}

Ui::Task<Action> reduce(State& s, Action a) {
    if (auto action = a.is<SelectTool>()) {
        s.currentTool = action->tool;
    } else if (a.is<CanvasPress>() or a.is<CanvasDrag>() or a.is<CanvasRelease>()) {
        _reduceDragAction(s, a);
    }

    return NONE;
};

export using Model = Ui::Model<State, Action, reduce>;

} // namespace Hideo::Canvas
