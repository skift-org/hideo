export module Hideo.Canvas.Model:bound;

import Karm.Core;
import Karm.Math;

using namespace Karm;

namespace Hideo::Canvas {

export struct Obb {
    Math::Vec2f center;
    Math::Vec2f size;
    f64 angle{0};

    Obb() = default;

    Obb(Math::Rectf rect)
        : center(rect.center()), size(rect.size()) {}

    Obb(Math::Vec2f center, Math::Vec2f size, f64 angle)
        : center(center), size(size), angle(angle) {
    }

    Math::Vec2f half() const {
        return size / 2;
    }

    Math::Vec2f axisX() const {
        return {Math::cos(angle), Math::sin(angle)};
    }

    Math::Vec2f axisY() const {
        auto x = axisX();
        return {-x.y, x.x};
    }

    Obb toWorld(Obb const& local) const {
        return {
            toWorld(local.center),
            local.size,
            local.angle + angle
        };
    }

    Math::Vec2f toWorld(Math::Vec2f local) const {
        return center + axisX() * local.x + axisY() * local.y;
    }

    Math::Vec2f toLocal(Math::Vec2f world) const {
        auto rel = world - center;
        return {rel.dot(axisX()), rel.dot(axisY())};
    }

    Obb toLocal(Obb const& other) const {
        return {
            toLocal(other.center), // Transform the center point
            other.size,            // Size remains invariant under rotation/translation
            other.angle - angle    // The relative rotation
        };
    }

    Array<Math::Vec2f, 4> points() const {
        auto half = size / 2.0;
        return {
            toWorld({-half.x, -half.y}),
            toWorld({half.x, -half.y}),
            toWorld({half.x, half.y}),
            toWorld({-half.x, half.y})
        };
    }

    Math::Rectf aabb() const {
        auto pts = points();
        auto minP = pts[0];
        auto maxP = pts[0];

        for (auto& p : pts) {
            minP = minP.min(p);
            maxP = maxP.max(p);
        }
        return {minP, maxP - minP};
    }

    bool contains(Math::Vec2f point) const {
        auto local = toLocal(point).abs();
        auto half = size / 2.0;
        return local.x < half.x && local.y < half.y;
    }

    bool colide(Obb const& other) const {
        Array axes = {
            axisX(), axisY(),
            other.axisX(), other.axisY()
        };

        auto p0 = points();
        auto p1 = other.points();

        for (auto const& axis : axes) {
            auto range0 = project(p0, axis);
            auto range1 = project(p1, axis);

            if (range0.v1 <= range1.v0 || range1.v1 <= range0.v0)
                return false;
        }
        return true;
    }

    static Pair<f64, f64> project(Array<Math::Vec2f, 4> const& pts, Math::Vec2f axis) {
        f64 minP = pts[0].dot(axis);
        f64 maxP = minP;
        for (usize i = 1; i < 4; i++) {
            f64 p = pts[i].dot(axis);
            minP = min(minP, p);
            maxP = max(maxP, p);
        }
        return {minP, maxP};
    }

    Obb translated(Math::Vec2f delta) {
        Obb result = *this;
        result.center = center + delta;
        return result;
    }

    Obb scaled(Math::Vec2f pivot, Math::Vec2f scale) const {
        Obb result = *this;

        auto localPos = toLocal(center);
        result.center = toWorld(pivot + (localPos - pivot) * scale);

        result.size.x = max(size.x * Math::abs(scale.x), 2.0);
        result.size.y = max(size.y * Math::abs(scale.y), 2.0);

        return result;
    }

    Obb rotated(Math::Vec2f pivot, f64 delta) const {
        Obb result = *this;
        auto offset = center - pivot;
        result.center = pivot + offset.rotate(delta);
        result.angle += delta;
        return result;
    }

    Obb mergeWith(Obb const& other) const {
        auto minP = -half();
        auto maxP = half();

        for (auto p : other.points()) {
            auto local = toLocal(p);
            minP = minP.min(local);
            maxP = maxP.max(local);
        }

        auto localCenter = (minP + maxP) / 2.0;
        auto newSize = maxP - minP;

        return {
            toWorld(localCenter),
            newSize,
            angle
        };
    }

    void repr(Io::Emit& e) const {
        e("(bound {} {} {})", center, size, angle);
    }
};

} // namespace Hideo::Canvas
