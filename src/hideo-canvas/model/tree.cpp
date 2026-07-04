export module Hideo.Canvas.Model:tree;

import Karm.Core;
import Karm.Math;
import :bound;
import :node;

using namespace Karm;

namespace Hideo::Canvas {

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

    Ref insert(Kind kind, Obb bound, Opt<Ref> parent = NONE) {
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

    bool hasMixedAnglesDescendants(Ref ref) const {
        f64 angle = byRef(ref).bound.angle;
        for (auto desc : descendantsOf(ref))
            if (not Math::epsilonEq(byRef(desc).bound.angle, angle))
                return true;
        return false;
    }

    Vec<Ref> descendantsOf(Slice<Ref> refs) const {
        Vec<Ref> descendants;

        for (auto ref : refs) {
            for (auto child : descendantsOf(ref)) {
                if (not contains(descendants, child))
                    descendants.pushBack(child);
            }
        }

        return descendants;
    }

    void removeRefs(Slice<Ref> refs) {
        auto descendants = descendantsOf(refs);

        for (usize i = _nodes.len(); i > 0; i--) {
            if (contains(descendants, _nodes[i - 1].ref)) {
                _nodes.removeAt(i - 1);
            }
        }
    }

    void reparentRoots(Slice<Ref> refs, Opt<Ref> newParent) {
        if (not refs)
            return;

        if (newParent)
            byRef(newParent.unwrap());

        for (auto ref : refs) {
            if (newParent) {
                auto parentRef = newParent.unwrap();
                if (parentRef == ref)
                    continue;

                if (contains(descendantsOf(ref), parentRef))
                    continue;
            }

            byRef(ref).parent = newParent;
        }
    }

    // MARK: Hit Testing -------------------------------------------------------

    Opt<Ref> _objectAtPoint(Ref ref, Math::Vec2f pos) const {
        auto const& node = byRef(ref);
        if (not node.bound.contains(pos))
            return NONE;

        for (auto const& child : iterRev(_nodes)) {
            if (child.parent == ref) {
                if (auto hit = _objectAtPoint(child.ref, pos); hit)
                    return hit;
            }
        }

        if (node.kind == Kind::FRAME)
            return NONE;

        return ref;
    }

    void _objectAtRect(Ref ref, Math::Rectf rect, Vec<Ref>& out, Opt<Ref> startFrame = NONE, bool allowFrameSelection = false) const {
        auto const& node = byRef(ref);
        auto selectionBound = Obb{rect};

        if (not node.bound.colide(selectionBound))
            return;

        if (node.kind != Kind::FRAME) {
            out.pushBack(ref);
        } else if (allowFrameSelection) {
            bool selectable = false;

            if (not node.parent and not startFrame) {
                selectable = true;
            } else if (node.parent and startFrame and node.parent.unwrap() == startFrame.unwrap()) {
                selectable = true;
            }

            if (selectable)
                out.pushBack(ref);
        }

        for (auto const& child : _nodes) {
            if (child.parent == ref)
                _objectAtRect(child.ref, rect, out, startFrame, allowFrameSelection);
        }
    }

    Opt<Ref> objectAt(Math::Vec2f pos) const {
        for (auto const& n : iterRev(_nodes)) {
            if (n.topLevel()) {
                if (auto hit = _objectAtPoint(n.ref, pos); hit)
                    return hit;
            }
        }

        return NONE;
    }

    Opt<Ref> frameAt(Math::Vec2f pos, Slice<Ref> ignored = {}) const {
        for (auto const& n : iterRev(_nodes)) {
            if (contains(ignored, n.ref))
                continue;

            if (n.kind == Kind::FRAME and n.bound.contains(pos))
                return n.ref;
        }

        return NONE;
    }

    Vec<Ref> objectAt(Math::Rectf rect) const {
        Vec<Ref> res;

        for (auto const& n : _nodes) {
            if (n.topLevel())
                _objectAtRect(n.ref, rect, res);
        }

        return res;
    }

    Vec<Ref> objectAt(Math::Rectf rect, Math::Vec2f startPos) const {
        Vec<Ref> res;
        auto startFrame = frameAt(startPos);

        for (auto const& n : _nodes) {
            if (n.topLevel())
                _objectAtRect(n.ref, rect, res, startFrame, true);
        }

        return res;
    }
};

} // namespace Hideo::Canvas
