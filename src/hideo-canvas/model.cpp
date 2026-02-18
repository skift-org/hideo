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
import Karm.App;

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

export struct SelectAll {};

export struct CopySelection {};

export struct CutSelection {};

export struct PasteSelection {};

export struct DeleteSelection {};

export struct FrameSelection {};

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

export using Action = Union<
    SelectTool,
    SelectAll,
    CopySelection,
    CutSelection,
    PasteSelection,
    FrameSelection,
    DeleteSelection,
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
    Ref ref;
    Math::Vec2f start;
    Math::Vec2f end;

    PlacingDragMode(Ref ref, Math::Vec2f start, Math::Vec2f end)
        : ref(ref), start(start), end(end) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            auto& n = s.tree.byRef(ref);
            if (App::match(drag->mods, App::KeyMod::SHIFT)) {
                auto delta = drag->pos - start;
                n.bound = Obb{Math::Rectf::fromTwoPoint(start, start + delta.snapToDiagonal())};
            } else {
                n.bound = Obb{Math::Rectf::fromTwoPoint(start, drag->pos)};
            }
            return makeRc<PlacingDragMode>(ref, start, drag->pos);
        }

        if (auto release = a.is<CanvasRelease>()) {
            if (start.dist(end) < 2) {
                auto& n = s.tree.byRef(ref);
                n.bound = Obb{Math::Rectf::fromTwoPoint(release->pos, release->pos).grow(50)};
            }

            s.currentTool = Tool::SELECT;
            return makeIdleDragMode();
        }

        return makeRc<PlacingDragMode>(ref, start, end);
    }
};

struct ResizingDragMode final : DragMode {
    Gizmo _gizmo;
    GizmoHandle handle;
    Math::Vec2f pivot;

    ResizingDragMode(Gizmo gizmo, GizmoHandle handle)
        : _gizmo(gizmo), handle(handle), pivot(gizmo.oppositePivot(handle)) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            auto scale = _gizmo.resizeScale(handle, drag->pos, App::match(drag->mods, App::KeyMod::SHIFT));
            s.selection.resize(s.tree, _gizmo.bound, pivot, scale);
            return makeRc<ResizingDragMode>(_gizmo, handle);
        }

        if (a.is<CanvasRelease>()) {
            s.selection.commitTransform(s.tree);
            s.currentTool = Tool::SELECT;
            return makeIdleDragMode();
        }

        return makeRc<ResizingDragMode>(_gizmo, handle);
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
            return makeIdleDragMode();
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
            return makeIdleDragMode();
        }

        return NONE;
    }

    Opt<Math::Rectf> selectionRect() const override {
        return Math::Rectf::fromTwoPoint(start, end);
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
            return makeIdleDragMode();
        }

        return makeRc<MovingSelectionDragMode>(start, parent);
    }
};

struct IdleDragMode final : DragMode {
    static Kind _toolToKind(Tool tool) {
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

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto press = a.is<CanvasPress>()) {
            if (s.currentTool != Tool::SELECT) {
                auto parent = s.tree.frameAt(press->pos);
                auto ref = s.tree.insert(
                    _toolToKind(s.currentTool),
                    Obb{Math::Rectf::fromTwoPoint(press->pos, press->pos)},
                    parent
                );

                s.selection.set(s.tree, {ref});
                return makeRc<PlacingDragMode>(ref, press->pos, press->pos);
            }

            if (auto gizmo = s.selection.createGizmo(s.tree); gizmo) {
                auto [hitHandle, rotate] = gizmo->hitHandle(press->pos);

                if (rotate) {
                    s.selection.beginTransform(s.tree);
                    auto startAngle = Math::atan2(press->pos.y - gizmo->bound.center.y, press->pos.x - gizmo->bound.center.x);
                    return makeRc<RotatingSelectionDragMode>(gizmo->bound.center, startAngle);
                }

                if (hitHandle != GizmoHandle::NONE) {
                    s.selection.beginTransform(s.tree);
                    return makeRc<ResizingDragMode>(*gizmo, hitHandle);
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
                        return makeRc<ResizingDragMode>(*gizmo, GizmoHandle::SE);
                    }
                }

                s.selection.beginTransform(s.tree);
                return makeRc<MovingSelectionDragMode>(press->pos);
            }

            if (auto frame = s.tree.frameAt(press->pos); frame and s.selection.selected(frame.unwrap())) {
                s.selection.beginTransform(s.tree);
                return makeRc<MovingSelectionDragMode>(press->pos);
            }

            s.selection.unselectAll();
            return makeRc<SelectingDragMode>(press->pos);
        }

        return makeIdleDragMode();
    }

    Opt<Gizmo> gizmo(State const& s) const override {
        return s.selection.createGizmo(s.tree);
    }
};

Rc<DragMode> makeIdleDragMode() {
    return makeRc<IdleDragMode>();
}

Ui::Task<Action> reduce(State& s, Action action) {
    if (auto a = action.is<SelectTool>()) {
        s.currentTool = a->tool;
    } else if (action.is<SelectAll>()) {
        s.selection.selectAll(s.tree);
    } else if (auto a = action.is<CopySelection>()) {
        if (not s.selection.empty())
            s.clipboard = s.selection.copy(s.tree);
    } else if (auto a = action.is<CopySelection>()) {
        if (not s.selection.empty()) {
            s.clipboard = s.selection.cut(s.tree);
            s.dragMode = makeIdleDragMode();
        }
    } else if (auto a = action.is<PasteSelection>()) {
        if (s.clipboard) {
            s.selection.paste(s.tree, s.clipboard.unwrap());
            s.dragMode = makeIdleDragMode();
        }
    } else if (auto a = action.is<FrameSelection>()) {
        if (not s.selection.empty()) {
            auto obb = s.selection.obb(s.tree);
            auto parentRef = s.tree.insert(Kind::FRAME, obb);
            s.tree.reparentRoots(s.selection.roots(), parentRef);
            s.selection.unselectAll();
            s.selection.select(s.tree, parentRef);
        }
    } else if (auto a = action.is<DeleteSelection>()) {
        s.selection.remove(s.tree);
        s.dragMode = makeIdleDragMode();
    } else if (
        action.is<CanvasPress>() or
        action.is<CanvasDrag>() or
        action.is<CanvasRelease>()
    ) {
        if (not s.dragMode)
            s.dragMode = makeIdleDragMode();
        auto nextMode = s.dragMode.unwrap()->reduce(s, action);
        if (nextMode)
            s.dragMode = nextMode;
    }

    return NONE;
};

export using Model = Ui::Model<State, Action, reduce>;

} // namespace Hideo::Canvas
