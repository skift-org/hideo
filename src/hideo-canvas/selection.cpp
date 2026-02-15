export module Hideo.Canvas:selection;

import Karm.Core;
import Karm.Gfx;
import Karm.Math;
import Karm.Ui;
import :bound;
import :gizmo;
import :node;
import :tree;

using namespace Karm;

namespace Hideo::Canvas {

export struct Selection {
    struct Transform {
        Vec<Ref> refs;
        Vec<Bound> initial;
    };

    Vec<Ref> _refs;
    Vec<Ref> _roots;
    Bound _bound;
    Opt<Transform> _transform;

    bool empty() const {
        return not _refs;
    }

    Slice<Ref const> roots() const {
        return _roots;
    }

    Bound const& bound() const {
        return _bound;
    }

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

    void refresh(Tree const& tree) {
        _update(tree);
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

    void beginTransform(Tree const& tree) {
        auto refs = tree.descendantsOf(_roots);
        Vec<Bound> initial;
        for (auto ref : refs)
            initial.pushBack(tree.byRef(ref).bound);

        _transform = Transform{
            .refs = std::move(refs),
            .initial = std::move(initial),
        };
    }

    void move(Tree& tree, Math::Vec2f delta);

    void resize(Tree& tree, SelectionGizmo const& gizmo, Math::Vec2f pivot, f64 sx, f64 sy);

    void rotate(Tree& tree, Math::Vec2f center, f64 delta);

    Opt<SelectionGizmo> createGizmo(Tree const& from) const;

    void commitTransform(Tree const& tree) {
        _transform = NONE;
        _update(tree);
    }

    void discardTransform(Tree& tree);
};

void Selection::move(Tree& tree, Math::Vec2f delta) {
    if (not _transform)
        return;

    auto const& transform = _transform.unwrap();
    for (usize i = 0; i < transform.refs.len(); i++) {
        auto& n = tree.byRef(transform.refs[i]);
        n.bound.center = transform.initial[i].center + delta;
    }

    _update(tree);
}

void Selection::resize(Tree& tree, SelectionGizmo const& gizmo, Math::Vec2f pivot, f64 sx, f64 sy) {
    if (not _transform)
        return;

    auto const& transform = _transform.unwrap();
    for (usize i = 0; i < transform.refs.len(); i++) {
        auto localCenter = gizmo.toLocal(transform.initial[i].center);
        auto newLocalCenter = Math::Vec2f{
            pivot.x + (localCenter.x - pivot.x) * sx,
            pivot.y + (localCenter.y - pivot.y) * sy,
        };

        auto& n = tree.byRef(transform.refs[i]);
        n.bound.center = gizmo.toWorld(newLocalCenter);
        n.bound.size = {
            max(transform.initial[i].size.x * Math::abs(sx), 2.0),
            max(transform.initial[i].size.y * Math::abs(sy), 2.0),
        };
    }

    _update(tree);
}

void Selection::rotate(Tree& tree, Math::Vec2f center, f64 delta) {
    if (not _transform)
        return;

    auto const& transform = _transform.unwrap();
    for (usize i = 0; i < transform.refs.len(); i++) {
        auto& n = tree.byRef(transform.refs[i]);
        n.bound.center = center + (transform.initial[i].center - center).rotate(delta);
        n.bound.angle = transform.initial[i].angle + delta;
    }

    _update(tree);
}

void Selection::discardTransform(Tree& tree) {
    if (not _transform)
        return;

    auto const& transform = _transform.unwrap();
    for (usize i = 0; i < transform.refs.len(); i++) {
        auto& n = tree.byRef(transform.refs[i]);
        n.bound = transform.initial[i];
    }

    _transform = NONE;
    _update(tree);
}

Opt<SelectionGizmo> Selection::createGizmo(Tree const& from) const {
    if (empty())
        return NONE;

    auto refs = from.descendantsOf(roots());
    if (not refs)
        return NONE;

    auto angle = from.byRef(refs[0]).bound.angle;
    bool mixedAngles = false;
    for (auto ref : refs) {
        if (Math::abs(from.byRef(ref).bound.angle - angle) > 1e-4) {
            mixedAngles = true;
            break;
        }
    }

    if (mixedAngles) {
        return SelectionGizmo{
            .bound = bound(),
            .uniformOnly = true,
        };
    }

    auto c = Math::Vec2f{Math::cos(angle), Math::sin(angle)};
    auto s = Math::Vec2f{-c.y, c.x};

    f64 minX = Limits<f64>::MAX;
    f64 maxX = -Limits<f64>::MAX;
    f64 minY = Limits<f64>::MAX;
    f64 maxY = -Limits<f64>::MAX;

    for (auto ref : refs) {
        for (auto p : from.byRef(ref).bound.points()) {
            auto x = p.dot(c);
            auto y = p.dot(s);
            minX = min(minX, x);
            maxX = max(maxX, x);
            minY = min(minY, y);
            maxY = max(maxY, y);
        }
    }

    auto cx = (minX + maxX) / 2;
    auto cy = (minY + maxY) / 2;
    auto center = c * cx + s * cy;

    Bound gizBound;
    gizBound.center = center;
    gizBound.size = {maxX - minX, maxY - minY};
    gizBound.angle = angle;

    return SelectionGizmo{
        .bound = gizBound,
        .uniformOnly = false,
    };
}

} // namespace Hideo::Canvas
