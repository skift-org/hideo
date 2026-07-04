export module Hideo.Canvas.Freehand;

import Karm.Core;
import Karm.Math;

using namespace Karm;

namespace Hideo::Canvas {

constexpr f64 RATE_OF_PRESSURE_CHANGE = 0.275;
constexpr f64 FIXED_PI = Math::PI + 0.0001;

export struct TaperOptions {
    bool cap = true;
    Union<bool, f64> taper = 0.;
    Math::Easing easing = Math::Easing::linear;
};

export struct StrokeOptions {
    f64 size = 6;
    f64 thinning = .6;
    f64 smoothing = .5;
    f64 streamline = .5;
    Math::Easing easing = Math::Easing::linear;
    bool simulatePressure = true;
    TaperOptions start = {};
    TaperOptions end = {};
    bool complete = true;
};

export struct StrokePoint {
    Math::Vec2f point;
    f64 pressure;
    f64 distance;
    Math::Vec2f vector;
    f64 runningLength;
};

export struct InputPoint {
    Math::Vec2f pos;
    f64 pressure = 0.5;
};

static f64 _easeStrokeRadius(f64 size, f64 thinning, f64 pressure, auto const& easing) {
    f64 t = 0.5 - thinning * (0.5 - pressure);
    t = easing(t);
    return size * t;
}

static Vec<StrokePoint> _processInputPoints(Slice<InputPoint> points, StrokeOptions const& options) {
    if (isEmpty(points))
        return {};

    f64 t = 0.15 + (1.0 - options.streamline) * 0.85;

    Vec<InputPoint> pts = points;

    if (pts.len() == 2) {
        auto last = pts[1];
        pts.trunc(1);
        for (usize i = 1; i < 5; ++i) {
            f64 step = (f64)i / 4.0;
            pts.pushBack({Math::lerp(pts[0].pos, last.pos, step), 0.5});
        }
    }

    if (pts.len() == 1) {
        pts.pushBack({
            pts[0].pos + Math::Vec2f{1.0, 1.0},
            pts[0].pressure,
        });
    }

    Vec<StrokePoint> strokePoints;
    strokePoints.pushBack({
        pts[0].pos,
        pts[0].pressure >= 0.0 ? pts[0].pressure : 0.25,
        0.0,
        {1.0, 1.0},
        0.0,
    });

    bool hasReachedMinimumLength = false;
    f64 runningLength = 0.0;
    StrokePoint prev = strokePoints[0];
    usize max = pts.len() - 1;

    for (usize i = 1; i < pts.len(); ++i) {
        Math::Vec2f point = (options.complete and i == max) ? pts[i].pos : Math::lerp(prev.point, pts[i].pos, t);

        if (prev.point.x == point.x and prev.point.y == point.y)
            continue;

        f64 d = point.dist(prev.point);
        runningLength += d;

        if (i < max and not hasReachedMinimumLength) {
            if (runningLength < options.size)
                continue;
            hasReachedMinimumLength = true;
        }

        prev = {
            point,
            pts[i].pressure >= 0.0 ? pts[i].pressure : 0.5,
            d,
            (prev.point - point).unit(),
            runningLength
        };

        strokePoints.pushBack(prev);
    }

    if (strokePoints.len() > 1) {
        strokePoints[0].vector = strokePoints[1].vector;
    }

    return strokePoints;
}

static Vec<Math::Vec2f> _expandStroke(Slice<StrokePoint> points, StrokeOptions const& options) {
    if (isEmpty(points) or options.size <= 0.0)
        return {};

    f64 totalLength = last(points).runningLength;

    auto resolveTaper = [&](Opt<Union<bool, f64>> const& t) -> f64 {
        if (not t)
            return 0.0;
        return t.unwrap().visit(
            [&](bool b) {
                return b ? max(options.size, totalLength) : 0.0;
            },
            [&](f64 v) {
                return v;
            }
        );
    };

    f64 taperStart = resolveTaper(options.start.taper);
    f64 taperEnd = resolveTaper(options.end.taper);

    f64 minDistance = Math::pow(options.size * options.smoothing, 2.0);

    Vec<Math::Vec2f> leftPts;
    Vec<Math::Vec2f> rightPts;

    f64 prevPressure = points[0].pressure;
    f64 pressureAcc = 0.0;
    isize avgCount = min((isize)10, (isize)points.len());

    for (isize i = 0; i < avgCount; i++) {
        f64 currPressure = points[i].pressure;
        if (options.simulatePressure) {
            f64 sp = min(1.0, points[i].distance / options.size);
            f64 rp = min(1.0, 1.0 - sp);
            currPressure = min(1.0, prevPressure + (rp - prevPressure) * (sp * RATE_OF_PRESSURE_CHANGE));
        }
        pressureAcc = (i == 0) ? currPressure : (pressureAcc + currPressure) / 2.0;
        prevPressure = pressureAcc;
    }

    f64 radius = _easeStrokeRadius(options.size, options.thinning, last(points).pressure, options.easing);
    Opt<f64> firstRadius = NONE;
    Math::Vec2f prevVector = points[0].vector;
    Math::Vec2f pl = points[0].point;
    Math::Vec2f pr = pl;
    Math::Vec2f tl = pl;
    Math::Vec2f tr = pr;
    bool isPrevPointSharpCorner = false;

    for (usize i = 0; i < points.len(); ++i) {
        f64 pressure = points[i].pressure;
        Math::Vec2f point = points[i].point;
        Math::Vec2f vector = points[i].vector;
        f64 distance = points[i].distance;
        f64 runningLength = points[i].runningLength;

        if (i < points.len() - 1 and totalLength - runningLength < 3.0)
            continue;

        if (options.thinning > 0.0) {
            if (options.simulatePressure) {
                f64 sp = min(1.0, distance / options.size);
                f64 rp = min(1.0, 1.0 - sp);
                pressure = min(1.0, prevPressure + (rp - prevPressure) * (sp * RATE_OF_PRESSURE_CHANGE));
            }
            radius = _easeStrokeRadius(options.size, options.thinning, pressure, options.easing);
        } else {
            radius = options.size / 2.0;
        }

        if (not firstRadius)
            firstRadius = radius;

        f64 ts = (runningLength < taperStart) ? options.start.easing(runningLength / taperStart) : 1.0;
        f64 te = (totalLength - runningLength < taperEnd) ? options.end.easing((totalLength - runningLength) / taperEnd) : 1.0;

        radius = max(0.01, radius * min(ts, te));

        Math::Vec2f nextVector = (i < points.len() - 1) ? points[i + 1].vector : points[i].vector;
        f64 nextDpr = (i < points.len() - 1) ? vector.dot(nextVector) : 1.0;
        f64 prevDpr = vector.dot(prevVector);

        bool isPointSharpCorner = prevDpr < 0.0 and not isPrevPointSharpCorner;
        bool isNextPointSharpCorner = nextDpr < 0.0;

        if (isPointSharpCorner or isNextPointSharpCorner) {
            Math::Vec2f offset = prevVector.normalInv() * radius;
            for (isize j = 0; j <= 13; ++j) {
                f64 step = (f64)j / 13.0;
                tl = (point - offset).rotateAround(point, FIXED_PI * step);
                leftPts.pushBack(tl);
                tr = (point + offset).rotateAround(point, FIXED_PI * -step);
                rightPts.pushBack(tr);
            }
            pl = tl;
            pr = tr;
            if (isNextPointSharpCorner)
                isPrevPointSharpCorner = true;
            continue;
        }

        isPrevPointSharpCorner = false;

        if (i == points.len() - 1) {
            Math::Vec2f offset = vector.normalInv() * radius;
            leftPts.pushBack(point - offset);
            rightPts.pushBack(point + offset);
            continue;
        }

        Math::Vec2f offset = lerp(nextVector, vector, nextDpr).normalInv() * radius;
        tl = point - offset;
        if (i <= 1 or pl.distSq(tl) > minDistance) {
            leftPts.pushBack(tl);
            pl = tl;
        }

        tr = point + offset;
        if (i <= 1 or pr.distSq(tr) > minDistance) {
            rightPts.pushBack(tr);
            pr = tr;
        }

        prevPressure = pressure;
        prevVector = vector;
    }

    Math::Vec2f firstPoint = points[0].point;
    Math::Vec2f lastPoint =
        points.len() > 1
            ? last(points).point
            : points[0].point + Math::Vec2f{1.0, 1.0};

    Vec<Math::Vec2f> startCap;
    Vec<Math::Vec2f> endCap;

    if (points.len() == 1) {
        if (not(taperStart > 0.0 or taperEnd > 0.0) or options.complete) {
            Math::Vec2f start = firstPoint.projectAlong((firstPoint - lastPoint).normalInv().unit(), -(firstRadius.unwrapOr(radius)));
            Vec<Math::Vec2f> dotPts;
            for (isize i = 1; i <= 13; ++i) {
                f64 step = (f64)i / 13.0;
                dotPts.pushBack(start.rotateAround(firstPoint, FIXED_PI * 2.0 * step));
            }
            return dotPts;
        }
    } else {
        if (taperStart > 0.0 or (taperEnd > 0.0 and points.len() == 1)) {
            // Tapered start, noop
        } else if (options.start.cap) {
            for (isize i = 1; i <= 13; ++i) {
                f64 step = (f64)i / 13.0;
                startCap.pushBack(rightPts[0].rotateAround(firstPoint, FIXED_PI * step));
            }
        } else {
            Math::Vec2f cornersVector = leftPts[0] - rightPts[0];
            Math::Vec2f offsetA = cornersVector * 0.5;
            Math::Vec2f offsetB = cornersVector * 0.51;
            startCap.pushBack(firstPoint - offsetA);
            startCap.pushBack(firstPoint - offsetB);
            startCap.pushBack(firstPoint + offsetB);
            startCap.pushBack(firstPoint + offsetA);
        }

        Math::Vec2f direction = (-last(points).vector).normalInv();

        if (taperEnd > 0.0 or (taperStart > 0.0 and points.len() == 1)) {
            endCap.pushBack(lastPoint);
        } else if (options.end.cap) {
            Math::Vec2f start = lastPoint.projectAlong(direction, radius);
            for (isize i = 1; i < 29; ++i) {
                f64 step = (f64)i / 29.0;
                endCap.pushBack(start.rotateAround(lastPoint, FIXED_PI * 3.0 * step));
            }
        } else {
            endCap.pushBack(lastPoint + (direction * radius));
            endCap.pushBack(lastPoint + (direction * (radius * 0.99)));
            endCap.pushBack(lastPoint - (direction * (radius * 0.99)));
            endCap.pushBack(lastPoint - (direction * radius));
        }
    }

    Vec<Math::Vec2f> result;
    for (auto p : leftPts)
        result.pushBack(p);
    for (auto p : endCap)
        result.pushBack(p);
    for (isize i = rightPts.len() - 1; i >= 0; --i)
        result.pushBack(rightPts[i]);
    for (auto p : startCap)
        result.pushBack(p);
    return result;
}

export Math::Path strokeToPath(Slice<Math::Vec2f> points, bool closed = true) {
    Math::Path path;

    // Fallback for shapes that don't have enough points for bezier smoothing
    if (points.len() < 4) {
        if (points.len() > 0) {
            path.moveTo(points[0]);
            for (usize i = 1; i < points.len(); ++i) {
                path.lineTo(points[i]);
            }
            if (closed) {
                path.close();
            }
        }
        return path;
    }

    Math::Vec2f a = points[0];
    Math::Vec2f b = points[1];
    Math::Vec2f c = points[2];

    path.moveTo(a);

    // Draw a quadratic curve to the midpoint of the second and third points
    path.quadTo(b, Math::lerp(b, c, 0.5));

    // For the rest of the points, draw smooth quadratic curves (SVG 'T' command)
    // to the midpoints of the segments.
    for (usize i = 2; i < points.len() - 1; ++i) {
        a = points[i];
        b = points[i + 1];
        path.smoothQuadTo(Math::lerp(a, b, 0.5));
    }

    if (closed) {
        path.close();
    }

    return path;
}

export Vec<Math::Vec2f> expandFreehand(Slice<InputPoint> points, StrokeOptions const& options) {
    auto strokePoints = _processInputPoints(points, options);
    return _expandStroke(strokePoints, options);
}

} // namespace Hideo::Canvas
