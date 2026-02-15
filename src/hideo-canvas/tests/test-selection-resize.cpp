#include <karm/test>

import Hideo.Canvas;
import Karm.Math;

using namespace Karm;

namespace Hideo::Canvas::Tests {

test$("canvas-selection-resize-multi-node") {
    Tree tree;

    auto refA = tree.insert(Kind::RECT, Obb{{0.0, 0.0}, {20.0, 10.0}, 0.0});
    auto refB = tree.insert(Kind::RECT, Obb{{100.0, 0.0}, {20.0, 10.0}, 0.0});

    Selection selection;
    selection.set(tree, {refA, refB});

    selection.beginTransform(tree);

    auto gizmo = selection.createGizmo(tree);
    expect$(gizmo != NONE);

    auto const giz = gizmo.unwrap();
    auto pivot = giz.oppositePivot(GizmoHandle::SE);
    selection.resize(tree, giz.bound, pivot, {2.0, 2.0});
    selection.commitTransform(tree);

    auto const& a = tree.byRef(refA).bound;
    auto const& b = tree.byRef(refB).bound;

    expect$(Math::abs(a.center.x - 10.0) < 1e-6);
    expect$(Math::abs(a.center.y - 5.0) < 1e-6);
    expect$(Math::abs(b.center.x - 210.0) < 1e-6);
    expect$(Math::abs(b.center.y - 5.0) < 1e-6);

    expect$(Math::abs(a.size.x - 40.0) < 1e-6);
    expect$(Math::abs(a.size.y - 20.0) < 1e-6);
    expect$(Math::abs(b.size.x - 40.0) < 1e-6);
    expect$(Math::abs(b.size.y - 20.0) < 1e-6);

    return Ok();
}

} // namespace Hideo::Canvas::Tests
