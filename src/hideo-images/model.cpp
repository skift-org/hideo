module;

#include <karm-gfx/cpu/canvas.h>
#include <karm-gfx/filters.h>

export module Hideo.Images:model;

import Karm.Image;
import Karm.Ui;
import :kernel;

using namespace Karm;

namespace Hideo::Images {

export using Hist = Array<Math::Vec3f, 64>;

enum struct Graph {
    HIST,
    RGB,
    LUMA,
    RED,
    GREEN,
    BLUE
};

void computeHistogram(Hist& hist, Gfx::Pixels pixels) {
    f64 max = 0;
    hist = {};

    for (auto y = 0; y < pixels.width(); ++y) {
        for (auto x = 0; x < pixels.height(); ++x) {
            auto pixel = pixels.load({x, y});

            hist[pixel.red / 4].x += 1;
            hist[pixel.green / 4].y += 1;
            hist[pixel.blue / 4].z += 1;

            max = ::max(max, hist[pixel.red / 4].x, hist[pixel.green / 4].y, hist[pixel.blue / 4].z);
        }
    }

    for (auto& h : hist) {
        h = h / max;
    }
}

void computeWaveform(Graph graph, Gfx::Pixels in, Gfx::MutPixels out) {
    out.clear(Gfx::ALPHA);

    for (auto y = 0; y < in.width(); ++y) {
        for (auto xx = 0; xx < out.width(); ++xx) {
            isize x = (xx / (f64)out.width()) * in.width();
            auto c = in.loadUnsafe({x, y});

            if (graph == Graph::RED or graph == Graph::RGB)
                out.blend({xx, 255 - c.red}, Gfx::Color{255, 0, 0}.withOpacity(0.1));

            if (graph == Graph::GREEN or graph == Graph::RGB)
                out.blend({xx, 255 - c.blue}, Gfx::Color{0, 255, 0}.withOpacity(0.1));

            if (graph == Graph::BLUE or graph == Graph::RGB)
                out.blend({xx, 255 - c.green}, Gfx::Color{0, 0, 255}.withOpacity(0.1));

            if (graph == Graph::LUMA or graph == Graph::RGB)
                out.blend({xx, (isize)(255 - (255 * c.luminance()))}, Gfx::Color{255, 255, 255}.withOpacity(0.1));
        }
    }
}

// MARK: Editor ----------------------------------------------------------------

static Math::Vec2i fitThumbSize(Math::Vec2i src, Math::Vec2i box = {512, 512}) {
    f32 scale = min(
        1.0f,
        min(box.width / static_cast<f32>(src.width), box.height / static_cast<f32>(src.height))
    );
    return {
        max(1, Math::floori(src.width * scale)),
        max(1, Math::floori(src.height * scale))
    };
}

static Rc<Gfx::Surface> generateThumbnail(Rc<Gfx::Surface> original) {
    auto thumb = Gfx::Surface::alloc(fitThumbSize(original->bound().size()));
    Gfx::CpuCanvas g;
    g.begin(*thumb);
    g.blit(original->bound(), thumb->bound(), *original);
    g.end();
    return thumb;
}

enum struct Panel {
    ADJUST,
    PRESETS,
};

struct Editor {
    Rc<Gfx::Surface> original;
    Rc<Gfx::Surface> in;
    Rc<Gfx::Surface> out;
    Rc<Gfx::Surface> waveform;
    Kernel kernel{};
    Flags<KernelFlags> flags{};
    Hist histogram{};
    Graph graph = Graph::HIST;
    bool before = false;

    void refresh() {
        if (before) {
            computeHistogram(histogram, *in);
            computeWaveform(graph, *in, *waveform);
        } else {
            kernel.applyWithLens(*in, *out, flags);
            computeHistogram(histogram, *out);
            computeWaveform(graph, *out, *waveform);
        }
    }

    static Editor create(Rc<Gfx::Surface> original) {
        auto in = generateThumbnail(original);
        auto out = Gfx::Surface::alloc(in->bound().size());
        Editor ed{
            original,
            in,
            out,
            Gfx::Surface::alloc({512, 255}),
        };
        ed.refresh();
        return ed;
    }
};

// MARK: Viewer ----------------------------------------------------------------

struct Viewer {
    Rc<Gfx::Surface> image;
};

// MARK: State -----------------------------------------------------------------

export struct State {
    Union<Viewer, Editor> mode;
    Panel panel = Panel::ADJUST;
};

struct Edit {};

struct Save {};

struct Cancel {};

struct Adjust {
    Adjustment adjustment;
    f32 value;
};

struct Auto {};

struct Reset {
    Adjustment adjustment;
};

struct Preset {
    Kernel preset;
};

struct Toggle {};

struct ToggleFlag {
    KernelFlags flag;
};

export using Action = Union<
    Edit,
    Save,
    Cancel,
    Adjust,
    Auto,
    Reset,
    Toggle,
    Preset,
    Panel,
    Graph,
    ToggleFlag>;

Ui::Task<Action> reduce(State& s, Action a) {
    s.mode.visit(Visitor{
        [&](Editor& m) {
            a.visit(Visitor{
                [&](Save) {
                    s.mode = Viewer{m.original};
                },
                [&](Cancel) {
                    s.mode = Viewer{m.original};
                },
                [&](Adjust adjust) {
                    m.kernel.change(adjust.adjustment, adjust.value);
                    m.refresh();
                },
                [&](Auto) {
                    m.kernel = autoAdjust(analyze(*m.in));
                    m.refresh();
                },
                [&](Reset reset) {
                    m.kernel.reset(reset.adjustment);
                    m.refresh();
                },
                [&](Preset preset) {
                    m.kernel = preset.preset;
                    m.refresh();
                },
                [&](Panel panel) {
                    s.panel = panel;
                },
                [&](Graph graph) {
                    m.graph = graph;
                    m.refresh();
                },
                [&](Toggle) {
                    m.before = not m.before;
                    m.refresh();
                },
                [&](ToggleFlag c) {
                    m.flags.toggle(c.flag);
                    m.refresh();
                },
                [](auto&) {
                    notImplemented();
                },
            });
        },
        [&](Viewer& m) {
            a.visit(Visitor{
                [&](Edit) {
                    s.mode = Editor::create(m.image);
                },
                [](auto&) {
                    notImplemented();
                },
            });
        },
    });

    return NONE;
}

export using Model = Ui::Model<State, Action, reduce>;

} // namespace Hideo::Images
