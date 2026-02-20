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

Ui::Child canvas(State const& s) {
    return makeRc<Canvas>(s) |
           Ui::keyboardShortcut(App::Key::A, App::KeyMod::CTRL, Model::bind<SelectAll>()) |
           Ui::keyboardShortcut(App::Key::C, App::KeyMod::CTRL, Model::bind<CopySelection>()) |
           Ui::keyboardShortcut(App::Key::X, App::KeyMod::CTRL, Model::bind<CutSelection>()) |
           Ui::keyboardShortcut(App::Key::V, App::KeyMod::CTRL, Model::bind<PasteSelection>()) |
           Ui::keyboardShortcut(App::Key::DELETE, Model::bind<DeleteSelection>()) |
           Ui::keyboardShortcut(App::Key::BKSPC, Model::bind<DeleteSelection>());
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

export Ui::Child app() {
    return Ui::reducer<Model>([](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::DRAW,
            .title = "Canvas"s,
            .body = [&] {
                return Ui::stack(
                           canvas(s),
                           Ui::stack(
                               toolbar(s) | Ui::align(Math::Align::BOTTOM | Math::Align::HCENTER)
                               // toolbarZoom() | Ui::align(Math::Align::BOTTOM | Math::Align::START) | Ui::insets(2),
                               // toolbarFormat() | Ui::align(Math::Align::TOP | Math::Align::END) | Ui::insets(2)
                           ) | Ui::insets(16)
                       ) |
                       Kr::scaffoldContent();
            },
        });
    });
}

} // namespace Hideo::Canvas
