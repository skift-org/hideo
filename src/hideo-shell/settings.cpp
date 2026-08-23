export module Hideo.Shell:settings;

import Mdi;
import Karm.Kira;
import Karm.Ui;
import Karm.Gfx;

import :model;
import :power;
import :notifications;

using namespace Karm;

namespace Hideo::Shell {

struct QuickSettingProps {
    Gfx::Icon icon;
    Opt<String> name;
    bool state = false;
    Opt<Ui::Send<>> press = NONE;
    Opt<Ui::Send<>> more = NONE;
};

Ui::Child quickSetting(QuickSettingProps props) {
    auto style = props.state
                     ? Ui::ButtonStyle::primary()
                     : Ui::ButtonStyle::secondary();

    auto primaryStyle = props.more
                            ? style.withRadii({4, 0, 0, 4})
                            : style;

    auto secondaryStyle = style.withRadii({0, 4, 4, 0});

    auto primary = [&] {
        if (props.name)
            return Ui::button(std::move(props.press), primaryStyle, props.icon, *props.name);
        return Ui::button(std::move(props.press), primaryStyle, props.icon);
    }();

    if (props.more) {
        return Ui::hflow(
            primary | Ui::grow(),
            Kr::separator(),
            Ui::button(std::move(props.more), secondaryStyle, Mdi::CHEVRON_RIGHT)
        );
    }

    return primary;
}

Ui::Child dateAndTime(State const& state) {
    auto [date, time] = state.dateTime;

    return Ui::vflow(
               Ui::labelLarge("{:02}:{:02}", time.hour, time.minute),
               Ui::labelMedium("Fri, Jul 28")
           ) |
           Ui::center() |
           Ui::bound() |
           Ui::button(
               Some(Ui::SINK<>),
               Ui::ButtonStyle::subtle()
                   .withPadding({0, 12})
           );
}

Ui::Child quickheader(State const& state) {
    return Ui::hflow(
        dateAndTime(state),
        Ui::grow(NONE),
        Ui::button(
            Some(Model::bind<ToggleSysPanel>()),
            Ui::ButtonStyle::secondary(),
            state.isSysPanelColapsed
                ? Mdi::CHEVRON_DOWN
                : Mdi::CHEVRON_UP
        )
    );
}

Ui::Child quickTools(State const&) {
    return Ui::hflow(
        8,
        Ui::button(
            Some(Model::bind<Lock>()),
            Ui::ButtonStyle::secondary(),
            Mdi::LOCK
        ),
        Ui::button(
            Some([](auto& n) {
                Model::bubble(n, ActivatePanel{Panel::NIL});
                Ui::showDialog(n, powerDialog());
            }),
            Ui::ButtonStyle::secondary(), Mdi::POWER
        ),
        Ui::grow(NONE),
        Ui::button(
            Some([](auto& n) {
                Model::bubble(n, ActivatePanel{Panel::NIL});
                Ui::showDialog(n, Kr::aboutDialog("Shell"s));
            }),
            Ui::ButtonStyle::secondary(),
            Mdi::INFORMATION
        ),
        Ui::button(
            Some(Model::bind<ToggleSysPanel>()),
            Ui::ButtonStyle::secondary(),
            Mdi::COG
        )
    );
}

Ui::Child colapsedQuickSettings(State const&) {
    auto settings = Ui::grid(
        {
            .rows = Ui::GridUnit::fixed(46).repeated(2),
            .columns = Ui::GridUnit::grow().repeated(2),
            .gaps = 8,
        },
        quickSetting({
            .icon = Mdi::SWAP_VERTICAL,
            .name = Some("Cellular Data"),
            .press = Some(Ui::SINK<>),
            .more = Some(Ui::SINK<>),
        }),
        quickSetting({
            .icon = Mdi::WIFI_STRENGTH_4,
            .name = Some("Wi-Fi"),
            .press = Some(Ui::SINK<>),
            .more = Some(Ui::SINK<>),
        }),
        quickSetting({
            .icon = Mdi::BLUETOOTH,
            .name = Some("Bluetooth"),
            .press = Some(Ui::SINK<>),
            .more = Some(Ui::SINK<>),
        }),
        quickSetting({
            .icon = Mdi::FLASHLIGHT,
            .name = Some("Flashlight"),
            .press = Some(Ui::SINK<>),
        })
    );

    return settings;
}

Gfx::Icon _iconForBrightnessValue(f64 value) {
    if (value < 0.01) {
        return Mdi::BRIGHTNESS_4;
    } else if (value < 0.33) {
        return Mdi::BRIGHTNESS_5;
    } else if (value < 0.66) {
        return Mdi::BRIGHTNESS_6;
    } else {
        return Mdi::BRIGHTNESS_7;
    }
}

Gfx::Icon _iconForVolumeValue(f64 value) {
    if (value < 0.01) {
        return Mdi::VOLUME_MUTE;
    } else if (value < 0.33) {
        return Mdi::VOLUME_LOW;
    } else if (value < 0.66) {
        return Mdi::VOLUME_MEDIUM;
    } else {
        return Mdi::VOLUME_HIGH;
    }
}

Ui::Child expendedQuickSettings(State const& s) {
    auto settings = Ui::grid(
        {
            .rows = Ui::GridUnit::fixed(46).repeated(4),
            .columns = Ui::GridUnit::grow().repeated(2),
            .gaps = 8,
        },
        quickSetting({
            .icon = Mdi::SWAP_VERTICAL,
            .name = Some("Cellular Data"),
            .press = Some(Ui::SINK<>),
            .more = Some(Ui::SINK<>),
        }),
        quickSetting({
            .icon = Mdi::WIFI_STRENGTH_4,
            .name = Some("Wi-Fi"),
            .press = Some(Ui::SINK<>),
            .more = Some(Ui::SINK<>),
        }),
        quickSetting({
            .icon = Mdi::BLUETOOTH,
            .name = Some("Bluetooth"),
            .press = Some(Ui::SINK<>),
            .more = Some(Ui::SINK<>),
        }),
        quickSetting({
            .icon = Mdi::FLASHLIGHT,
            .name = Some("Flashlight"),
            .press = Some(Ui::SINK<>),
        }),
        quickSetting({
            .icon = Mdi::MAP_MARKER_OUTLINE,
            .name = Some("Location"),
            .press = Some(Ui::SINK<>),
        }),
        quickSetting({
            .icon = App::formFactor == App::FormFactor::MOBILE ? Mdi::LAPTOP : Mdi::CELLPHONE,
            .name = Some("Tablet Mode"),
            .state = App::formFactor == App::FormFactor::MOBILE,
            .press = Some(Model::bind<ToggleTablet>()),
        }),
        quickSetting({
            .icon = Mdi::CIRCLE_HALF_FULL,
            .name = Some("Dark Mode"),
            .press = Some(Ui::SINK<>),
        }),
        quickSetting({
            .icon = Mdi::BRIGHTNESS_2,
            .name = Some("Night Light"),
            .state = s.nightLight,
            .press = Some(Model::bind<ToggleNightLight>()),
        })
    );

    return Ui::vflow(
        8,
        Kr::slider(
            s.brightness,
            [](auto& n, auto value) {
                Model::bubble(n, ChangeBrightness{value});
            },
            _iconForBrightnessValue(s.brightness)
        ),
        Kr::slider(
            s.volume,
            [](auto& n, auto value) {
                Model::bubble(n, ChangeVolume{value});
            },
            _iconForVolumeValue(s.volume)
        ),
        settings | Ui::grow(), quickTools(s)
    );
}

export Ui::Child quickSettings(State const& state) {
    if (state.isSysPanelColapsed)
        return colapsedQuickSettings(state);
    return expendedQuickSettings(state);
}

export Ui::Child sysFlyout(State const& state) {
    Ui::Children body;
    body.pushBack(quickheader(state));
    if (state.isSysPanelColapsed) {
        body.pushBack(colapsedQuickSettings(state));
        body.pushBack(Ui::labelMedium("Notifications") | Ui::insets({6, 0, 0, 12}));
        body.pushBack(notifications(state) | Ui::grow());
    } else {
        body.pushBack(expendedQuickSettings(state) | Ui::grow());
    }
    body.pushBack(Kr::dragHandle());

    return Ui::vflow(8, body) |
           Ui::box({
               .margin = {8, 8, 32, 8},
               .padding = {12, 12, 0, 12},
               .borderRadii = 8,
               .borderWidth = 1,
               .borderFill = Some(Ui::GRAY800),
               .backgroundFill = Some(Ui::GRAY950),
           }) |
           Ui::bound() |
           Ui::dismisable(
               Model::bind<ActivatePanel>(Panel::NIL),
               Ui::DismisDir::TOP,
               0.3
           ) |
           Ui::slideIn(Ui::SlideFrom::TOP);
}

} // namespace Hideo::Shell
