module;

#include <karm-gfx/canvas.h>

export module Hideo.Camera;

import Mdi;
import Karm.Ui;
import Karm.Kira;
import Karm.Core;
import Karm.Av;
import Karm.App;
import :model;

using namespace Karm;

namespace Hideo::Camera {

struct VideoSurface : Ui::View<VideoSurface> {
    Rc<Av::VideoStream> _stream;
    Opt<Av::VideoFrame> _frame;

    VideoSurface(Rc<Av::VideoStream> stream) : _stream(stream) {}

    void paint(Gfx::Canvas& g, Math::Recti) override {
        if (_frame)
            g.blit(bound(), _frame.unwrap().surface->pixels());
    }

    void event(App::Event& e) override {
        if (auto ae = e.is<Node::AnimateEvent>()) {
            Ui::shouldAnimate(*this);
            Ui::shouldRepaint(*this);
            auto next = _stream->next();
            if (next) {
                if (not _frame)
                    Ui::shouldLayout(*this);
                _frame = next;
            }
        }

        Ui::View<VideoSurface>::event(e);
    }

    Math::Vec2i size(Math::Vec2i size, Ui::Hint hint) override {
        if (not _frame)
            return 0;
        if (hint == Ui::Hint::MIN)
            return _frame->surface->bound().fit(Math::Recti{size}).size();
        return _frame->surface->bound().size().cast<isize>();
    }
};

Ui::Child cameraInfoDialog(State const& s) {
    auto infos = s.camera->info();
    Vec<Ui::Child> els;
    els.pushBack(Ui::labelMedium("name: {}\n", infos.name));
    els.pushBack(Ui::labelMedium("driver: {}\n", infos.driver));

    auto formats = s.camera->formats();
    for (auto& f : formats) {
        els.pushBack(Ui::labelMedium("{}x{} {}fps", f.resolution.width, f.resolution.height, f.framerate));
    }

    return Kr::dialogContent({
        Kr::dialogTitleBar("Camera Debug"s),
        Kr::dialogBody(els) | Ui::vscroll() | Ui::maxSize({Ui::UNCONSTRAINED, 256}),
    });
}

Ui::Child appContent(State const& s) {
    auto viewport =
        Ui::stack(
            makeRc<VideoSurface>(s.stream),
            Ui::canvas([guidelines = s.guidelines](Gfx::Canvas& g, Math::Vec2i size) {
                g.strokeStyle(Gfx::Stroke{
                    .fill = Gfx::WHITE.withOpacity(0.5),
                    .width = 1,
                });

                if (guidelines.crossLines) {
                    g.stroke(Math::Edgei{0, size}.cast<f64>());
                    g.stroke(Math::Edgei{0, size.y, size.x, 0}.cast<f64>());
                }

                if (guidelines.guideLines) {
                    // Rule of third guidelines
                    g.stroke(Math::Edgei{0, size.y / 3, size.x, size.y / 3}.cast<f64>());
                    g.stroke(Math::Edgei{0, size.y * 2 / 3, size.x, size.y * 2 / 3}.cast<f64>());

                    g.stroke(Math::Edgei{size.x / 3, 0, size.x / 3, size.y}.cast<f64>());
                    g.stroke(Math::Edgei{size.x * 2 / 3, 0, size.x * 2 / 3, size.y}.cast<f64>());
                }

                if (guidelines.reticule) {
                    // Focus reticle
                    g.stroke(Math::Ellipsef{
                        size.cast<f64>() / 2.,
                        (f64)min(size.y / 12, size.x / 12),
                    });
                }
            })
        ) |
        Ui::fit();

    auto topBar =
        Ui::vflow(
            8,
            Ui::button(
                [&](auto& n) {
                    Ui::showDialog(n, cameraInfoDialog(s));
                },
                Ui::ButtonStyle::regular().withForegroundFill(Gfx::WHITE).withRadii(999),
                Mdi::BUG
            ),
            Ui::button(
                Ui::SINK<>,
                Ui::ButtonStyle::regular().withForegroundFill(Gfx::WHITE).withRadii(999),
                Mdi::COG
            ),

            Ui::grow(NONE),

            Ui::button(
                Ui::SINK<>,
                Ui::ButtonStyle::regular().withForegroundFill(Gfx::WHITE).withRadii(999),
                Mdi::TUNE
            )
        ) |
        Ui::insets(24);

    auto bottomBar =
        Ui::vflow(
            Ui::button(
                Ui::SINK<>,
                Ui::ButtonStyle::regular().withRadii(999).withPadding(12),
                Ui::icon(Mdi::CAMERA_FLIP, 24)
            ) | Ui::center(),

            Ui::button(
                Model::bind<Capture>(),
                Ui::ButtonStyle::regular().withRadii(999).withPadding(16),
                Ui::icon(Mdi::CAMERA, 38)
            ) | Ui::center() |
                Ui::grow(),

            Ui::button(
                Model::bindIf<OpenLast>(s.lastImage.has()),
                Ui::ButtonStyle::regular().withRadii(999),
                (s.lastImage
                     ? Ui::image(s.lastImage.unwrap(), 999) | Ui::cover()
                     : Ui::empty()) |
                    Ui::pinSize(48)
            ) | Ui::center()
        ) |
        Ui::box({
            .padding = {32, 8},
            .backgroundFill = Ui::GRAY950.withOpacity(0.6),
        });

    return Ui::stack(
        viewport,
        Ui::hflow(topBar, Ui::grow(NONE), bottomBar)
    );
}

export Ui::Child app(Rc<Av::Camera> cam, Rc<Av::VideoStream> stream) {
    return Ui::reducer<Model>({cam, stream}, [](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::CAMERA,
            .title = "Camera"s,
            .body = [&] {
                return appContent(s);
            },
        });
    });
}

} // namespace Hideo::Camera
