export module Hideo.Canvas:tree;

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

} // namespace Hideo::Canvas
