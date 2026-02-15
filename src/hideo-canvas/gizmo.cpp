export module Hideo.Canvas:gizmo;

import Karm.Core;
import Karm.Gfx;
import Karm.Math;
import Karm.Ui;
import :bound;

using namespace Karm;

namespace Hideo::Canvas {

export enum struct GizmoHandle {
    NONE,
    N,
    NE,
    E,
    SE,
    S,
    SW,
    W,
    NW,
    ROTATE,
};

export struct SelectionGizmo {
    Bound bound;
    bool uniformOnly = false;

    Math::Vec2f axisX() const {
        return Math::Vec2f{Math::cos(bound.angle), Math::sin(bound.angle)};
    }

    Math::Vec2f axisY() const {
        auto c = axisX();
        return Math::Vec2f{-c.y, c.x};
    }

    Math::Vec2f toWorld(Math::Vec2f local) const {
        return bound.center + axisX() * local.x + axisY() * local.y;
    }

    Math::Vec2f toLocal(Math::Vec2f world) const {
        auto rel = world - bound.center;
        return {
            rel.dot(axisX()),
            rel.dot(axisY()),
        };
    }

    Math::Vec2f handlePos(GizmoHandle handle) const {
        auto half = bound.size / 2;

        switch (handle) {
        case GizmoHandle::N:
            return toWorld({0, -half.y});
        case GizmoHandle::NE:
            return toWorld({half.x, -half.y});
        case GizmoHandle::E:
            return toWorld({half.x, 0});
        case GizmoHandle::SE:
            return toWorld({half.x, half.y});
        case GizmoHandle::S:
            return toWorld({0, half.y});
        case GizmoHandle::SW:
            return toWorld({-half.x, half.y});
        case GizmoHandle::W:
            return toWorld({-half.x, 0});
        case GizmoHandle::NW:
            return toWorld({-half.x, -half.y});
        case GizmoHandle::ROTATE:
            return toWorld({0, -half.y - 16});
        default:
            return bound.center;
        }
    }

    Vec<GizmoHandle> handles() const {
        if (uniformOnly) {
            return {
                GizmoHandle::NW,
                GizmoHandle::NE,
                GizmoHandle::SE,
                GizmoHandle::SW,
                GizmoHandle::ROTATE,
            };
        }

        return {
            GizmoHandle::N,
            GizmoHandle::NE,
            GizmoHandle::E,
            GizmoHandle::SE,
            GizmoHandle::S,
            GizmoHandle::SW,
            GizmoHandle::W,
            GizmoHandle::NW,
            GizmoHandle::ROTATE,
        };
    }

    GizmoHandle hitHandle(Math::Vec2f pos, f64 radius = 10) const {
        for (auto handle : handles()) {
            if (handlePos(handle).dist(pos) <= radius)
                return handle;
        }

        return GizmoHandle::NONE;
    }

    bool isResizeHandle(GizmoHandle handle) const {
        return handle != GizmoHandle::NONE and handle != GizmoHandle::ROTATE;
    }

    void paint(Gfx::Canvas& g) const {
        auto points = bound.points();

        g.push();
        g.beginPath();
        g.moveTo(points[0]);
        g.lineTo(points[1]);
        g.lineTo(points[2]);
        g.lineTo(points[3]);
        g.lineTo(points[0]);
        g.stroke({.fill = Ui::ACCENT500, .width = 1});

        auto rotatePos = handlePos(GizmoHandle::ROTATE);
        auto topCenter = handlePos(GizmoHandle::N);
        g.beginPath();
        g.moveTo(topCenter);
        g.lineTo(rotatePos);
        g.stroke({.fill = Ui::ACCENT500, .width = 1});

        for (auto handle : handles()) {
            auto pos = handlePos(handle);
            auto circle = Math::Ellipsef{pos, 4.0};
            g.fillStyle(Gfx::WHITE);
            g.fill(circle);
            g.strokeStyle({.fill = Ui::ACCENT500, .width = 1});
            g.stroke(circle);
        }
        g.pop();
    }

    Math::Vec2f oppositePivot(GizmoHandle handle) const {
        auto half = bound.size / 2;

        switch (handle) {
        case GizmoHandle::N:
            return {0, half.y};
        case GizmoHandle::NE:
            return {-half.x, half.y};
        case GizmoHandle::E:
            return {-half.x, 0};
        case GizmoHandle::SE:
            return {-half.x, -half.y};
        case GizmoHandle::S:
            return {0, -half.y};
        case GizmoHandle::SW:
            return {half.x, -half.y};
        case GizmoHandle::W:
            return {half.x, 0};
        case GizmoHandle::NW:
            return {half.x, half.y};
        default:
            return {0, 0};
        }
    }

    Pair<f64, f64> resizeScale(GizmoHandle handle, Math::Vec2f dragPos, bool forceUniform) const {
        auto half = bound.size / 2;
        auto local = toLocal(dragPos);

        auto minX = -half.x;
        auto maxX = half.x;
        auto minY = -half.y;
        auto maxY = half.y;

        if (handle == GizmoHandle::N or handle == GizmoHandle::NE or handle == GizmoHandle::NW)
            minY = local.y;
        if (handle == GizmoHandle::S or handle == GizmoHandle::SE or handle == GizmoHandle::SW)
            maxY = local.y;
        if (handle == GizmoHandle::W or handle == GizmoHandle::NW or handle == GizmoHandle::SW)
            minX = local.x;
        if (handle == GizmoHandle::E or handle == GizmoHandle::NE or handle == GizmoHandle::SE)
            maxX = local.x;

        auto oldW = bound.size.x;
        auto oldH = bound.size.y;

        auto newW = max(maxX - minX, 4.0);
        auto newH = max(maxY - minY, 4.0);

        f64 sx = newW / oldW;
        f64 sy = newH / oldH;

        bool uniform = uniformOnly or forceUniform;
        if (uniform) {
            auto k = max(Math::abs(sx), Math::abs(sy));
            k = max(k, 0.02);
            sx = k;
            sy = k;
        }

        return {sx, sy};
    }
};

} // namespace Hideo::Canvas
