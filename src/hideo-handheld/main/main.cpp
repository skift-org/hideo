#include <karm/entry>

import Karm.App;
import Karm.Kira;
import Karm.Ui;
import Karm.Image;
import Karm.Font;
import Karm.Gfx;
import Karm.Math;
import Mdi;

using namespace Karm;
using namespace Karm::Literals;
using namespace Karm::Ref::Literals;

namespace Hideo::Handheld {

struct State {
    bool quickMenuVisible = false;
    bool inGame = false;
    bool inMenu = true;

    struct QuickMenuToggle {};

    struct LaunchGame {};

    struct QuitGame {};

    struct LaunchHome {};

    using Action = Union<QuickMenuToggle, LaunchGame, QuitGame, LaunchHome>;

    Ui::Task<Action> reduce(Action const& action) {
        return action.visit(
            [&](QuickMenuToggle) {
                quickMenuVisible = not quickMenuVisible;
                return NONE;
            },
            [&](LaunchGame) {
                inGame = true;
                inMenu = false;
                return NONE;
            },
            [&](QuitGame) {
                inGame = false;
                inMenu = true;
                quickMenuVisible = false;
                return NONE;
            },
            [&](LaunchHome) {
                inMenu = true;
                quickMenuVisible = false;
                return NONE;
            }
        );
    }
};

using Model = Ui::Model<State, State::Action>;

static Opt<Rc<Gfx::Fontface>> _inputFontface = NONE;

Ui::Child appMenu();

Rc<Gfx::Fontface> inputFontface() {
    if (not _inputFontface) {
        _inputFontface = Font::loadFontfaceOrFallback("bundle://hideo-handheld/fonts/BPreplayBold.ttf"_url).unwrap();
    }
    return *_inputFontface;
}

Gfx::ProseStyle inputMedium() {
    return {
        .font = Gfx::Font{
            inputFontface(),
            16,
        },
    };
}

Ui::Child buttonHint(Str button, Str description, Gfx::Color color) {
    return Ui::hflow(
        Ui::text(inputMedium(), button) | Ui::center() | Ui::minSize(26) |
            Ui::box({
                .margin = 4,
                .borderRadii = 999,
                .backgroundFill = color,
                .foregroundFill = color.luminance() > 0.6 ? Gfx::BLACK : Gfx::WHITE,
            }),
        Ui::labelLarge(description) |
            Ui::center() |
            Ui::insets({0, 12, 0, 2})
    );
}

Ui::Child buttonHints(Ui::Children children) {
    return Ui::hflow(
               6,
               std::move(children)
           ) |
           Ui::insets(8);
}

Ui::Child logo(Str text) {
    return Ui::titleMedium(text) | Ui::vcenter();
}

Ui::Child statusWidget(Gfx::Icon icon) {
    return Ui::hflow(
               12,
               Ui::icon(icon) | Ui::center() | Ui::minSize(22)
           ) |
           Ui::box({
               .padding = 8,
               .borderRadii = 999,
               .borderWidth = 1,
               .backgroundFill = Ui::GRAY900,
           });
}

Ui::Child statusWidget(Str description) {
    return Ui::labelLarge(description) | Ui::center() |
           Ui::box({
               .padding = {8, 16},
               .borderRadii = 999,
               .borderWidth = 1,
               .backgroundFill = Ui::GRAY900,
           });
}

Ui::Child statusWidget(Gfx::Icon icon, Str description) {
    return Ui::hflow(
               6,
               Ui::icon(icon) | Ui::center() | Ui::minSize(22), Ui::labelLarge(description) | Ui::center()
           ) |
           Ui::box({
               .padding = {8, 16, 8, 8},
               .borderRadii = 999,
               .borderWidth = 1,
               .backgroundFill = Ui::GRAY900,
           });
}

Ui::Child status() {
    return Ui::hflow(
               6,
               statusWidget(Mdi::BROADCAST),
               statusWidget(Mdi::BATTERY_50, "50%"),
               statusWidget("11:29")
           ) |
           Ui::insets({12, 0});
}

Ui::Child tileGameCover(Ref::Url image) {
    return Ui::image(image, 6);
}

Ui::Child tileAppCover(Gfx::Icon icon, Gfx::ColorRamp ramp) {
    return Ui::icon(icon, 96) |
           Ui::center() | Ui::bound() |
           Ui::box({
               .borderRadii = 6,
               .backgroundFill = ramp[6],
               .foregroundFill = ramp[1],
           });
}

Ui::Child tileContent(Ui::Child child) {
    return child |
           Ui::pinSize(192) |
           Ui::box({
               .borderRadii = 6,
               .shadowStyle = Gfx::BoxShadow::elevated(8),
           }) |
           Ui::align(Math::Align::BOTTOM | Math::Align::START);
}

Ui::Child tileButton(Ui::Send<> onPress, Ui::Child child) {
    return Ui::button(
        onPress,
        Ui::ButtonStyle{
            .hoverStyle = {
                .borderRadii = 6,
                .borderWidth = 2,
                .borderFill = Ui::ACCENT500,
            },
            .pressStyle = {
                .borderRadii = 6,
                .borderWidth = 2,
                .borderFill = Ui::ACCENT400,
            },
        },
        child
    );
}

Ui::Child appItem() {
    return tileButton(Ui::SINK<>, tileContent(tileGameCover("bundle://hideo-handheld/tiles/celeste.qoi"_url)));
}

Ui::Child appList() {
    return Ui::vflow(
               Ui::grow(NONE),
               Ui::headlineMedium("Celeste") |
                   Ui::insets({0, 48, 16}),
               Ui::hflow(
                   26,
                   tileButton(Model::bind<State::LaunchGame>(), tileContent(tileGameCover("bundle://hideo-handheld/tiles/celeste.qoi"_url))),
                   tileButton(Model::bind<State::LaunchGame>(), tileContent(tileGameCover("bundle://hideo-handheld/tiles/doom.qoi"_url))),
                   tileButton(Model::bind<State::LaunchGame>(), tileContent(tileGameCover("bundle://hideo-handheld/tiles/minicraft.qoi"_url))),
                   tileButton(Model::bind<State::LaunchGame>(), tileContent(tileGameCover("bundle://hideo-handheld/tiles/vvvvvv.qoi"_url))),
                   tileButton(Model::bind<State::LaunchGame>(), tileContent(tileGameCover("bundle://hideo-handheld/tiles/lego-island.qoi"_url))),
                   tileButton(Model::bind<State::LaunchGame>(), tileContent(tileAppCover(Mdi::FOLDER, Gfx::EMERALD_RAMP))),

                   Kr::separator(Gfx::GRAY500),
                   tileButton(Ui::SINK<>, tileContent(tileAppCover(Mdi::APPS, Gfx::ZINC_RAMP)))
               ) | Ui::insets({0, 48, 48}) |
                   Ui::hscroll()
           ) |
           Ui::grow();
}

Ui::Child runningAppItem() {
    return Ui::hflow(
        6,
        Ui::hflow(
            12,
            Ui::image("bundle://hideo-handheld/tiles/celeste.qoi"_url, 6) |
                Ui::box({
                    .borderRadii = 6,
                    .borderWidth = 1,
                    .borderFill = Ui::GRAY50.withOpacity(0.4),
                }) |
                Ui::pinSize(48) | Ui::vcenter(),
            Ui::vflow(
                Ui::titleMedium("Celeste"s),
                Ui::bodySmall("Version: v0.1"s)
            ) | Ui::grow()
        ) | Ui::grow(),
        Ui::button(
            [&](auto& n) {
                Ui::showDialog(n, appMenu() | Ui::center());
            },
            Mdi::DOTS_HORIZONTAL
        ) | Ui::vcenter(),
        Ui::button(
            Model::bind<State::QuitGame>(),
            Mdi::CLOSE
        ) | Ui::vcenter()
    );
}

Ui::Child quickSettings(State const& s) {
    Ui::Children items;

    items.pushBack(
        Ui::hflow(Ui::grow(NONE), status())
    );

    if (s.inGame) {
        items.pushBack(Kr::rowContent(Ui::button(Model::bind<State::LaunchHome>(), "Open home")));
        items.pushBack(Kr::titleRow("Running"s));
        items.pushBack(
            Ui::vflow(
                Kr::rowContent(runningAppItem()),
                Kr::separator(),
                Kr::rowContent(runningAppItem()),
                Kr::separator(),
                Kr::rowContent(runningAppItem())
            ) |
            Kr::card()
        );
    }

    items.pushBack(Kr::titleRow("Sound"s));
    items.pushBack(
        Ui::vflow(
            Kr::sliderRow(0.5, Ui::SINK<f64>, "Volume"s)
        ) |
        Kr::card()
    );

    items.pushBack(Kr::titleRow("Display"s));
    items.pushBack(
        Ui::vflow(
            Kr::sliderRow(0.5, Ui::SINK<f64>, "Brightness"s),
            Kr::toggleRow(true, Ui::SINK<bool>, "Night Mode"s)
        ) |
        Kr::card()
    );

    items.pushBack(Kr::titleRow("Wireless"s));
    items.pushBack(
        Ui::vflow(
            Kr::toggleRow(true, Ui::SINK<bool>, "NFC"s),
            Kr::toggleRow(true, Ui::SINK<bool>, "RaftShare™"s),
            Kr::rowContent(Ui::text(Ui::TextStyles::bodyMedium().withColor(Ui::GRAY400), "RaftShare™ keeps consoles together like otters in a raft, letting you share games and apps with the ones around you, wirelessly."))
        ) |
        Kr::card()
    );

    items.pushBack(Kr::titleRow("Appearance"s));
    items.pushBack(
        Ui::vflow(
            Kr::toggleRow(true, Ui::SINK<bool>, "Dark Mode"s)
        ) |
        Kr::card()
    );

    items.pushBack(Kr::titleRow("About"s));
    items.pushBack(
        Ui::vflow(
            Kr::rowContent(
                Ui::button(
                    [](auto& n) {
                        Ui::showDialog(n, Kr::aboutDialog("Ottercat"s));
                    },
                    "About Ottercat"
                )
            )
        ) |
        Kr::card()
    );

    return Ui::hflow(
               Ui::grow(NONE),
               Ui::hflow(
                   Kr::separator(),
                   Ui::vflow(
                       Ui::vflow(
                           8,
                           std::move(items)
                       ) |
                           Ui::insets({8, 16, 8, 16}) |
                           Ui::vscroll() | Ui::grow(),
                       Kr::separator(),
                       buttonHints({
                           Ui::grow(NONE),
                           buttonHint("A", "SELECT", Ui::GRAY50),
                           buttonHint("B", "BACK", Ui::GRAY50) | Ui::cond(s.quickMenuVisible),
                       })
                   ) |
                       Ui::box({.backgroundFill = Ui::GRAY950}) | Ui::grow()
               ) |
                   Ui::slideIn(Ui::SlideFrom::END) | Ui::pinSize(320)
           ) |
           Ui::box({.backgroundFill = Gfx::BLACK.withOpacity(0.4)});
}

Ui::Child topBar() {
    return Ui::hflow(
               6,
               Ui::grow(NONE),
               status()
           ) |
           Ui::insets({8, 16, 8, 16});
}

Ui::Child appMenu() {
    return Kr::contextMenuContent({
        Kr::dialogHeader({
            Ui::hflow(
                12,
                Ui::image("bundle://hideo-handheld/tiles/celeste.qoi"_url, 6) |
                    Ui::box({
                        .borderRadii = 6,
                        .borderWidth = 1,
                        .borderFill = Ui::GRAY50.withOpacity(0.4),
                    }) |
                    Ui::pinSize(64) | Ui::vcenter(),
                Ui::vflow(
                    Kr::dialogTitle("Celeste"s),
                    Kr::dialogDescription("Version: v0.1"s),
                    Kr::dialogDescription("Size: 10.5MiB"s)
                )
            ),
        }),
        Kr::separator(),
        Kr::contextMenuItem(Ui::closeDialog, Mdi::PLAY, "Open"s),
        Kr::contextMenuItem(Ui::closeDialog, Mdi::PLAYLIST_REMOVE, "Remove from recents"s),
        Kr::contextMenuItem(Ui::closeDialog, Mdi::DELETE_FOREVER, "Uninstall"s),
        Kr::separator(),
        Kr::contextMenuItem(Ui::closeDialog, Mdi::CANCEL, "Cancel"s),
    });
}

Ui::Child homeMenu(State const& s) {
    return Ui::vflow(
               topBar(),
               appList(),
               buttonHints({
                   buttonHint("  MENU  ", "SETTINGS", Ui::ACCENT500),
                   Ui::grow(NONE),
                   buttonHint("X", "QUIT GAME", Ui::GRAY50) | Ui::cond(s.inGame),
                   buttonHint("Y", "OPTIONS", Ui::GRAY50) | Ui::cond(not s.quickMenuVisible),
                   buttonHint("A", "LAUCN", Ui::GRAY50),
                   buttonHint("B", "BACK", Ui::GRAY50) | Ui::cond(s.quickMenuVisible),
               })
           ) |
           Ui::keyboardShortcut(App::Key::Y, [&](auto& n) {
               if (not s.quickMenuVisible)
                   Ui::showDialog(n, appMenu() | Ui::center());
           });
}

Ui::Child stacking(State const& s) {
    Ui::Children items;
    if (s.inGame) {
        items.pushBack(
            Ui::image(
                "bundle://hideo-handheld/gameplay/celeste.qoi"_url
            ) |
            Ui::cover()
        );
    } else {
        items.pushBack(
            Ui::image(
                "bundle://hideo-handheld/covers/celeste.qoi"_url
            ) |
            Ui::cover()
        );
    }

    if (s.inMenu) {
        items.pushBack(
            homeMenu(s) |
            Ui::backgroundFilter(Gfx::OverlayFilter{Ui::GRAY950.withOpacity(0.7)})
        );
    }

    if (s.quickMenuVisible) {
        items.pushBack(quickSettings(s));
    }

    return Ui::stack(std::move(items));
}

Ui::Child app() {
    return Ui::reducer<Model>({}, [](State const& s) {
        return stacking(s) |
               Ui::keyboardShortcut(App::Key::M, Model::bind<State::QuickMenuToggle>()) |
               Ui::dialogLayer();
    });
}

} // namespace Hideo::Handheld

Async::Task<> entryPointAsync(Sys::Env& env, Async::CancellationToken ct) {
    App::formFactor = App::FormFactor::MOBILE;

    co_return co_await Ui::runAsync(
        env,
        Hideo::Handheld::app() | Ui::pinSize({640, 480}),
        ct
    );
}
