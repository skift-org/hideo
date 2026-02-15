module;

#include <karm/macros>

export module Hideo.Canvas:model;

import Karm.Core;
import Karm.Math;
import Karm.Ui;

using namespace Karm;

namespace Hideo::Canvas {

export struct Bound {
    Math::Vec2f center;
    Math::Vec2f size;
    f64 angle{0};

    Bound() = default;

    Bound(Math::Rectf rect)
        : center(rect.center()), size(rect.size()) {}

    Array<Math::Vec2f, 4> points() const {
        auto c = Math::Vec2f{Math::cos(angle), Math::sin(angle)};
        auto s = Math::Vec2f{-c.y, c.x};

        auto p0 = center - c * size.x / 2 - s * size.y / 2;
        auto p1 = center + c * size.x / 2 - s * size.y / 2;
        auto p2 = center + c * size.x / 2 + s * size.y / 2;
        auto p3 = center - c * size.x / 2 + s * size.y / 2;

        return {p0, p1, p2, p3};
    }

    Math::Rectf aabb() const {
        auto points = this->points();
        auto min = points[0];
        auto max = points[0];

        for (auto& p : points) {
            min = min.min(p);
            max = max.max(p);
        }

        return {min, max - min};
    }

    bool contains(Math::Vec2f point) const {
        auto c = Math::Vec2f{Math::cos(angle), Math::sin(angle)};
        auto s = Math::Vec2f{-c.y, c.x};

        auto local = point - center;
        auto x = local.dot(c);
        auto y = local.dot(s);

        return x >= -size.x / 2 and
               x < size.x / 2 and
               y >= -size.y / 2 and
               y < size.y / 2;
    }

    bool colide(Bound other) const {
        auto c0 = Math::Vec2f{Math::cos(angle), Math::sin(angle)};
        auto s0 = Math::Vec2f{-c0.y, c0.x};
        auto c1 = Math::Vec2f{Math::cos(other.angle), Math::sin(other.angle)};
        auto s1 = Math::Vec2f{-c1.y, c1.x};

        auto axes = Array<Math::Vec2f, 4>{c0, s0, c1, s1};
        auto p0 = points();
        auto p1 = other.points();

        auto project = [](Array<Math::Vec2f, 4> const& points, Math::Vec2f axis) {
            auto minProj = points[0].dot(axis);
            auto maxProj = minProj;

            for (usize i = 1; i < points.len(); i++) {
                auto proj = points[i].dot(axis);
                minProj = min(minProj, proj);
                maxProj = max(maxProj, proj);
            }

            return Pair<f64, f64>{minProj, maxProj};
        };

        for (auto axis : axes) {
            auto i0 = project(p0, axis);
            auto i1 = project(p1, axis);

            if (i0.v1 <= i1.v0 or i1.v1 <= i0.v0)
                return false;
        }

        return true;
    }

    void translate(Math::Vec2f delta) {
        center = center + delta;
    }

    void repr(Io::Emit& e) const {
        e("(bound {} {} {})", center, size, angle);
    }
};

export using Ref = u64;

export enum struct Kind {
    RECT,
    FRAME,
    TEXT,
    GROUP,
};

export struct Node {
    Ref ref;
    Opt<Ref> parent;
    Kind kind;
    Bound bound;

    bool topLevel() const {
        return not parent;
    }
};

export struct Tree {
    Vec<Node> _nodes;
    Ref _nextRef{1};

    Node const& byRef(Ref ref) const {
        for (auto const& n : _nodes) {
            if (n.ref == ref)
                return n;
        }

        panic("invalid node ref");
    }

    Node& byRef(Ref ref) {
        for (auto& n : _nodes) {
            if (n.ref == ref)
                return n;
        }

        panic("invalid node ref");
    }

    Opt<Ref> parentOf(Ref ref) const {
        return byRef(ref).parent;
    }

    Ref insert(Kind kind, Bound bound, Opt<Ref> parent = NONE) {
        auto ref = _nextRef++;
        _nodes.pushBack({
            .ref = ref,
            .parent = parent,
            .kind = kind,
            .bound = bound,
        });
        return ref;
    }

    Vec<Ref> descendantsOf(Ref ref) const {
        Vec<Ref> descendants;
        Vec<Ref> stack = {ref};

        while (stack) {
            auto curr = stack.popBack();
            descendants.pushBack(curr);

            for (auto const& n : _nodes) {
                if (n.parent == curr)
                    stack.pushBack(n.ref);
            }
        }

        return descendants;
    }

    Vec<Ref> descendantsOf(Slice<Ref const> refs) const {
        Vec<Ref> descendants;

        for (auto ref : refs) {
            for (auto child : descendantsOf(ref)) {
                if (not contains(descendants, child))
                    descendants.pushBack(child);
            }
        }

        return descendants;
    }

    void removeRefs(Slice<Ref const> refs) {
        auto descendants = descendantsOf(refs);

        for (usize i = _nodes.len(); i > 0; i--) {
            if (contains(descendants, _nodes[i - 1].ref)) {
                _nodes.removeAt(i - 1);
            }
        }
    }

    Opt<Ref> objectAt(Math::Vec2f pos) const {
        for (auto const& n : iterRev(_nodes)) {
            if (n.bound.contains(pos))
                return n.ref;
        }
        return NONE;
    }

    Vec<Ref> objectAt(Math::Rectf rect) const {
        Vec<Ref> res;
        auto selectionBound = Bound{rect};
        for (auto const& n : _nodes) {
            if (n.bound.colide(selectionBound))
                res.pushBack(n.ref);
        }
        return res;
    }
};

export struct Selection {
    Vec<Ref> _refs;
    Vec<Ref> _roots;
    Bound _bound;

    bool selected(Ref ref) const {
        return contains(_refs, ref);
    }

    void _updateRoots(Tree const& tree) {
        _roots.clear();

        for (auto ref : _refs) {
            bool foundSelectedAncestor = false;
            auto ancestor = tree.parentOf(ref);
            while (ancestor) {
                if (contains(_refs, ancestor.unwrap())) {
                    foundSelectedAncestor = true;
                    break;
                }
                ancestor = tree.parentOf(ancestor.unwrap());
            }

            if (not foundSelectedAncestor)
                _roots.pushBack(ref);
        }
    }

    void _updateBound(Tree const& tree) {
        if (not _refs) {
            _bound = Bound{};
            return;
        }

        auto merged = tree.byRef(_refs[0]).bound.aabb();
        for (auto const& ref : _refs) {
            auto const& n = tree.byRef(ref);
            merged = merged.mergeWith(n.bound.aabb());
        }

        _bound = Bound{merged};
    }

    void _update(Tree const& tree) {
        _updateRoots(tree);
        _updateBound(tree);
    }

    void select(Tree const& tree, Ref ref) {
        if (contains(_refs, ref))
            return;

        _refs.pushBack(ref);
        _update(tree);
    }

    void set(Tree const& tree, Vec<Ref> refs) {
        _refs = std::move(refs);
        _update(tree);
    }

    void unselect(Tree const& tree, Ref ref) {
        if (not contains(_refs, ref))
            return;

        _refs.removeAll(ref);
        _update(tree);
    }

    void unselectAll() {
        _refs.clear();
        _roots.clear();
        _bound = Bound{};
    }

    void remove(Tree& from) {
        from.removeRefs(_roots);
        unselectAll();
    }

    void moveBy(Tree& tree, Math::Vec2f delta) {
        auto moved = tree.descendantsOf(_roots);
        for (auto ref : moved) {
            auto& n = tree.byRef(ref);
            n.bound.translate(delta);
        }
        _update(tree);
    }

    void moveTo(Tree& tree, Math::Vec2f to) {
        auto delta = (to - _bound.center);
        moveBy(tree, delta);
    }

    void rotate(Tree& tree, f64 angle) {
        auto moved = tree.descendantsOf(_roots);
        for (auto ref : moved) {
            auto& n = tree.byRef(ref);
            n.bound.angle += angle;
        }
        _update(tree);
    }

    Tree copy(Tree& from) {
        Tree out;
        Vec<Ref> oldRefs;
        Vec<Ref> newRefs;

        for (auto root : _roots) {
            for (auto ref : from.descendantsOf(root)) {
                if (contains(oldRefs, ref))
                    continue;

                auto const& oldNode = from.byRef(ref);

                Opt<Ref> newParent = NONE;
                if (oldNode.parent) {
                    for (usize i = 0; i < oldRefs.len(); i++) {
                        if (oldRefs[i] == oldNode.parent.unwrap()) {
                            newParent = newRefs[i];
                            break;
                        }
                    }
                }

                auto newRef = out.insert(oldNode.kind, oldNode.bound, newParent);
                oldRefs.pushBack(oldNode.ref);
                newRefs.pushBack(newRef);
            }
        }

        return out;
    }

    Tree cut(Tree& from) {
        auto out = copy(from);
        remove(from);
        return out;
    }

    void paste(Tree& to, Tree what) {
        Vec<Ref> oldRefs;
        Vec<Ref> newRefs;

        for (auto const& oldNode : what._nodes) {
            Opt<Ref> newParent = NONE;
            if (oldNode.parent) {
                for (usize i = 0; i < oldRefs.len(); i++) {
                    if (oldRefs[i] == oldNode.parent.unwrap()) {
                        newParent = newRefs[i];
                        break;
                    }
                }
            }

            auto newRef = to.insert(oldNode.kind, oldNode.bound, newParent);
            oldRefs.pushBack(oldNode.ref);
            newRefs.pushBack(newRef);
        }

        _refs = std::move(newRefs);
        _update(to);
    }
};

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

    Opt<Math::Rectf> selectionRect() const;
};

struct DragMode {
    virtual ~DragMode() = default;

    virtual Opt<Rc<DragMode>> reduce(State& s, Action a) = 0;
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
            n.bound = Bound{Math::Rectf::fromTwoPoint(start, drag->pos)};
            s.selection._update(s.tree);
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
    Ref ref;
    Math::Vec2f start;
    Math::Vec2f end;

    ResizingDragMode(Ref ref, Math::Vec2f start, Math::Vec2f end)
        : ref(ref), start(start), end(end) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            auto& n = s.tree.byRef(ref);
            n.bound = Bound{Math::Rectf::fromTwoPoint(start, drag->pos)};
            s.selection._update(s.tree);
            return makeRc<ResizingDragMode>(ref, start, drag->pos);
        }

        if (a.is<CanvasRelease>()) {
            s.currentTool = Tool::SELECT;
            return makeIdleDragMode();
        }

        return makeRc<ResizingDragMode>(ref, start, end);
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
    Math::Vec2f last;

    MovingSelectionDragMode(Math::Vec2f last)
        : last(last) {}

    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto drag = a.is<CanvasDrag>()) {
            s.selection.moveBy(s.tree, drag->pos - last);
            return makeRc<MovingSelectionDragMode>(drag->pos);
        }

        if (a.is<CanvasRelease>()) {
            s.currentTool = Tool::SELECT;
            return makeIdleDragMode();
        }

        return makeRc<MovingSelectionDragMode>(last);
    }
};

struct IdleDragMode final : DragMode {
    Opt<Rc<DragMode>> reduce(State& s, Action a) override {
        if (auto press = a.is<CanvasPress>()) {
            if (s.currentTool == Tool::SELECT) {
                if (auto ref = s.objectAt(press->pos); ref) {
                    auto const pressedRef = ref.unwrap();
                    if (not s.selection.selected(pressedRef)) {
                        s.selection.set(s.tree, {pressedRef});
                    }

                    if (press->resize) {
                        return makeRc<ResizingDragMode>(pressedRef, press->pos, press->pos);
                    }

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
};

Rc<DragMode> makeIdleDragMode() {
    return makeRc<IdleDragMode>();
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
