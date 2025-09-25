module;

#include <karm-gfx/buffer.h>
#include <karm-gfx/cpu/canvas.h>

export module Hideo.Camera:model;

import Karm.Core;
import Karm.Av;
import Karm.Ui;
import Karm.Ref;
import Karm.Image;

using namespace Karm;

namespace Hideo::Camera {

struct Guidelines {
    bool crossLines = false;
    bool guideLines = true;
    bool reticule = false;
};

struct State {
    Rc<Av::VideoStream> stream;
    Opt<Rc<Gfx::Surface>> lastImage = NONE;
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

using Action = Union<
    Capture>;

Ui::Task<Action> reduce(State& s, Action a) {
    a.visit(Visitor{
        [&](Capture) {
            auto videoFrame = s.stream->next();
            if (videoFrame) {
                s.lastImage = _generateThumbnail(videoFrame->surface);
                Image::save(videoFrame->surface->pixels(), "file:./output.bmp"_url).unwrap("could not save picture");
            }
        },
    });

    return NONE;
}

export using Model = Ui::Model<State, Action, reduce>;

} // namespace Hideo::Camera