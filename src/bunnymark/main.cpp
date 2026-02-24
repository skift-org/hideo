#include <karm/entry>

import Karm.Core;
import Karm.App;
import Karm.Gfx;
import Karm.Sys;
import Karm.Math;
import Karm.Font;

using namespace Karm;

// --- Constants ---------------------------------------------------------------

static constexpr Math::Vec2i WINDOW_SIZE = {600, 480};
static constexpr int BATCH_SIZE = 5;

// --- Logic -------------------------------------------------------------------

struct Bunny {
    Math::Vec2f pos;
    Math::Vec2f vel;
    Gfx::Color color;
};

struct World {
    Vec<Bunny> bunnies;
    Math::Rand rand{123};

    void spawn(int count, Math::Vec2f origin) {
        for (int i = 0; i < count; i++) {
            bunnies.pushBack(
                {.pos = origin,
                 .vel = {rand.nextFloat(-5.0, 5.0), rand.nextFloat(-5.0, 5.0)},
                 .color = Gfx::hsvToRgb({rand.nextFloat(0, 360), 0.7, 0.9})}
            );
        }
    }

    void update(Math::Vec2i bound) {
        double gravity = 0.98;
        double maxX = bound.width;
        double maxY = bound.height;

        for (auto& b : bunnies) {
            b.pos = b.pos + b.vel;
            b.vel.y += gravity;

            if (b.pos.x < 0) {
                b.pos.x = 0;
                b.vel.x *= -1;
            } else if (b.pos.x > maxX) {
                b.pos.x = maxX;
                b.vel.x *= -1;
            }

            if (b.pos.y < 0) {
                b.pos.y = 0;
                b.vel.y = 0;
            } else if (b.pos.y > maxY) {
                b.pos.y = maxY;
                b.vel.y *= -0.9; // Bounce
                if (Math::abs(b.vel.y) < 0.5)
                    b.vel.y = 0; // Sleep
            }
        }
    }
};

// --- Resources ---------------------------------------------------------------

static Opt<Rc<Gfx::Fontface>> _regularFontface;

Rc<Gfx::Fontface> font() {
    if (!_regularFontface)
        _regularFontface = Font::loadFontfaceOrFallback(
                               "bundle://fonts-inter/fonts/Inter-Regular.ttf"_url
        )
                               .unwrap();
    return *_regularFontface;
}

// --- Handler -----------------------------------------------------------------

struct Handler : App::Handler {
    Rc<App::Window> win;
    World world;

    bool isMouseDown = false;
    Math::Vec2f mousePos = {0, 0};

    // FPS Counter
    int frames = 0;
    int fps = 0;
    Duration timeAccum = 0;
    SystemTime lastTime = Sys::now();

    Handler(Rc<App::Window> win) : win(win) {}

    void handle(App::WindowId, App::Event& e) override {
        if (auto ke = e.is<App::KeyboardEvent>()) {
            if (ke->type == App::KeyboardEvent::RELEASE and ke->key == App::Key::ESC)
                win->close();
        }

        if (auto me = e.is<App::MouseEvent>()) {
            mousePos = me->pos.cast<f64>();
            if (me->type == App::MouseEvent::PRESS) {
                isMouseDown = true;
                world.spawn(BATCH_SIZE, mousePos);
            } else if (me->type == App::MouseEvent::RELEASE) {
                isMouseDown = false;
            }
        }
        e.accept();
    }

    void paint(Gfx::Canvas& g, Math::Vec2i) {
        // --- FPS Logic ---
        auto now = Sys::now();
        timeAccum += (now - lastTime);
        lastTime = now;
        frames++;
        if (timeAccum >= Duration::fromSecs(1)) {
            fps = frames;
            frames = 0;
            timeAccum = 0;
        }

        // --- Render ---

        g.clear(Gfx::BLUE500);

        // 1. Bunnies (Vector)
        for (auto const& b : world.bunnies) {
            g.fillStyle(b.color);
            // Body
            g.fill(Math::Rectf{b.pos.x, b.pos.y, 26, 26}.cast<isize>(), 4.0);
            // Ears
            g.fill(Math::Rectf{b.pos.x + 2, b.pos.y - 10, 8, 12}.cast<isize>(), 2.0);
            g.fill(Math::Rectf{b.pos.x + 16, b.pos.y - 10, 8, 12}.cast<isize>(), 2.0);
        }

        // 2. GUI
        Gfx::ProseStyle style{.font = Gfx::Font{font(), 16}, .color = Gfx::WHITE};
        String info = Io::format("Bunnies: {} | FPS: {}", world.bunnies.len(), fps);

        g.push();
        g.translate({30, 30});
        Gfx::Prose prose(style, info);
        prose.layout(999_au);
        g.fill(prose);
        g.pop();
    }

    void update() override {
        // Spawn more while holding
        if (isMouseDown)
            world.spawn(BATCH_SIZE, mousePos);

        // Physics
        world.update(win->bound().size());

        // Paint
        auto surface = win->acquireSurface();
        Gfx::CpuCanvas g;
        g.begin(surface);
        paint(g, surface.bound().size());
        g.end();
        win->releaseSurface(surface.bound());
    }
};

// --- Entry Point -------------------------------------------------------------

Async::Task<> entryPointAsync(Sys::Context& ctx, Async::CancellationToken ct) {
    auto app = co_trya$(App::Application::createAsync(ctx, App::ApplicationProps::simple(), ct));
    auto win =
        co_trya$(app->createWindowAsync({.title = "BunnyMark"s, .size = WINDOW_SIZE}, ct));

    // Standard runAsync handles the loop
    co_return co_await app->runAsync(makeRc<Handler>(win), ct);
}
