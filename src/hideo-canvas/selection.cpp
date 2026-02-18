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
    struct TransformContext {
        Vec<Ref> refs;
        Vec<Obb> initial;
    };

    Vec<Ref> _refs;
    Vec<Ref> _roots;
    Opt<TransformContext> _transform;

    bool empty() const {
        return not _refs;
    }

    Math::Rectf aabb(Tree const& from) const {
        auto res = from.byRef(_roots[0]).bound.aabb();
        for (usize i = 1; i < _roots.len(); ++i)
            res = res.mergeWith(from.byRef(_roots[i]).bound.aabb());
        return res;
    }

    Obb obb(Tree const& from) const {
        auto res = from.byRef(_roots[0]).bound;
        for (usize i = 1; i < _roots.len(); ++i)
            res = res.mergeWith(from.byRef(_roots[i]).bound);
        return res;
    }

    Slice<Ref> roots() const {
        return _roots;
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

    void select(Tree const& tree, Ref ref) {
        if (contains(_refs, ref))
            return;

        _refs.pushBack(ref);
        _updateRoots(tree);
    }

    void selectAll(Tree const& tree) {
        unselectAll();
        _refs =
            iter(tree._nodes)
                .map([](auto& n) {
                    return n.ref;
                })
                .collect<Vec<Ref>>();
        _updateRoots(tree);
    }

    void set(Tree const& tree, Vec<Ref> refs) {
        _refs = std::move(refs);
        _updateRoots(tree);
    }

    void unselect(Tree const& tree, Ref ref) {
        if (not contains(_refs, ref))
            return;

        _refs.removeAll(ref);
        _updateRoots(tree);
    }

    void unselectAll() {
        _refs.clear();
        _roots.clear();
    }

    void remove(Tree& from) {
        from.removeRefs(_roots);
        unselectAll();
    }

    void moveBy(Tree& tree, Math::Vec2f delta) {
        auto moved = tree.descendantsOf(_roots);
        for (auto ref : moved) {
            auto& n = tree.byRef(ref);
            n.bound = n.bound.translated(delta);
        }
        _updateRoots(tree);
    }

    // void moveTo(Tree& tree, Math::Vec2f to) {
    //     auto delta = (to - _bound.center);
    //     moveBy(tree, delta);
    // }

    void rotate(Tree& tree, f64 angle) {
        auto moved = tree.descendantsOf(_roots);
        for (auto ref : moved) {
            auto& n = tree.byRef(ref);
            n.bound.angle += angle;
        }
        _updateRoots(tree);
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
        _updateRoots(to);
    }

    void beginTransform(Tree const& tree) {
        auto refs = tree.descendantsOf(_roots);
        Vec<Obb> initial;
        for (auto ref : refs)
            initial.pushBack(tree.byRef(ref).bound);

        _transform = TransformContext{
            .refs = std::move(refs),
            .initial = std::move(initial),
        };
    }

    void move(Tree& tree, Math::Vec2f delta) {
        if (not _transform)
            return;

        auto const& transform = _transform.unwrap();
        for (usize i = 0; i < transform.refs.len(); i++) {
            auto& n = tree.byRef(transform.refs[i]);
            n.bound.center = transform.initial[i].center + delta;
        }
        _updateRoots(tree);
    }

    void resize(Tree& tree, Obb const& source, Math::Vec2f pivot, Math::Vec2f scale) {
        if (not _transform)
            return;

        auto const& transform = _transform.unwrap();

        for (usize i = 0; i < transform.refs.len(); i++) {
            auto localObb = source.toLocal(transform.initial[i]);
            auto localPivot = localObb.toLocal(pivot);
            auto scaledObb = localObb.scaled(localPivot, scale);
            auto& n = tree.byRef(transform.refs[i]);
            n.bound = source.toWorld(scaledObb);
        }
        _updateRoots(tree);
    }

    void rotate(Tree& tree, Math::Vec2f center, f64 delta) {
        if (not _transform)
            return;

        auto const& transform = _transform.unwrap();
        for (usize i = 0; i < transform.refs.len(); i++) {
            auto& n = tree.byRef(transform.refs[i]);
            n.bound.center = center + (transform.initial[i].center - center).rotate(delta);
            n.bound.angle = transform.initial[i].angle + delta;
        }
        _updateRoots(tree);
    }

    bool hasMixedAngles(Tree const& tree) const {
        f64 refAngle = tree.byRef(_roots[0]).bound.angle;
        for (auto ref : _roots) {
            auto const& node = tree.byRef(ref);
            if (not Math::epsilonEq(node.bound.angle, refAngle))
                return true;

            if (node.kind == Kind::FRAME)
                if (tree.hasMixedAnglesDescendants(ref))
                    return true;
        }

        return false;
    }

    Opt<Gizmo> createGizmo(Tree const& from) const {
        if (empty())
            return NONE;

        if (not _roots)
            return NONE;

        bool mixedAngles = hasMixedAngles(from);
        return Gizmo{
            .bound = mixedAngles ? aabb(from) : obb(from),
            .uniformOnly = mixedAngles,
        };
    }

    void commitTransform(Tree const& tree) {
        _transform = NONE;
        _updateRoots(tree);
    }

    void discardTransform(Tree& tree) {
        if (not _transform)
            return;

        auto const& transform = _transform.unwrap();
        for (usize i = 0; i < transform.refs.len(); i++) {
            auto& n = tree.byRef(transform.refs[i]);
            n.bound = transform.initial[i];
        }

        _transform = NONE;
        _updateRoots(tree);
    }
};

} // namespace Hideo::Canvas
