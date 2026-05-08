module;

#include <karm/macros>

export module Hideo.Graph;

import Mdi;
import Karm.Core;
import Karm.Ui;
import Karm.Kira;
import Karm.Gfx;
import Karm.Math;
import Karm.Logger;

using namespace Karm;
using namespace Karm::Literals;

namespace Hideo::Graph {

// MARK: Interval Arithmetic ---------------------------------------------------

struct Interval {
    f64 low;
    f64 high;

    static constexpr Interval empty() {
        return {Math::NAN, Math::NAN};
    }

    static constexpr Interval entire() {
        return {Math::NEG_INF, Math::INF};
    }

    Interval(f64 const& scalar)
        : low{scalar}, high{scalar} {}

    Interval(f64 low, f64 high)
        : low{low}, high{high} {}

    f64 mid() const {
        return (low + high) / 2;
    }

    f64 span() const {
        return high - low;
    }

    bool isEmpty() const {
        return low >= high;
    }

    Interval inverse() const { // 1 / x
        return {1 / low, 1 / high};
    }

    Interval operator+(Interval const& other) const {
        return {low + other.low, high + other.high};
    }

    Interval operator-(Interval const& other) const {
        return {low - other.high, high - other.low};
    }

    Interval operator*(Interval const& other) const {
        Array products{
            low * other.low,
            low * other.high,
            high * other.low,
            high * other.high,
        };

        return Interval{
            (iter(products) | Min()).unwrapOr(0),
            (iter(products) | Max()).unwrapOr(0),
        };
    }

    Interval operator/(Interval const& other) const {
        if (other.low <= 0.0 and other.high >= 0.0)
            return entire();
        return *this * other.inverse();
    }

    bool overlaps(Interval const& other) const {
        return low < other.high and high > other.low;
    }

    bool contains(f64 value) const {
        return low <= value and high >= value;
    }
};

export Interval sin(Interval const& x) {
    if (x.span() >= Math::TAU)
        return {-1.0, 1.0};

    f64 sinLow = Math::sin(x.low);
    f64 sin_high = Math::sin(x.high);

    f64 minVal = min(sinLow, sin_high);
    f64 maxVal = max(sinLow, sin_high);

    f64 nextPeak = Math::ceil((x.low - Math::HALF_PI) / Math::TAU) * Math::TAU + Math::HALF_PI;
    if (nextPeak <= x.high)
        maxVal = 1.0;

    f64 nextValley = Math::ceil((x.low - 3.0 * Math::HALF_PI) / Math::TAU) * Math::TAU + 3.0 * Math::HALF_PI;
    if (nextValley <= x.high)
        minVal = -1.0;

    return {minVal, maxVal};
}

export Interval cos(Interval const& x) {
    if (x.span() >= Math::TAU)
        return {-1.0, 1.0};

    f64 cosLow = Math::cos(x.low);
    f64 cosHigh = Math::cos(x.high);

    f64 minVal = min(cosLow, cosHigh);
    f64 maxVal = max(cosLow, cosHigh);

    f64 nextPeak = Math::ceil(x.low / Math::TAU) * Math::TAU;
    if (nextPeak <= x.high)
        maxVal = 1.0;

    f64 nextValey = Math::ceil((x.low - Math::PI) / Math::TAU) * Math::TAU + Math::PI;
    if (nextValey <= x.high)
        minVal = -1.0;

    return {minVal, maxVal};
}

// MARK: Graphing --------------------------------------------------------------

Rc<Gfx::Surface> graph(auto f, Interval xrange, Interval yrange) {
    auto surface = Gfx::Surface::alloc(1024);
    surface->mutPixels().clear(Gfx::BLUE.withOpacity(0.5));

    Vec<Math::Recti> u = {
        {0, 0, 1024, 1024},
    };

    while (u) {
        Vec<Math::Recti> uNext;
        for (auto r : u) {
            Interval xinput{
                xrange.low + r.start() * (xrange.high - xrange.low) / 1024,
                xrange.low + r.end() * (xrange.high - xrange.low) / 1024,
            };

            Interval yinput{
                yrange.low + r.top() * (yrange.high - yrange.low) / 1024,
                yrange.low + r.bottom() * (yrange.high - yrange.low) / 1024,
            };

            auto result = f(xinput, yinput);

            if (result.overlaps(0)) {
                if (r.width > 1 or r.height > 1) {
                    auto [s, e] = r.vsplit(r.width / 2);
                    auto [st, sb] = s.hsplit(r.height / 2);
                    auto [et, eb] = e.hsplit(r.height / 2);
                    uNext.pushBack(st);
                    uNext.pushBack(sb);
                    uNext.pushBack(et);
                    uNext.pushBack(eb);
                }
            } else {
                surface
                    ->mutPixels()
                    .clip(r)
                    .clear(Gfx::randomColor().withOpacity(0.1));
            }
        }
        u = uNext;
    }
    return surface;
}

// MARK: Application -----------------------------------------------------------

struct Relation {
    Gfx::Color color;
    String expression;
};

struct State {
    Rc<Gfx::Surface> tile;
    Vec<Relation> relations = {
        Relation{.color = Gfx::BLUE, .expression = "1/x=y"s}
    };
};

struct AddRelation {
};

using Action = Union<AddRelation>;

Ui::Task<Action> reduce(State& state, Action action) {
    action.visit(Visitor{
        [&](AddRelation) {
            state.relations.emplaceBack();
        },
    });
    return NONE;
}

using Model = Ui::Model<State, Action, reduce>;

export Ui::Child relationRow(Relation const& relation) {
    return Ui::hflow(
        4,
        Ui::icon(Mdi::DRAG_VERTICAL_VARIANT) | Ui::vcenter(),
        Ui::button(Ui::SINK<>, Ui::ButtonStyle::subtle().withForegroundFill(relation.color), Mdi::CIRCLE),
        Ui::codeMedium(relation.expression) | Ui::vcenter() | Ui::grow(),
        Ui::button(Ui::SINK<>, Ui::ButtonStyle::subtle(), Mdi::FUNCTION_VARIANT),
        Ui::button(Ui::SINK<>, Ui::ButtonStyle::subtle(), Mdi::DOTS_HORIZONTAL)
    );
}

export Ui::Child sidenav(State const& s) {
    Ui::Children items;
    for (auto const& relation : s.relations)
        items.pushBack(relationRow(relation));
    items.pushBack(Ui::button(Model::bind<AddRelation>(), Mdi::PLUS, "Add Relation"s));
    return Kr::sidenavContent(std::move(items));
}

export Ui::Child app() {
    auto g = graph(
        [](auto x, auto y) {
            return (Interval{1} / x) - y;
        },
        {-10, 10},
        {-10, 10}
    );

    return Ui::reducer<Model>({g}, [](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::GRAPH,
            .title = "Graph"s,
            .sidebar = [&] {
                return sidenav(s);
            },
            .body = [&] {
                return Ui::image(s.tile) |
                       Ui::bound() |
                       Ui::box({
                           .borderRadii = 6,
                           .backgroundFill = Gfx::WHITE,
                           .overflow = Ui::BoxOverflow::HIDDEN,
                       });
            },
        });
    });
}

} // namespace Hideo::Graph
