#include <karm-sys/entry.h>

import Mdi;
import Karm.Ui;
import Karm.Kira;
import Karm.Font;
import Karm.Gfx;
import Karm.Math;
import Karm.App;
import Karm.Vte;

using namespace Karm;

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

Ui::Child app() {
    auto terminal = makeRc<Vte::Terminal>(Vte::Theme{});
    // terminal->_attrs.fg = Gfx::GRAY200;
    // terminal->write("~");
    // terminal->_attrs.fg = Gfx::BLUE;
    // terminal->write(" λ ");
    // terminal->_attrs.fg = Gfx::GRAY200;
    // terminal->write("ls -la\n");
    //
    // terminal->write("-rw-r--r-- 1 smnx smnx    3298 Aug  1 13:51 readme.md\n");
    // terminal->write("-rw-r--r-- 1 smnx smnx    3298 Aug  1 13:51 readme.md\n");
    // terminal->write("-rw-r--r-- 1 smnx smnx    3298 Aug  1 13:51 readme.md\n");
    // terminal->write("-rw-r--r-- 1 smnx smnx    3298 Aug  1 13:51 readme.md\n");
    // terminal->write("-rw-r--r-- 1 smnx smnx    3298 Aug  1 13:51 readme.md\n");
    // terminal->write("-rw-r--r-- 1 smnx smnx    3298 Aug  1 13:51 readme.md\n");
    // terminal->write("-rw-r--r-- 1 smnx smnx    3298 Aug  1 13:51 readme.md\n");
    // terminal->write("-rw-r--r-- 1 smnx smnx    3298 Aug  1 13:51 readme.md\n");
    // terminal->write("-rw-r--r-- 1 smnx smnx    3298 Aug  1 13:51 readme.md\n");
    // terminal->write("-rw-r--r-- 1 smnx smnx    3298 Aug  1 13:51 readme.md\n");
    // terminal->_attrs.fg = Gfx::GRAY200;
    terminal->write("~");
    terminal->_attrs.fg = Gfx::BLUE;
    terminal->write(" λ ");

    return Kr::scaffold({
        .icon = Mdi::CONSOLE_LINE,
        .title = "Console"s,
        .body = [terminal] {
            return Vte::viewport(terminal) | Ui::insets(6) |
                   Kr::contextMenu([] {
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
                   });
        },
    });
}

} // namespace Hideo::Console

Async::Task<> entryPointAsync(Sys::Context& ctx, Async::CancellationToken ct) {
    co_return co_await Ui::runAsync(ctx, Hideo::Console::app(), ct);
}
