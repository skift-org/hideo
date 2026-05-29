#include <karm/entry>

import Mdi;
import Karm.Ui;
import Karm.Kira;
import Karm.Font;
import Karm.Gfx;
import Karm.Math;
import Karm.App;
import Karm.Vte;

using namespace Karm;
using namespace Karm::Literals;

namespace Hideo::Console {

Ui::Child colorBubble(Gfx::Color color) {
    return Ui::empty(16) |
           Ui::box({
               .borderRadii = 4,
               .borderWidth = 1,
               .borderFill = Ui::GRAY50.withOpacity(0.2),
               .backgroundFill = color,
           });
}

Ui::Child colorSchemeOption(Vte::ColorScheme scheme) {
    Ui::Children grays;
    Ui::Children colors;

    for (usize i = 0; i < 8; i++) {
        grays.pushBack(colorBubble(scheme.colors[i]));
        colors.pushBack(colorBubble(scheme.colors[i + 8]));
    }
    return Ui::hflow(
               6,
               Ui::labelMedium(scheme.name) | Ui::center() | Ui::insets({0, 64, 0, 0}),
               Ui::grow(NONE),
               Ui::labelLarge(scheme.colors[7], "AaBbCc") |
                   Ui::box({
                       .padding = 6,
                       .borderRadii = 4,
                       .backgroundFill = scheme.colors[0],
                   }),
               Ui::vflow(
                   2,
                   Ui::hflow(2, std::move(grays)),
                   Ui::hflow(2, std::move(colors))
               )
           ) |
           Ui::insets({6, 16});
}

Ui::Child settingsDialog() {
    return Kr::dialogContent({
        Kr::dialogTitleBar("Settings"s),
        Kr::dialogBody({
            Kr::titleRow("Font"s),
            Ui::vflow(
                Kr::numberRow(16.0, Ui::SINK<f64>, 100, "Font Size"s)
            ) | Kr::card(),

            Kr::titleRow("Color Scheme"s),
            Ui::vflow(
                colorSchemeOption(Vte::ColorScheme::light()),
                colorSchemeOption(Vte::ColorScheme::dark()) | Ui::button(Ui::SINK<>, Ui::ButtonStyle::regular()),
                Kr::separator(),
                colorSchemeOption(Vte::ColorScheme::solarized()),
                colorSchemeOption(Vte::ColorScheme::dracula()),
                colorSchemeOption(Vte::ColorScheme::nord())
            ) | Kr::card(),

            Kr::titleRow("Terminal"s),
            Ui::vflow(
                Kira::toggleRow(true, Ui::SINK<bool>, "Scrollback"s),
                Kira::numberRow(1000.0, Ui::SINK<f64>, 100, "Scrollback Size"s)
            ) | Kr::card(),
        }) | Ui::vscroll() |
            Ui::grow(),
    });
}

struct State {
    Rc<Vte::Terminal> terminal;
    Rc<Sys::Pty> pty;
};

using Action = Union<
    Bytes, App::KeyboardEvent>;

static Ui::Task<Action> reduce(State& s, Action a) {
    a.visit(
        [&](Bytes b) {
            s.terminal->write(b);
        },
        [&](App::KeyboardEvent const& e) {
            Io::TextEncoder<> enc{*s.pty};
            if (e.type == App::KeyboardEvent::PRESS) {
                if (e.key == App::Key::ENTER) {
                    (void)enc.writeRune('\n');
                } else if (e.key == App::Key::BKSPC) {
                    (void)enc.writeRune('\b');
                } else {
                    (void)enc.writeRune(e.rune);
                }
            }
        }
    );
    return NONE;
}

using Model = Ui::Model<State, Action, reduce>;

Ui::Child contextMenu() {
    return Kr::contextMenuContent({
        Kr::contextMenuItem(Ui::SINK<>, Mdi::CONTENT_COPY, "Copy"),
        Kr::contextMenuItem(NONE, Mdi::CONTENT_PASTE, "Paste"),
        Kr::separator(),
        Kr::contextMenuItem(Ui::SINK<>, Mdi::SELECT_ALL, "Select All"),
        Kr::separator(),
        Kr::contextMenuItem(
            [](auto& n) {
                Ui::showDialog(n, settingsDialog());
            },
            Mdi::COG, "Settings"
        ),
    });
}

Ui::Child app(Rc<Vte::Terminal> terminal, Rc<Sys::Pty> pty) {
    return Ui::reducer<Model>(
        State{terminal, pty},
        [](State const& s) {
            return Kr::scaffold({
                .icon = Mdi::CONSOLE_LINE,
                .title = "Console"s,
                .body = [&] {
                    return Vte::viewport(s.terminal, Model::map<App::KeyboardEvent>()) |
                           Ui::insets(6) |
                           Kr::contextMenu([] {
                               return contextMenu();
                           }) |
                           Kr::scaffoldContent();
                },
            });
        }
    );
}

} // namespace Hideo::Console

Async::Task<> _handleAsync(Rc<Sys::Pty> pty, Ui::Child app, Async::CancellationToken ct) {
    Array<u8, Io::DEFAULT_BUFFER_SIZE> buf;

    while (true) {
        co_try$(ct.errorIfCanceled());
        auto read = co_trya$(pty->readAsync(buf, ct));
        Hideo::Console::Model::event(*app, sub(buf, 0, read));
    }
}

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    auto terminal = makeRc<Vte::Terminal>(Vte::Theme{});

    Sys::Command command{
        .exe = "luna"s,
        .env = {}
    };
    auto [process, p] = co_try$(command.spawnPty());
    auto pty = makeRc<Sys::Pty>(std::move(p));

    auto app = Hideo::Console::app(terminal, pty);
    Async::detach(_handleAsync(pty, app, ct));
    co_return co_await Ui::runAsync(env, app, ct);
}
