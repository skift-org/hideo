export module Hideo.Oobe;

import Mdi;
import Karm.Core;
import Karm.Ui;
import Karm.Kira;
import Karm.App;
import Karm.Ref;
import Karm.Gfx;
import Karm.Math;

using namespace Karm;
using namespace Karm::Literals;
using namespace Karm::Ref::Literals;

namespace Hideo::Oobe {

enum struct Step {
    WELCOME,
    NETWORK,
    DEVICE,
    ACCOUNT,
    ACCOUNT_LOCAL,
    ACCOUNT_ONLINE,
    ACCOUNT_REGISTER,
    APPS,
    FINISH,
    FINISHING,
};

struct State {
    Step step = Step::WELCOME;
};

using Action = Union<Step>;

Ui::Task<Action> reduce(State& s, Action a) {
    a.visit(
        [&](Step step) {
            s.step = step;
        }
    );

    return NONE;
}

using Model = Ui::Model<State, Action, reduce>;

Ui::Child welcomeStep(State const&) {
    return Ui::vflow(
        12,
        Ui::labelSmall(Ui::GRAY500, "Step 1 of 6"),
        Ui::titleLarge("Welcome!"),
        Ui::bodyMedium("Let’s get your device ready with a few simple steps."),
        Ui::vflow(
            Kr::rowContent(
                Some(Ui::icon(Mdi::WIFI)),
                "Connect to a network for updates and online services."s,
                NONE, NONE
            ),
            Kr::rowContent(
                Some(Ui::icon(Mdi::ACCOUNT_CIRCLE)),
                "Set up your account to sync and personalize your experience."s,
                NONE, NONE

            ),
            Kr::rowContent(
                Some(Ui::icon(Mdi::TEXT_BOX)),
                "Install a few apps so you can start using your device right away."s,
                NONE, NONE
            )
        ),
        Ui::bodySmall(Ui::GRAY500, "You can change these settings later in Settings.") | Ui::center(),
        Ui::grow(NONE),
        Ui::hflow(
            Ui::button(
                Some([](auto& n) {
                    Ui::showDialog(
                        n,
                        Kr::dialogContent({
                            Kr::dialogHeader({
                                Kr::dialogTitle("Skip setup?"s),
                                Kr::dialogDescription(
                                    "If you skip now, your device will use the default username “user” "
                                    "and install the standard set of apps."s
                                ),
                            }),
                            Kr::dialogFooter({
                                Ui::grow(NONE),
                                Kr::dialogCancel(),
                                Kr::dialogAction(
                                    Some(Model::bind(Step::FINISH)),
                                    "Skip Setup"s
                                ),
                            }),
                        })
                    );
                }),
                Ui::ButtonStyle::subtle(),
                "Skip"
            ),
            Ui::grow(NONE),
            Ui::button(
                Some(Model::bind(Step::NETWORK)),
                Ui::ButtonStyle::regular(),
                "Get started"
            )
        )
    );
}

Ui::Child networkStep(State const&) {
    return Ui::vflow(
        12,
        Ui::labelSmall(Ui::GRAY500, "Step 2 of 6"),
        Ui::titleLarge("Get connected"),
        Ui::bodyMedium("Choose a Wi-Fi network to connect this device to the internet."),
        Kr::card(
            Kr::rowContent(Some(Ui::icon(Mdi::ETHERNET)), "Wired Connection"s, Some("Connected"), NONE),
            Kr::separator(),
            Ui::vflow(
                Kr::rowContent(Some(Ui::icon(Mdi::WIFI_STRENGTH_4_LOCK)), "Home Wi-Fi"s, Some("Connected"), NONE),
                Kr::rowContent(Some(Ui::icon(Mdi::WIFI_STRENGTH_3_LOCK)), "Office"s, NONE, NONE),
                Kr::rowContent(Some(Ui::icon(Mdi::WIFI_STRENGTH_1)), "Guest network"s, NONE, NONE),
                Kr::rowContent(Some(Ui::icon(Mdi::WIFI_STRENGTH_1_LOCK)), "FBI Van"s, NONE, NONE),
                Kr::rowContent(Some(Ui::icon(Mdi::WIFI_STRENGTH_1_LOCK)), "Smart Lightbulb"s, NONE, NONE),
                Kr::rowContent(Some(Ui::icon(Mdi::WIFI_STRENGTH_1_LOCK)), "Bob's Iphone"s, NONE, NONE)
            ) |
                Ui::vhscroll() | Ui::pinSize({Ui::UNCONSTRAINED, 260})
        ),
        Ui::grow(NONE),
        Ui::hflow(
            Ui::button(
                Some(Model::bind(Step::WELCOME)),
                Ui::ButtonStyle::text(),
                "Back"
            ),
            Ui::grow(NONE),
            Ui::button(
                Some(Model::bind(Step::ACCOUNT)),
                Ui::ButtonStyle::regular(),
                "Continue"
            )
        )
    );
}

Ui::Child accountStep(State const&) {
    return Ui::vflow(
        12,
        Ui::labelSmall(Ui::GRAY500, "Step 3 of 6"),
        Ui::titleLarge("Set up your account"),
        Ui::bodyMedium("Choose how you want to sign in and sync data on this device."),
        Ui::vflow(
            12,
            Kr::buttonRow(
                Some(Model::bind(Step::ACCOUNT_LOCAL)),
                Mdi::ACCOUNT_CIRCLE_OUTLINE,
                "Use a local account"s,
                Some("Keep everything on this device only. No syncing."s),
                "Setup…"s
            ) | Kr::card(),

            Ui::vflow(
                Kr::buttonRow(
                    Some(Model::bind(Step::ACCOUNT_ONLINE)),
                    Mdi::ACCOUNT,
                    "Sign in to an existing account"s,
                    Some("Use your existing profile to restore settings, and apps"s),
                    "Login…"s
                ),
                Kr::separator(),
                Kr::buttonRow(
                    Some(Model::bind(Step::ACCOUNT_REGISTER)),
                    Mdi::ACCOUNT_PLUS,
                    "Create a new account"s,
                    Some("Set up a fresh profile."s),
                    "Register…"s
                )
            ) | Kr::card()
        ) |
            Ui::grow(),
        Ui::hflow(
            Ui::button(
                Some(Model::bind(Step::NETWORK)),
                Ui::ButtonStyle::text(),
                "Back"
            )
        )
    );
}

Ui::Child accountLocalStep(State const&) {
    return Ui::vflow(
        12,
        Ui::labelSmall(Ui::GRAY500, "Step 3 of 6"),
        Ui::titleLarge("Set up local account"),
        Ui::bodyMedium("Create an account that only exists on this device. Your data stays local and is not synced online."),
        Ui::vflow(
            6,
            Kr::input(Mdi::ACCOUNT, "Username"s, ""s, Ui::SINK<String>),
            Kr::input(Mdi::LOCK, "Password"s, ""s, Ui::SINK<String>),
            Kr::input(Mdi::LOCK, "Confirm password"s, ""s, Ui::SINK<String>)
        ),
        Ui::grow(NONE),
        Ui::hflow(
            Ui::button(
                Some(Model::bind(Step::ACCOUNT)),
                Ui::ButtonStyle::text(),
                "Back"
            ),
            Ui::grow(NONE),
            Ui::button(
                Some(Model::bind(Step::DEVICE)),
                Ui::ButtonStyle::regular(),
                "Create"
            )
        )
    );
}

Ui::Child accountOnlineStep(State const&) {
    return Ui::vflow(
        12,
        Ui::labelSmall(Ui::GRAY500, "Step 3 of 6"),
        Ui::titleLarge("Sign in to your account"),
        Ui::bodyMedium(
            "Use your online account to sync apps, settings and data across your devices. "
            "You’ll sign in through an instance you choose."
        ),
        Kr::rowContent(
            Some(Ui::icon(Mdi::SERVER_NETWORK)),
            "Instance"s,
            Some("skift.cute.engineering"s),
            Some(
                Ui::button(
                    Some(Ui::SINK<>),
                    "Change…"
                )
            )
        ) | Kr::card(),
        Ui::vflow(
            6,
            Kr::input(
                Mdi::ACCOUNT,
                "Email"s,
                ""s,
                Ui::SINK<String>
            ),
            Kr::input(Mdi::LOCK, "Password"s, ""s, Ui::SINK<String>),
            Ui::empty(),
            Ui::button(
                Some(Ui::SINK<>),
                Ui::ButtonStyle::text(),
                "Forgot password…"s
            )
        ),
        Ui::grow(NONE),
        Ui::hflow(
            Ui::button(
                Some(Model::bind(Step::ACCOUNT)),
                Ui::ButtonStyle::text(),
                "Back"
            ),
            Ui::grow(NONE),
            Kr::indeterminedProgress() | Ui::insets({0, 12}),
            Ui::button(
                Some(Model::bind(Step::DEVICE)),
                Ui::ButtonStyle::regular(),
                "Login"
            )
        )
    );
}

Ui::Child accountRegisterStep(State const&) {
    return Ui::vflow(
        12,
        Ui::labelSmall(Ui::GRAY500, "Step 3 of 6"),
        Ui::titleLarge("Create a new account"),
        Ui::bodyMedium(
            "Create a new online account on an instance so you can sync your data across devices."
        ),
        Kr::rowContent(
            Some(Ui::icon(Mdi::SERVER_NETWORK)),
            "Instance"s,
            Some("skift.cute.engineering"s),
            Some(
                Ui::button(
                    Some(Ui::SINK<>),
                    "Change…"
                )
            )
        ) | Kr::card(),
        Ui::vflow(
            6,
            Kr::input(Mdi::ACCOUNT, "Username"s, ""s, Ui::SINK<String>),
            Kr::input(Mdi::ACCOUNT, "Email"s, ""s, Ui::SINK<String>),
            Kr::input(Mdi::LOCK, "Password"s, ""s, Ui::SINK<String>),
            Kr::input(Mdi::LOCK, "Confirm password"s, ""s, Ui::SINK<String>)
        ),
        Ui::grow(NONE),
        Ui::hflow(
            Ui::button(
                Some(Model::bind(Step::ACCOUNT)),
                Ui::ButtonStyle::text(),
                "Back"
            ),
            Ui::grow(NONE),
            Kr::indeterminedProgress() | Ui::insets({0, 12}),
            Ui::button(
                Some(Model::bind(Step::DEVICE)),
                Ui::ButtonStyle::regular(),
                "Register"
            )
        )
    );
}

Ui::Child deviceStep(State const&) {
    return Ui::vflow(
        12,
        Ui::labelSmall(
            Ui::GRAY500,
            "Step 4 of 6"
        ),
        Ui::titleLarge("Name this device"),
        Ui::bodyMedium("Pick a name to help you recognize this device across apps and services."),
        Ui::empty(8),
        Kr::card(
            Kr::buttonRow(
                Some(Ui::SINK<>),
                "Device name"s,
                Some("{user}'s Laptop"),
                "Change…"s
            ),
            Kr::separator(),
            Kr::toggleRow(
                true,
                Ui::SINK<bool>,
                "Back up this device"s
            )
        ),
        Ui::grow(NONE),
        Ui::hflow(
            Ui::button(
                Some(Model::bind(Step::ACCOUNT)),
                Ui::ButtonStyle::text(),
                "Back"
            ),
            Ui::grow(NONE),
            Ui::button(
                Some(Model::bind(Step::APPS)),
                Ui::ButtonStyle::regular(),
                "Continue"
            )
        )
    );
}

Ui::Child appsStep(State const&) {
    return Ui::vflow(
        12,
        Ui::labelSmall(Ui::GRAY500, "Step 5 of 6"),
        Ui::titleLarge("Get some apps"),
        Ui::bodyMedium("Install a few essentials now. You can always add more later."),
        Ui::empty(8),
        Kr::card(
            Kr::checkboxRow(true, Ui::SINK<bool>, "Install recommended apps"s),
            Kr::separator(),
            Kr::treeRow(
                NONE,
                "Customise…"s,
                NONE,
                Ui::Slot{
                    [&] -> Ui::Child {
                        return Ui::vflow(
                                   Kr::rowContent(
                                       NONE,
                                       "Core Apps"s,
                                       Some("Files, Console, Apps, Settings"s),
                                       Some(Ui::labelMedium("Required"))
                                   ),
                                   Kr::separator(),
                                   Kr::rowContent(
                                       NONE,
                                       "Media"s,
                                       Some("Camera, Videos, Images"s),
                                       Some(Kr::checkbox(true, Ui::SINK<bool>))
                                   ),
                                   Kr::separator(),
                                   Kr::rowContent(
                                       NONE,
                                       "Productivity"s,
                                       Some("Contacts, Calendar, Messages, Notes, Clock"s),
                                       Some(Kr::checkbox(true, Ui::SINK<bool>))
                                   ),
                                   Kr::separator(),
                                   Kr::rowContent(
                                       NONE,
                                       "Web & Info"s,
                                       Some("Browser, Weather, Maps"s),
                                       Some(
                                           Kr::checkbox(true, Ui::SINK<bool>)
                                       )
                                   ),
                                   Kr::separator(),
                                   Kr::rowContent(
                                       NONE,
                                       "Office Suite"s,
                                       Some("Spreadsheet, Slides, Writer"s),
                                       Some(Kr::checkbox(true, Ui::SINK<bool>))
                                   ),
                                   Kr::separator(),
                                   Kr::rowContent(
                                       NONE,
                                       "Games"s,
                                       Some("Fun preinstalled titles"s),
                                       Some(Kr::checkbox(true, Ui::SINK<bool>))
                                   ),
                                   Kr::separator(),
                                   Kr::rowContent(
                                       NONE,
                                       "Design Tools"s,
                                       Some("Fonts, Canvas"s),
                                       Some(Kr::checkbox(true, Ui::SINK<bool>))
                                   ),
                                   Kr::separator(),
                                   Kr::rowContent(
                                       NONE,
                                       "Developer Tools"s,
                                       Some("Code, Zoo"s),
                                       Some(Kr::checkbox(true, Ui::SINK<bool>))
                                   )
                               ) |
                               Ui::vscroll() | Ui::pinSize({Ui::UNCONSTRAINED, 180});
                    },
                }
            )
        ),
        Ui::grow(NONE),
        Ui::hflow(
            Ui::button(Some(Model::bind(Step::DEVICE)), Ui::ButtonStyle::text(), "Back"),
            Ui::grow(NONE),
            Ui::button(Some(Model::bind(Step::FINISH)), Ui::ButtonStyle::regular(), "Review")
        )
    );
}

Ui::Child finishStep(State const&) {
    return Ui::vflow(
        12,
        Ui::labelSmall(Ui::GRAY500, "Step 6 of 6"),
        Ui::titleLarge("Review your choices"),
        Ui::bodyMedium("Here’s a summary of your setup. Adjust anything you like before finishing."),
        Kr::rowContent(
            Some(Ui::icon(Mdi::CHECK, Some(Gfx::GREEN500))),
            "Network"s,
            Some("Connected to “Home Wi-Fi”"s),
            NONE
        ),
        Kr::rowContent(
            Some(Ui::icon(Mdi::CHECK, Some(Gfx::GREEN500))),
            "Account"s,
            Some("Signed in as me@smnx.sh"s),
            NONE
        ),
        Kr::rowContent(
            Some(Ui::icon(Mdi::CHECK, Some(Gfx::GREEN500))),
            "Backup"s,
            Some("Backup enabled for “{deviceName}”"s),
            NONE
        ),
        Kr::rowContent(
            Some(Ui::icon(Mdi::CHECK, Some(Gfx::GREEN500))),
            "Applications"s,
            Some("Suggested apps will be installed"s),
            NONE
        ),

        Ui::grow(NONE),
        Ui::hflow(
            Ui::button(
                Some(Model::bind(Step::APPS)),
                Ui::ButtonStyle::text(),
                "Back"
            ),
            Ui::grow(NONE),
            Ui::button(
                Some(Model::bind(Step::FINISHING)),
                Ui::ButtonStyle::primary(),
                "Finish setup"
            )
        )
    );
}

Ui::Child finishingStep(State const&) {
    return Ui::vflow(
        12,
        Ui::labelSmall(Ui::GRAY500, "Almost there"),
        Ui::titleLarge("Finalizing setup"),
        Ui::bodyMedium("Saving your settings and getting everything ready to use."),
        Ui::vflow(
            16,
            Math::Align::CENTER,
            Kr::indeterminedProgress(32),
            Ui::labelMedium("Applying configurations…"),
            Ui::button(Some(Model::bind(Step::FINISH)), Ui::ButtonStyle::regular(), "Cancel"s)
        ) | Ui::center() |
            Ui::grow()
    );
}

Ui::Child stepContent(State const& s) {
    switch (s.step) {
    case Step::WELCOME:
        return welcomeStep(s);
    case Step::NETWORK:
        return networkStep(s);
    case Step::ACCOUNT:
        return accountStep(s);
    case Step::ACCOUNT_LOCAL:
        return accountLocalStep(s);
    case Step::ACCOUNT_ONLINE:
        return accountOnlineStep(s);
    case Step::ACCOUNT_REGISTER:
        return accountRegisterStep(s);
    case Step::DEVICE:
        return deviceStep(s);
    case Step::APPS:
        return appsStep(s);
    case Step::FINISH:
        return finishStep(s);
    case Step::FINISHING:
        return finishingStep(s);
    default:
        unreachable();
    }
}

Ui::Child stepContainer(Ui::Child child) {
    return child |
           Ui::insets(16) |
           Ui::box({
               .borderRadii = 8,
               .borderWidth = 1,
               .borderFill = Some(Ui::GRAY800),
               .backgroundFill = Some(Ui::GRAY950),
               .shadowStyle = Some(Gfx::BoxShadow::elevated(16)),
           }) |
           Ui::pinSize({480, 520}) |
           Ui::center();
}

Ui::Child appContent(State const& s) {
    if (App::formFactor == App::FormFactor::MOBILE) {
        return stepContent(s) |
               Ui::insets(16);
    }

    return Ui::stack(
        Ui::image("bundle://hideo-shell/wallpapers/abstract.qoi"_url) |
            Ui::cover() |
            Ui::grow(),
        stepContainer(stepContent(s))
    );
}

export Ui::Child app() {
    return Ui::reducer<Model>(
        {Step::WELCOME},
        [](State const& s) {
            return appContent(s) |
                   Ui::pinSize(
                       App::formFactor == App::FormFactor::MOBILE
                           ? Math::Vec2i{411, 731}
                           : Math::Vec2i{1280, 720}
                   ) |
                   Ui::dialogLayer();
            ;
        }
    );
}

} // namespace Hideo::Oobe
