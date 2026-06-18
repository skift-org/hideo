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
using namespace Karm::Literals;

namespace Hideo::Canvas {

Gfx::Color _kindColor(Kind kind) {
    switch (kind) {
    case Kind::FRAME:
        return Gfx::WHITE;
    case Kind::FREEHAND:
        return Gfx::PINK500;
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

Ui::Child canvasContextMenu(State const& s) {
    return Kr::contextMenuContent({
        Kr::contextMenuItem(Model::bindIf<CopySelection>(not s.selection.empty()), Mdi::CONTENT_COPY, "Copy"s),
        Kr::contextMenuItem(Model::bindIf<CutSelection>(not s.selection.empty()), Mdi::CONTENT_CUT, "Cut"s),
        Kr::contextMenuItem(Model::bindIf<PasteSelection>(s.clipboard.has()), Mdi::CONTENT_PASTE, "Paste"s),
        Kr::contextMenuItem(Model::bindIf<DeleteSelection>(not s.selection.empty()), Mdi::TRASH_CAN, "Delete"),
        Kr::separator(),
        Kr::contextMenuItem(Model::bind<SelectAll>(), Mdi::SELECT_ALL, "Select All"),
        Kr::separator(),
        Kr::contextMenuItem(Model::bindIf<FrameSelection>(not s.selection.empty()), Mdi::ARTBOARD, "Frame Selection"),
    });
}

struct Viewport : Ui::View<Viewport> {
    State const& _state;
    Ui::MouseListener _listener;

    Viewport(State const& state)
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

        if (not node.freehand) {
            g.fillStyle(_kindColor(node.kind));
            g.fill(rect);
        }

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

        if (node.freehand) {
            auto path = strokeToPath(expandFreehand(node.freehand, {}));
            g.fillStyle(node.freehandColor);
            g.fill(path, Gfx::FillRule::NONZERO);
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
                if (e->button == App::MouseButton::RIGHT) {
                    Ui::showPopover(*this, e->pos, canvasContextMenu(_state));
                } else {
                    Model::bubble<CanvasPress>(
                        *this,
                        {
                            .pos = e->pos.cast<f64>(),
                            .resize = App::match(e->mods, App::KeyMod::SHIFT),
                        }
                    );
                }
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
                            .pos = (e->pos).cast<f64>(),
                            .mods = e->mods,
                        }
                    );
                break;

            default:
                unreachable();
            }
        }
    }
};

Ui::Child viewport(State const& s) {
    return makeRc<Viewport>(s) |
           Ui::keyboardShortcut(App::Key::A, App::KeyMod::CTRL, Model::bind<SelectAll>()) |
           Ui::keyboardShortcut(App::Key::C, App::KeyMod::CTRL, Model::bind<CopySelection>()) |
           Ui::keyboardShortcut(App::Key::X, App::KeyMod::CTRL, Model::bind<CutSelection>()) |
           Ui::keyboardShortcut(App::Key::V, App::KeyMod::CTRL, Model::bind<PasteSelection>()) |
           Ui::keyboardShortcut(App::Key::DELETE, Model::bind<DeleteSelection>()) |
           Ui::keyboardShortcut(App::Key::BKSPC, Model::bind<DeleteSelection>());
}

Ui::Child colorButton(State const& s, Gfx::Color color) {
    return (s.freehandColor == color ? Ui::icon(Mdi::CHECK, color.luminance() > 0.7 ? Gfx::BLACK : Gfx::WHITE) : Ui::empty(18)) |
           Ui::box({
               .padding = 2,
               .borderRadii = 99,
               .backgroundFill = color,
           }) |
           Ui::button(Model::bind<ChooseFreeHandColor>(color), Ui::ButtonStyle::subtle().withRadii(99));
}

Ui::Child colorBar(State const& s) {
    return Ui::hflow(
               4,
               colorButton(s, Gfx::WHITE),
               colorButton(s, Gfx::BLACK),
               colorButton(s, Gfx::RED),
               colorButton(s, Gfx::ORANGE),
               colorButton(s, Gfx::AMBER),
               colorButton(s, Gfx::YELLOW),
               colorButton(s, Gfx::LIME),
               colorButton(s, Gfx::GREEN),
               colorButton(s, Gfx::EMERALD),
               colorButton(s, Gfx::TEAL),
               colorButton(s, Gfx::CYAN),
               colorButton(s, Gfx::SKY),
               colorButton(s, Gfx::BLUE),
               colorButton(s, Gfx::INDIGO),
               colorButton(s, Gfx::VIOLET),
               colorButton(s, Gfx::PURPLE),
               colorButton(s, Gfx::FUCHSIA),
               colorButton(s, Gfx::PINK),
               colorButton(s, Gfx::ROSE)
           ) |
           Ui::box({
               .margin = 4,
               .padding = 4,
               .borderRadii = 99,
               .backgroundFill = Ui::GRAY900,
           });
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
               toolbarButton(s, Tool::FREEHAND, Mdi::GESTURE),
               toolbarButton(s, Tool::FRAME, Mdi::ARTBOARD),
               toolbarButton(s, Tool::RECT, Mdi::RECTANGLE),
               toolbarButton(s, Tool::TEXT, Mdi::FORMAT_TEXT)
           ) |
           Ui::box({
               .padding = 2,
               .borderRadii = 4,
               .backgroundFill = Ui::GRAY900,
           });
}

Ui::Child toolbarZoom() {
    return Ui::button(Ui::SINK<>, Mdi::MAGNIFY, "100%");
}

Ui::Child toolbarFormat() {
    return Ui::button(Ui::SINK<>, Mdi::FORMAT_TEXTBOX);
}

Ui::Child viewportPanel(State const& s) {
    return Ui::stack(
               viewport(s),
               Ui::stack(
                   toolbar(s) | Ui::align(Math::Align::BOTTOM | Math::Align::HCENTER),
                   colorBar(s) | Ui::align(Math::Align::TOP | Math::Align::HCENTER)
                   // toolbarZoom() | Ui::align(Math::Align::BOTTOM | Math::Align::START) | Ui::insets(2),
                   // toolbarFormat() | Ui::align(Math::Align::TOP | Math::Align::END) | Ui::insets(2)
               ) | Ui::insets(16)
           ) |
           Kr::scaffoldContent();
}

Ui::Child propertiesPanel(State const&) {
    return Ui::empty(240) | Kr::scaffoldContent();
}

export Ui::Child app() {
    return Ui::reducer<Model>([](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::DRAW,
            .title = "Canvas"s,
            .endTools = [&] -> Ui::Children {
                return {
                    Ui::button(Model::bind<ToggleProperties>(), Ui::ButtonStyle::subtle(), Mdi::TUNE)
                };
            },
            .sidebar = [&] {
                return Kr::sidenavContent({}) | Kr::resizable(Kr::ResizeHandlePosition::END);
            },
            .body = [&] {
                if (s.propertiesVisible) {
                    return Ui::hflow(
                        2,
                        viewportPanel(s) | Ui::grow(),
                        propertiesPanel(s)
                    );
                } else {
                    return viewportPanel(s);
                }
            },
        });
    });
}

} // namespace Hideo::Canvas
