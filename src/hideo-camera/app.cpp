export module Hideo.Camera;

import Mdi;
import Karm.Ui;
import Karm.Kira;
import Karm.Core;
import Karm.Av;
import Karm.App;
import Karm.Gfx;
import Karm.Math;

import :model;

using namespace Karm;
using namespace Karm::Literals;
using namespace Karm::Math::Literals;

namespace Hideo::Camera {

struct VideoSurface : Ui::View<VideoSurface> {
    Rc<Av::VideoStream> _stream;
    Opt<Av::VideoFrame> _frame;

    VideoSurface(Rc<Av::VideoStream> stream) : _stream(stream) {}

    void paint(Gfx::Canvas& g, Math::RectAu) override {
        if (_frame)
            g.blit(bound().cast<isize>(), _frame.expect().surface);
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

    Math::Vec2Au size(Math::Vec2Au size, Ui::Hint hint) override {
        if (not _frame)
            return {};
        if (hint == Ui::Hint::MIN)
            return _frame->surface->bound().cast<f64>().fit(Math::Rectf{size.cast<f64>()}).size().cast<Math::Au>();
        return _frame->surface->bound().size().cast<Math::Au>();
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
        Kr::dialogBody(els) | Ui::vscroll() | Ui::maxSize({Ui::UNCONSTRAINED, 256_au}),
    });
}

Ui::Child appContent(State const& s) {
    auto viewport =
        Ui::stack(
            makeRc<VideoSurface>(s.stream),
            Ui::onPaint([guidelines = s.guidelines](Gfx::Canvas& g, Math::Vec2Au sizeAu) {
                auto size = sizeAu.cast<isize>();
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
            8_au,
            Ui::button(
                Some([&](auto& n) {
                    Ui::showDialog(n, cameraInfoDialog(s));
                }),
                Ui::ButtonStyle::regular().withForegroundFill(Gfx::WHITE).withRadii(999),
                Mdi::BUG
            ),
            Ui::button(
                Some(Ui::SINK<>),
                Ui::ButtonStyle::regular().withForegroundFill(Gfx::WHITE).withRadii(999),
                Mdi::COG
            ),

            Ui::grow(NONE),

            Ui::button(
                Some(Ui::SINK<>),
                Ui::ButtonStyle::regular().withForegroundFill(Gfx::WHITE).withRadii(999),
                Mdi::TUNE
            )
        );

    auto bottomBar =
        Ui::vflow(
            Ui::button(
                Some(Ui::SINK<>),
                Ui::ButtonStyle::regular().withRadii(999).withPadding(12_au),
                Ui::icon(Mdi::CAMERA_FLIP, 24_au)
            ) | Ui::center(),

            Ui::button(
                Some(Model::bind<Capture>()),
                Ui::ButtonStyle::regular().withRadii(999).withPadding(16_au),
                Ui::icon(Mdi::CAMERA, 38_au)
            ) | Ui::center() |
                Ui::grow(),

            Ui::button(
                Model::bindIf<OpenLast>(s.lastImage.has()),
                Ui::ButtonStyle::regular().withRadii(999),
                (s.lastImage
                     ? Ui::image(s.lastImage.expect(), Some(999)) | Ui::cover()
                     : Ui::empty()) |
                    Ui::pinSize(48_au)
            ) | Ui::center()
        );

    return Ui::hflow(
        topBar,
        viewport | Ui::bound() | Kr::scaffoldContent() | Ui::insets({0_au, 8_au}) | Ui::grow(),
        bottomBar
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
