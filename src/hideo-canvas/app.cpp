export module Hideo.Canvas;

import Mdi;
import Karm.Core;
import Karm.Ui;
import Karm.Kira;
import Karm.Gfx;
import Karm.Math;
import Karm.Kira;
import Karm.App;

using namespace Karm;

namespace Hideo::Canvas {

enum struct Tool {
    SELECT,
    FRAME,
    TEXT,
    RECT,
};

enum struct Mode {
    IDLE,
    PLACING,
    RESIZING,
    SELECTING
};

struct Object {
    u64 id;
    Math::Rectf bound;

    Gfx::Color backgroundColor;
};

struct State {
    u64 idAllocator = 1;
    Tool currentTool = Tool::SELECT;
    Mode currentMode = Mode::IDLE;
    Vec<u64> selected;
    Math::Vec2f dragStart;
    Math::Vec2f dragEnd;
    Vec<Object> objects;

    Opt<u64> objectAt(Math::Vec2f pos) {
        for (auto& o : iterRev(objects)) {
            if (o.bound.contains(pos))
                return o.id;
        }
        return NONE;
    }

    Vec<u64> objectAt(Math::Rectf rect) {
        Vec<u64> res;
        for (auto& o : objects) {
            if (o.bound.colide(rect))
                res.pushBack(o.id);
        }
        return res;
    }
};

struct SelectTool {
    Tool tool;
};

struct CanvasPress {
    Math::Vec2f pos;
};

struct CanvasRelease {
    Math::Vec2f pos;
};

struct CanvasDrag {
    Math::Vec2f pos;
};

using Action = Union<SelectTool, CanvasPress, CanvasRelease, CanvasDrag>;

Ui::Task<Action> reduce(State& s, Action a) {
    a.visit(Visitor{
        [&](SelectTool a) {
            s.currentTool = a.tool;
        },
        [&](CanvasPress a) {
            s.dragStart = a.pos;
            s.dragEnd = a.pos;
            if (s.currentTool == Tool::SELECT) {
                s.selected.clear();
                if (auto id = s.objectAt(a.pos); id.has()) {
                    s.selected = {id.unwrap()};
                } else {
                    s.currentMode = Mode::SELECTING;
                }
            } else {
                s.objects.pushBack({
                    .id = s.idAllocator++,
                    .bound = {
                        a.pos,
                        {},
                    },
                    .backgroundColor = s.currentTool == Tool::FRAME ? Gfx::WHITE : Gfx::GRAY400,
                });
                s.currentMode = Mode::PLACING;
            }
        },
        [&](CanvasRelease d) {
            if (s.currentMode == Mode::PLACING) {
                if (s.dragStart.dist(s.dragEnd) < 2) {
                    last(s.objects).bound = Math::Rectf::fromTwoPoint(d.pos, d.pos).grow(50);
                }
            }

            s.currentMode = Mode::IDLE;
            s.currentTool = Tool::SELECT;
        },
        [&](CanvasDrag d) {
            s.dragEnd = d.pos;
            if (s.currentMode == Mode::RESIZING or s.currentMode == Mode::PLACING) {
                last(s.objects).bound = Math::Rectf::fromTwoPoint(s.dragStart, d.pos);
            } else if (s.currentMode == Mode::SELECTING) {
                s.selected = s.objectAt(Math::Rectf::fromTwoPoint(s.dragStart, d.pos));
            }
        },
    });

    return NONE;
}

using Model = Ui::Model<State, Action, reduce>;

// MARK: Model -----------------------------------------------------------------

struct Canvas : Ui::View<Canvas> {
    State const& _state;
    Ui::MouseListener _listener;

    Canvas(State const& state)
        : _state(state) {}

    void paint(Gfx::Canvas& g, Math::Recti) override {
        g.push();
        g.clip(bound());

        for (auto& o : _state.objects) {
            g.push();
            g.beginPath();
            g.rect(o.bound);
            g.fill(o.backgroundColor);

            if (contains(_state.selected, o.id))
                g.stroke({.fill = Ui::ACCENT500, .width = 1});
            g.pop();
        }

        if (_state.currentMode == Mode::SELECTING) {
            Kr::paintSelection(g, Math::Rectf::fromTwoPoint(_state.dragStart, _state.dragEnd));
        }
        g.pop();
    }

    void event(App::Event& event) override {
        if (auto e = event.is<App::MouseEvent>(); e and bound().contains(e->pos)) {
            switch (e->type) {
            case App::MouseEvent::PRESS:
                Model::bubble<CanvasPress>(*this, {e->pos.cast<f64>()});
                break;
            case App::MouseEvent::RELEASE:
                Model::bubble<CanvasRelease>(*this, {e->pos.cast<f64>()});
                break;
            case App::MouseEvent::SCROLL:
                break;
            case App::MouseEvent::MOVE:
                if (e->buttons.has(App::MouseButton::LEFT))
                    Model::bubble<CanvasDrag>(*this, {e->pos.cast<f64>()});
                break;
            default:
                unreachable();
            }
        }
    }
};

Ui::Child canvas(State const& s) {
    return makeRc<Canvas>(s);
}

Ui::Child toolbarButton(State const& s, Tool tool, Gfx::Icon icon) {
    return Ui::button(
        Model::bind<SelectTool>(tool),
        s.currentTool == tool
            ? Ui::ButtonStyle::subtle().withForegroundFill(Ui::ACCENT500)
            : Ui::ButtonStyle::subtle(),
        icon
    );
}

Ui::Child toolbar(State const& s) {
    return Ui::hflow(
               4,
               toolbarButton(s, Tool::SELECT, Mdi::CURSOR_DEFAULT),
               toolbarButton(s, Tool::FRAME, Mdi::ARTBOARD),
               toolbarButton(s, Tool::RECT, Mdi::RECTANGLE),
               toolbarButton(s, Tool::TEXT, Mdi::FORMAT_TEXT)
           ) |
           Ui::box({
               .margin = 16,
               .padding = 2,
               .borderRadii = 4,
               .backgroundFill = Ui::GRAY900,
               .shadowStyle = Gfx::BoxShadow::elevated(4),
           });
}

Ui::Child documentPanel() {
    return Ui::vflow(
               Kr::labelRow("Layers"s)
           ) |
           Ui::box({
               .backgroundFill = Ui::GRAY900,
           }) |
           Ui::minSize({240, Ui::UNCONSTRAINED});
}

Ui::Child propertiesPanel() {
    return Ui::vflow(
               Kr::labelRow("Postion"s),
               Kr::separator(),
               Kr::labelRow("Appearance"s),
               Kr::separator(),
               Kr::labelRow("Fill"s),
               Kr::separator(),
               Kr::labelRow("Stroke"s),
               Kr::separator(),
               Kr::labelRow("Effects"s),
               Kr::separator(),
               Kr::labelRow("Export"s),
               Kr::separator()
           ) |
           Ui::box({
               .backgroundFill = Ui::GRAY900,
           }) |
           Ui::minSize({240, Ui::UNCONSTRAINED});
}

export Ui::Child app() {
    return Ui::reducer<Model>([](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::DRAW,
            .title = "Canvas"s,
            .body = [&] {
                return Ui::hflow(
                    documentPanel(),
                    Kr::separator(),
                    Ui::stack(canvas(s), toolbar(s) | Ui::align(Math::Align::BOTTOM | Math::Align::HCENTER)) | Ui::grow(),
                    Kr::separator(),
                    propertiesPanel()
                );
            },
        });
    });
}

} // namespace Hideo::Canvas
