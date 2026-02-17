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
};

export struct Gizmo {
    static constexpr int RESIZE_HANDLE_HIT_RADIUS = 4;
    static constexpr int ROTATE_HANDLE_HIT_RADIUS = 8;

    Obb bound;
    bool uniformOnly = false;

    Math::Vec2f handlePos(GizmoHandle handle) const {
        auto half = bound.size / 2;

        switch (handle) {
        case GizmoHandle::N:
            return bound.toWorld({0, -half.y});
        case GizmoHandle::NE:
            return bound.toWorld({half.x, -half.y});
        case GizmoHandle::E:
            return bound.toWorld({half.x, 0});
        case GizmoHandle::SE:
            return bound.toWorld({half.x, half.y});
        case GizmoHandle::S:
            return bound.toWorld({0, half.y});
        case GizmoHandle::SW:
            return bound.toWorld({-half.x, half.y});
        case GizmoHandle::W:
            return bound.toWorld({-half.x, 0});
        case GizmoHandle::NW:
            return bound.toWorld({-half.x, -half.y});
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
        };
    }

    Tuple<GizmoHandle, bool> hitHandle(Math::Vec2f pos) const {
        for (auto handle : handles()) {
            if (handlePos(handle).dist(pos) <= RESIZE_HANDLE_HIT_RADIUS)
                return {handle, false};
        }

        if (not bound.contains(pos)) {
            for (auto handle : handles()) {
                if (handlePos(handle).dist(pos) <= ROTATE_HANDLE_HIT_RADIUS)
                    return {handle, true};
            }
        }

        return {GizmoHandle::NONE, false};
    }

    void paint(Gfx::Canvas& g) const {
        auto rect = Math::Rectf::fromCenter({0, 0}, bound.size);

        g.push();
        g.translate(bound.center);
        g.rotate(bound.angle);
        g.strokeStyle({.fill = Ui::ACCENT500, .width = 1});
        g.stroke(rect);
        g.pop();

        g.push();
        for (auto handle : handles()) {
            auto pos = handlePos(handle);
            auto circle = Math::Ellipsef{pos, RESIZE_HANDLE_HIT_RADIUS};
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

    Math::Vec2f resizeScale(GizmoHandle handle, Math::Vec2f dragPos, bool forceUniform) const {
        auto half = bound.size / 2;
        auto local = bound.toLocal(dragPos);

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
