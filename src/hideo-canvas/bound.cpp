export module Hideo.Canvas:bound;

import Karm.Core;
import Karm.Math;

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

} // namespace Hideo::Canvas
