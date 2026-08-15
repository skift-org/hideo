#include <karm/test>

import Hideo.Canvas.Model;
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

test$("canvas-tree-reparent-refs") {
    Tree tree;

    auto a = tree.insert(Kind::FRAME, Obb{{0.0, 0.0}, {10.0, 10.0}, 0.0});
    auto b = tree.insert(Kind::RECT, Obb{{20.0, 0.0}, {10.0, 10.0}, 0.0});
    auto c = tree.insert(Kind::TEXT, Obb{{40.0, 0.0}, {10.0, 10.0}, 0.0}, Some(a));
    auto d = tree.insert(Kind::GROUP, Obb{{60.0, 0.0}, {10.0, 10.0}, 0.0});

    tree.reparentRoots(Vec<Ref>{a}, Some(d));

    expectEq$(tree.parentOf(a), d);
    expectEq$(tree.parentOf(c), a);
    expectEq$(tree.parentOf(b), NONE);

    tree.reparentRoots(Vec<Ref>{a}, Some(c));
    expectEq$(tree.parentOf(a), d);

    return Ok();
}

test$("canvas-hit-test-frame-content") {
    Tree tree;

    auto frame = tree.insert(Kind::FRAME, Obb{{100.0, 100.0}, {120.0, 120.0}, 0.0});
    auto child = tree.insert(Kind::RECT, Obb{{100.0, 100.0}, {40.0, 40.0}, 0.0}, Some(frame));
    auto nestedFrame = tree.insert(Kind::FRAME, Obb{{120.0, 120.0}, {30.0, 30.0}, 0.0}, Some(frame));

    auto hitChild = tree.objectAt(Math::Vec2f{100.0, 100.0});
    expectEq$(hitChild, child);

    auto hitFrameInterior = tree.objectAt(Math::Vec2f{70.0, 70.0});
    expectEq$(hitFrameInterior, NONE);

    auto lasso = tree.objectAt(Math::Rectf::fromTwoPoint({60.0, 60.0}, {140.0, 140.0}));
    expect$(contains(lasso, child));
    expectNot$(contains(lasso, frame));

    auto lassoFromRoot = tree.objectAt(
        Math::Rectf::fromTwoPoint({20.0, 20.0}, {160.0, 160.0}),
        Math::Vec2f{20.0, 20.0}
    );
    expect$(contains(lassoFromRoot, frame));

    auto lassoFromParent = tree.objectAt(
        Math::Rectf::fromTwoPoint({80.0, 80.0}, {140.0, 140.0}),
        Math::Vec2f{80.0, 80.0}
    );
    expect$(contains(lassoFromParent, nestedFrame));

    return Ok();
}

test$("canvas-frame-gizmo-uniform-only-on-mixed-child-angles") {
    Tree tree;

    auto frame = tree.insert(Kind::FRAME, Obb{{100.0, 100.0}, {120.0, 120.0}, 0.0});
    tree.insert(Kind::RECT, Obb{{90.0, 100.0}, {30.0, 20.0}, 0.1}, Some(frame));
    tree.insert(Kind::RECT, Obb{{110.0, 100.0}, {30.0, 20.0}, -0.2}, Some(frame));

    Selection selection;
    selection.set(tree, {frame});

    auto gizmo = selection.createGizmo(tree);
    expect$(gizmo != NONE);
    expect$(gizmo.unwrap().uniformOnly);

    return Ok();
}

test$("canvas-frame-gizmo-uniform-only-on-child-frame-angle-mismatch") {
    Tree tree;

    auto frame = tree.insert(Kind::FRAME, Obb{{100.0, 100.0}, {120.0, 120.0}, 0.0});
    tree.insert(Kind::RECT, Obb{{100.0, 100.0}, {30.0, 20.0}, 0.25}, Some(frame));

    Selection selection;
    selection.set(tree, {frame});

    auto gizmo = selection.createGizmo(tree);
    expect$(gizmo != NONE);
    expect$(gizmo.unwrap().uniformOnly);

    return Ok();
}

test$("canvas-uniform-gizmo-aabb-uses-roots-not-frame-children") {
    Tree tree;

    auto frame = tree.insert(Kind::FRAME, Obb{{100.0, 100.0}, {80.0, 80.0}, 0.0});
    auto child = tree.insert(Kind::RECT, Obb{{220.0, 100.0}, {20.0, 20.0}, 0.25}, Some(frame));

    Selection selection;
    selection.set(tree, {frame, child});

    auto gizmo = selection.createGizmo(tree);
    expect$(gizmo != NONE);
    expect$(gizmo.unwrap().uniformOnly);

    auto expected = tree.byRef(frame).bound.aabb();
    expect$(Math::abs(gizmo.unwrap().bound.center.x - expected.center().x) < 1e-6);
    expect$(Math::abs(gizmo.unwrap().bound.center.y - expected.center().y) < 1e-6);
    expect$(Math::abs(gizmo.unwrap().bound.size.x - expected.width) < 1e-6);
    expect$(Math::abs(gizmo.unwrap().bound.size.y - expected.height) < 1e-6);

    return Ok();
}

} // namespace Hideo::Canvas::Tests
