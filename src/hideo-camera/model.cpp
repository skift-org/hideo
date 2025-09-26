module;

#include <karm-gfx/buffer.h>
#include <karm-gfx/cpu/canvas.h>

export module Hideo.Camera:model;

import Karm.Core;
import Karm.Av;
import Karm.Ui;
import Karm.Ref;
import Karm.Image;
import Karm.Sys;

using namespace Karm;

namespace Hideo::Camera {

struct Guidelines {
    bool crossLines = false;
    bool guideLines = true;
    bool reticule = false;
};

struct State {
    Rc<Av::Camera> camera;
    Rc<Av::VideoStream> stream;
    Opt<Rc<Gfx::Surface>> lastImage = NONE;
    Ref::Url lastImageUrl = ""_url;
    Guidelines guidelines = {};
};

Rc<Gfx::Surface> _generateThumbnail(Rc<Gfx::Surface> src) {
    auto thumb = Gfx::Surface::alloc(48);
    Gfx::CpuCanvas g;
    g.begin(thumb->mutPixels());
    g.blit(thumb->bound().fit(src->bound()), thumb->bound(), src->pixels());
    g.end();

    return thumb;
}

export struct Capture {};

export struct OpenLast {};

using Action = Union<
    Capture,
    OpenLast>;

Ui::Task<Action> reduce(State& s, Action a) {
    a.visit(Visitor{
        [&](Capture) {
            auto videoFrame = s.stream->next();
            if (videoFrame) {
                s.lastImage = _generateThumbnail(videoFrame->surface);
                auto now = Sys::now();
                auto dt = DateTime::fromInstant(now);
                auto filename = Io::format(
                    "Photo-{:04}{:02}{:02}_{:02}{:02}{:02}_{:05}.bmp",
                    dt.date.year.val(), dt.date.month.val() + 1, dt.date.day.val() + 1,
                    dt.time.hour, dt.time.minute, dt.time.second,
                    now.val() % 100000
                );
                s.lastImageUrl = "location://pictures/Camera"_url / filename;
                Image::save(videoFrame->surface->pixels(), s.lastImageUrl).unwrap("could not save picture");
            }
        },
        [&](OpenLast) {
            Sys::launch(
                {
                    .action = Ref::Uti::PUBLIC_PREVIEW,
                    .objects = {s.lastImageUrl},
                }
            )
                .unwrap("could not launch intent");
        },
    });

    return NONE;
}

export using Model = Ui::Model<State, Action, reduce>;

} // namespace Hideo::Camera