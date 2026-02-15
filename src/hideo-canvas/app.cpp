export module Hideo.Canvas:app;

import Mdi;
import Karm.Core;
import Karm.Ui;
import Karm.Kira;
import Karm.Gfx;
import Karm.Math;
import Karm.App;
import :model;

using namespace Karm;

namespace Hideo::Canvas {

Gfx::Color _kindColor(Kind kind) {
    switch (kind) {
    case Kind::FRAME:
        return Gfx::WHITE;
    case Kind::RECT:
        return Gfx::GRAY400;
    case Kind::TEXT:
        return Gfx::GRAY400;
    case Kind::GROUP:
        return Gfx::GRAY400;
    default:
        unreachable();
    }
}

// MARK: Model -----------------------------------------------------------------

struct Canvas : Ui::View<Canvas> {
    State const& _state;
    Ui::MouseListener _listener;

    Canvas(State const& state)
        : _state(state) {}

    void _paintChildren(Gfx::Canvas& g, Ref parent) {
        for (auto const& child : _state.tree._nodes) {
            if (child.parent == parent)
                _paintNode(g, child.ref);
        }
    }

    void _paintNode(Gfx::Canvas& g, Ref ref) {
        auto const& node = _state.tree.byRef(ref);
        auto rect = Math::Rectf::fromCenter({0, 0}, node.bound.size);

        g.push();
        g.translate(node.bound.center);
        g.rotate(node.bound.angle);
        g.fillStyle(_kindColor(node.kind));
        g.fill(rect);

        if (node.kind == Kind::FRAME) {
            g.strokeStyle({.fill = Ui::GRAY700, .width = 1});
            g.stroke(rect);
        }

        if (_state.selection.selected(node.ref)) {
            g.strokeStyle({.fill = Ui::ACCENT500, .width = 1});
            g.stroke(rect);
        }
        g.pop();

        if (node.kind == Kind::FRAME) {
            g.push();
            g.translate(node.bound.center);
            g.rotate(node.bound.angle);
            g.clip(rect);

            g.push();
            g.rotate(-node.bound.angle);
            g.translate(-node.bound.center);

            _paintChildren(g, ref);
            g.pop();
            g.pop();
            return;
        }

        _paintChildren(g, ref);
    }

    void paint(Gfx::Canvas& g, Math::Recti) override {
        g.push();
        g.clip(bound());

        for (auto const& node : _state.tree._nodes) {
            if (node.topLevel())
                _paintNode(g, node.ref);
        }

        if (auto selectionRect = _state.selectionRect(); selectionRect) {
            Kr::paintSelection(g, selectionRect.unwrap());
        }

        if (auto gizmo = _state.gizmo(); gizmo) {
            gizmo.unwrap().paint(g);
        }
        g.pop();
    }

    void event(App::Event& event) override {
        if (auto e = event.is<App::MouseEvent>(); e and bound().contains(e->pos)) {
            switch (e->type) {
            case App::MouseEvent::PRESS:
                Model::bubble<CanvasPress>(
                    *this,
                    {
                        .pos = e->pos.cast<f64>(),
                        .resize = App::match(e->mods, App::KeyMod::SHIFT),
                    }
                );
                break;

            case App::MouseEvent::RELEASE:
                Model::bubble<CanvasRelease>(
                    *this,
                    {e->pos.cast<f64>()}
                );
                break;

            case App::MouseEvent::SCROLL:
                break;

            case App::MouseEvent::MOVE:
                if (e->buttons.has(App::MouseButton::LEFT))
                    Model::bubble<CanvasDrag>(
                        *this,
                        {
                            .pos = e->pos.cast<f64>(),
                            .uniformResize = App::match(e->mods, App::KeyMod::SHIFT),
                        }
                    );
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
               Kr::labelRow("Position"s),
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
