export module Hideo.Images:editor;

import Mdi;
import Karm.Core;
import Karm.Ui;
import Karm.Kira;
import Karm.Gfx;
import Karm.Math;

import :model;
import :kernel;

using namespace Karm::Literals;

namespace Hideo::Images {

Ui::Child histogram(Hist const& hist) {
    return Ui::canvas([hist](Gfx::Canvas& g, Math::Vec2i size) {
        usize n = hist.len();
        if (n < 2 || size.x <= 1 || size.y <= 1)
            return;

        auto maxC = [&](usize c) {
            f64 m = 1e-9;
            for (usize i = 0; i < n; ++i)
                m = max(m, (f64)hist._buf[i]._els[c]);
            return m;
        };
        f64 xunit = (f64)(size.x - 1) / (n - 1);

        auto draw = [&](usize c, Gfx::Color stroke, Gfx::Color fill) {
            f64 m = maxC(c);
            g.beginPath();
            g.moveTo({0, (f64)size.y});
            for (usize i = 0; i < n; ++i) {
                f64 y = (f64)size.y * (1.0 - (hist._buf[i]._els[c] / m));
                g.lineTo({i * xunit, y});
            }
            g.stroke(Gfx::stroke(stroke).withWidth(1.0));

            g.lineTo({(f64)(size.x - 1), (f64)size.y});
            g.closePath();
            g.fill(fill.withOpacity(0.2));
        };

        draw(0, Gfx::RED, Gfx::RED800);
        draw(1, Gfx::GREEN, Gfx::GREEN800);
        draw(2, Gfx::BLUE, Gfx::BLUE800);
    });
}

Ui::Child editorHistogram(Editor const& editor) {
    auto graphSelect =
        Ui::hflow(
            6,
            Ui::button(
                Model::bind(Graph::HIST),
                Ui::ButtonStyle::text().withForegroundFill(Ui::GRAY50.withOpacity(editor.graph == Graph::HIST ? 1 : 0.5)),
                Ui::labelSmall("Hist")
            ),

            Kr::separator(),

            Ui::button(
                Model::bind(Graph::RGB),
                Ui::ButtonStyle::text().withForegroundFill(Ui::GRAY50.withOpacity(editor.graph == Graph::RGB ? 1 : 0.5)),
                Ui::labelSmall("RGB")
            ),

            Ui::button(
                Model::bind(Graph::RED),
                Ui::ButtonStyle::text().withForegroundFill(Ui::GRAY50.withOpacity(editor.graph == Graph::RED ? 1 : 0.5)),
                Ui::labelSmall("R")
            ),

            Ui::button(
                Model::bind(Graph::GREEN),
                Ui::ButtonStyle::text().withForegroundFill(Ui::GRAY50.withOpacity(editor.graph == Graph::GREEN ? 1 : 0.5)),
                Ui::labelSmall("G")
            ),
            Ui::button(
                Model::bind(Graph::BLUE),
                Ui::ButtonStyle::text().withForegroundFill(Ui::GRAY50.withOpacity(editor.graph == Graph::BLUE ? 1 : 0.5)),
                Ui::labelSmall("B")
            ),

            Ui::button(
                Model::bind(Graph::LUMA),
                Ui::ButtonStyle::text().withForegroundFill(Ui::GRAY50.withOpacity(editor.graph == Graph::LUMA ? 1 : 0.5)),
                Ui::labelSmall("Luma")
            )
        ) |
        Ui::insets(8);

    return Ui::vflow(
        graphSelect,
        Ui::stack(
            editor.graph == Graph::HIST ? histogram(editor.histogram) : Ui::image(editor.waveform),
            Ui::stack(
                Ui::button(
                    Model::bind<ToggleFlag>(KernelFlags::SHADOW_CLIP),
                    Ui::ButtonStyle::subtle().withForegroundFill(editor.flags.has(KernelFlags::SHADOW_CLIP) ? Ui::GRAY50 : Ui::GRAY600),
                    Ui::icon(Mdi::TRIANGLE, 12)
                ) | Ui::align(Math::Align::TOP_START),
                Ui::button(
                    Model::bind<ToggleFlag>(KernelFlags::HIGHLIGHT_CLIP),
                    Ui::ButtonStyle::subtle().withForegroundFill(editor.flags.has(KernelFlags::HIGHLIGHT_CLIP) ? Ui::GRAY50 : Ui::GRAY600),
                    Ui::icon(Mdi::TRIANGLE, 12)
                ) | Ui::align(Math::Align::TOP_END)
            ) | Ui::insets(8)
        ) |
            Ui::pinSize(192) | Ui::grow()
    );
}

Ui::Child editorPreview(Editor const& editor) {
    return Ui::image(editor.before ? editor.in : editor.out) |
           Ui::box({
               .borderWidth = 1,
               .borderFill = Ui::GRAY50.withOpacity(0.1),
           }) |
           Ui::insets(8) |
           Ui::fit();
}

Ui::Child editorPresets(Editor const& editor) {
    return Ui::vflow(
        Ui::vscroll(
            Ui::vflow(
                Kr::titleRow("Presets"s),

                Ui::button(Model::bind<Preset>(Presets::NEUTRAL), "Neutral"),
                Ui::button(Model::bind<Preset>(Presets::PUNCHY), "PUNCHY"),
                Ui::button(Model::bind<Preset>(Presets::FLAT), "FLAT"),

                Ui::empty(4),
                Ui::button(Model::bind<Preset>(Presets::WARM_SUNSET), "WARM_SUNSET"),
                Ui::button(Model::bind<Preset>(Presets::COOL_MIST), "COOL_MIST"),
                Ui::button(Model::bind<Preset>(Presets::RETRO_FADE), "RETRO_FADE"),

                Ui::empty(4),
                Ui::button(Model::bind<Preset>(Presets::HIGH_KEY), "HIGH_KEY"),
                Ui::button(Model::bind<Preset>(Presets::TEAL_ORANGE), "TEAL_ORANGE"),
                Ui::button(Model::bind<Preset>(Presets::MOODY_LOW_KEY), "MOODY_LOW_KEY"),

                Ui::empty(4),
                Ui::button(Model::bind<Preset>(Presets::KODACHROME), "KODACHROME"),
                Ui::button(Model::bind<Preset>(Presets::MATTE_FILM), "MATTE_FILM"),

                Ui::empty(4),
                Ui::button(Model::bind<Preset>(Presets::BLEACH_BYPASS), "BLEACH_BYPASS"),
                Ui::button(Model::bind<Preset>(Presets::SEPIA_FADE), "SEPIA_FADE"),
                Ui::button(Model::bind<Preset>(Presets::DREAMY_GLOW), "DREAMY_GLOW"),
                Ui::button(Model::bind<Preset>(Presets::CROSS_PROCESS), "CROSS_PROCESS"),
                Ui::button(Model::bind<Preset>(Presets::POP_ART_PUNCH), "POP_ART_PUNCH"),
                Ui::button(Model::bind<Preset>(Presets::PASTEL_DREAM), "PASTEL_DREAM"),
                Ui::button(Model::bind<Preset>(Presets::CYBERPUNK), "CYBERPUNK")
            )
        ) | Kr::scaffoldContent() |
            Ui::grow(),
        editorHistogram(editor) | Kr::scaffoldContent() | Kr::resizable(Kr::ResizeHandlePosition::TOP, {192}, NONE)
    );
}

Ui::Child adjustmentSlider(Editor const& editor, Adjustment adjustment, Kr::Slider::Origin origin) {
    Ui::ButtonStyle resetStyle = Ui::ButtonStyle::text().withMargin(0).withPadding(0);
    resetStyle.idleStyle.foregroundFill = Gfx::ALPHA;
    resetStyle.hoverStyle.backgroundFill = Ui::GRAY950;
    resetStyle.pressStyle.backgroundFill = Ui::GRAY950;
    resetStyle.pressStyle.backgroundFill = Ui::GRAY950;

    return Ui::vflow(
        4,
        Ui::hflow(
            Ui::labelSmall("{}", adjustment),
            Ui::grow(NONE),
            Ui::stack(
                Ui::labelSmall("{:.1}", editor.kernel.value(adjustment)) | Ui::end(),
                Ui::button(
                    [=](auto& n) {
                        Model::bubble(n, Reset{adjustment});
                    },
                    resetStyle, Ui::labelSmall("reset")
                )
            )
        ) | Ui::insets({0, 8}),
        Kr::slider(
            editor.kernel.sliderValue(adjustment),
            [=](auto& n, f32 value) {
                Model::bubble(n, Adjust{adjustment, value});
            },
            origin
        )
    );
}

Ui::Child adjustmentGroup(Str name, Ui::Children els) {
    return Ui::vflow(
        Ui::titleSmall(name) |
            Ui::insets({16, 12, 8, 12}),
        Ui::vflow(6, els) | Ui::insets({8, 4, 8, 4})
    );
}

Ui::Child editorProperties(Editor const& editor) {
    return Ui::vflow(
        Ui::vscroll(
            Ui::vflow(
                Ui::hflow(
                    4,
                    Ui::button(Model::bind<Auto>(), Mdi::AUTO_FIX),
                    Ui::button(Model::bind<Preset>(Presets::NEUTRAL), Mdi::RESTORE),
                    Ui::grow(NONE),
                    Ui::button(Model::bind<Toggle>(), editor.before ? Mdi::EYE_OFF : Mdi::EYE)
                ) | Ui::insets(6),
                adjustmentGroup(
                    "Light"s,
                    {
                        adjustmentSlider(editor, Adjustment::EXPOSURE, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::CONTRAST, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::HIGHLIGHTS, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::SHADOWS, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::BLACKS, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::WHITES, Kr::Slider::HALF),
                    }
                ),

                Kr::separator(),
                adjustmentGroup(
                    "Colors"s,
                    {
                        adjustmentSlider(editor, Adjustment::TEMPERATURE, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::TINT, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::VIBRANCE, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::SATURATION, Kr::Slider::HALF),
                    }
                ),

                Kr::separator(),
                adjustmentGroup(
                    "Vignet"s,
                    {
                        adjustmentSlider(editor, Adjustment::VIGNETTE_AMOUNT, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::VIGNETTE_FEATHER, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::VIGNETTE_MIDPOINT, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::VIGNETTE_ROUNDNESS, Kr::Slider::ZERO),
                    }
                ),

                Kr::separator(),
                adjustmentGroup(
                    "Lens Correction"s,
                    {
                        adjustmentSlider(editor, Adjustment::LENS_RADIAL1, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::LENS_RADIAL2, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::LENS_RADIAL3, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::LENS_TANGENT_X, Kr::Slider::HALF),
                        adjustmentSlider(editor, Adjustment::LENS_TANGENT_Y, Kr::Slider::ZERO),
                        adjustmentSlider(editor, Adjustment::LENS_CENTER_X, Kr::Slider::ZERO),
                        adjustmentSlider(editor, Adjustment::LENS_CENTER_Y, Kr::Slider::ZERO),
                        adjustmentSlider(editor, Adjustment::LENS_SCALE, Kr::Slider::ZERO),
                    }
                )
            )
        ) | Kr::scaffoldContent() |
            Ui::grow(),
        editorHistogram(editor) | Kr::scaffoldContent() | Kr::resizable(Kr::ResizeHandlePosition::TOP, {192}, NONE)
    );
}

Ui::Child editorSidepanel(State const& s) {
    Ui::Child panel =
        s.panel == Panel::ADJUST
            ? editorProperties(s.mode.unwrap<Editor>())
            : editorPresets(s.mode.unwrap<Editor>());

    return Ui::hflow(
        4,
        panel | Ui::grow(),
        Ui::vflow(
            4,
            Ui::button(Model::bind<Panel>(Panel::ADJUST), Ui::ButtonStyle::subtle().withForegroundFill(s.panel == Panel::ADJUST ? Ui::GRAY50 : Ui::GRAY500), Mdi::TUNE),
            Ui::button(Model::bind<Panel>(Panel::PRESETS), Ui::ButtonStyle::subtle().withForegroundFill(s.panel == Panel::PRESETS ? Ui::GRAY50 : Ui::GRAY500), Mdi::PALETTE_SWATCH_VARIANT)
        )
    );
}

Ui::Child editorApp(State const& s) {
    return Kr::scaffold({
        .icon = Mdi::IMAGE,
        .title = "Images"s,
        .startTools = [&] -> Ui::Children {
            return {
                Ui::button(
                    Model::bind<Cancel>(),
                    Ui::ButtonStyle::subtle(),
                    Mdi::UNDO
                ),

                Ui::button(
                    Model::bind<Cancel>(),
                    Ui::ButtonStyle::subtle(),
                    Mdi::REDO
                ),
            };
        },
        .endTools = [&] -> Ui::Children {
            return {
                Ui::button(
                    Model::bind<Cancel>(),
                    Ui::ButtonStyle::subtle(),
                    Mdi::CANCEL,
                    "Cancel"
                ),

                Ui::button(
                    Model::bind<Cancel>(),
                    Ui::ButtonStyle::primary(),
                    Mdi::FLOPPY,
                    "Save Changes"
                ),
            };
        },
        .body = [&] {
            return Ui::hflow(
                editorPreview(s.mode.unwrap<Editor>()) | Ui::bound() | Kr::scaffoldContent() | Ui::grow(),
                editorSidepanel(s) | Kr::resizable(Kr::ResizeHandlePosition::START, {320}, NONE)
            );
        },
    });
}

} // namespace Hideo::Images
