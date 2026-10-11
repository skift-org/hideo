export module Hideo.Sysmon:app;

import Mdi;
import Karm.Ui;
import Karm.Kira;
import Karm.Core;
import Karm.Gfx;
import Karm.Math;
import Karm.Chart;

import :model;

using namespace Karm::Literals;
using namespace Karm::Math::Literals;

namespace Hideo::Resources {

// MARK: Sidebar ---------------------------------------------------------------

Ui::Child graph(Gfx::Color color, Chart::Serie serie) {
    Chart::Chart c = {
        .data = {
            .series = {
                serie
            },
        },
        .theme = {.palette = {color}},
        .style = {
            .min = Some(0.0),
            .max = Some(1.0),
        },
    };
    return Kr::chart(std::move(c), {72_au, 0_au}) |
           Ui::box({
               .borderRadii = 4,
               .borderWidth = 1,
               .borderFill = Some(color),
               .backgroundFill = Some(color.withOpacity(0.25)),
               .overflow = Ui::BoxOverflow::HIDDEN,
           });
}

Ui::Child sidebarItem(bool selected, Ui::Send<> onPress, Chart::Serie serie, Gfx::Color color, String title, String description) {
    return Kr::sidenavItem(
        selected,
        Some(onPress),
        Ui::hflow(
            4_au,
            graph(color, serie),
            Ui::vflow(
                Ui::titleSmall(title),
                Ui::labelMedium(description)
            ) |
                Ui::insets(4_au) |
                Ui::minSize({112_au, Ui::UNCONSTRAINED})
        )
    );
}

Ui::Child sidebar(State const& s) {
    return Kr::sidenavContent({
        Kr::sidenavItem(
            s.page == Page::PROCESSES,
            Some(Model::bind(Page::PROCESSES)),
            Mdi::FILE_TREE,
            "Processes"s
        ),

        sidebarItem(
            s.page == Page::PROCESSORS,
            Model::bind(Page::PROCESSORS),
            s.hartData[0],
            Gfx::BLUE,
            "Processors"s,
            "AMD Ryzen 7 7700X"s
        ),
        sidebarItem(
            s.page == Page::MEMORY,
            Model::bind(Page::MEMORY),
            s.memoryData,
            Gfx::CYAN,
            "Memory"s,
            "32GB DDR5 3200MHz"s
        ),
    });
}

// MARK: Processes -------------------------------------------------------------

Ui::Child processListItem(State const& s, Rc<Sys::ProcessStat> const& process) {
    return Ui::button(
        Some(Model::bind<SelectProcess>(process->id)),
        s.selected == process->id ? Ui::ButtonStyle::regular() : Ui::ButtonStyle::subtle(),
        Mdi::COG,
        process->name
    );
}

Ui::Child processListContent(State const& s) {
    return Ui::vflow(
               8_au,
               iter(s.processes) |
                   Select([&](auto& p) {
                       return processListItem(s, p);
                   }) |
                   Collect<Ui::Children>()
           ) |
           Ui::insets(16_au) |
           Ui::vscroll();
}

Ui::Child processPage(State const& s) {
    return Ui::vflow(
        4_au,
        processListContent(s) | Kr::scaffoldContent() | Ui::grow(),
        Ui::hflow(
            4_au,
            Math::Align::END | Math::Align::VFILL,
            Ui::button(Model::bindIf<DetailProcess>(s.selectedProcess().has()), Ui::ButtonStyle::subtle(), Mdi::INFORMATION_OUTLINE),
            Ui::button(Model::bindIf<KillProcess>(s.selectedProcess().has()), Ui::ButtonStyle::destructive(), "End Task")
        ) | Ui::end()
    );
}

// MARK: Processor -------------------------------------------------------------

Ui::Child processorPage(State const& s) {
    return Ui::vflow(
               iter(s.hartData) | Selecti([](Chart::Serie const& s, usize i) {
                   return Ui::vflow(
                              4_au,
                              Ui::titleMedium("HART {}", i + 1),
                              graph(Gfx::BLUE, s) | Ui::minSize({Ui::UNCONSTRAINED, 240_au}),
                              Ui::labelMedium("{:.1}%", last(s.values) * 100)

                          ) |
                          Ui::insets(16_au) |
                          Kr::scaffoldContent();
               }) |
               Collect<Ui::Children>()
           ) |
           Ui::vscroll();
}

// MARK: Memory ----------------------------------------------------------------

Ui::Child memoryPage(State const& s) {
    return Ui::vflow(
               Ui::vflow(
                   4_au,
                   Ui::titleMedium("Memory"),
                   graph(Gfx::CYAN, s.memoryData) | Ui::grow(),
                   Ui::labelMedium("{} / {}", DataSize{s.memoryInfo.physicalUsed()}, DataSize{s.memoryInfo.physicalTotal})
               ) | Ui::insets(16_au) |
                   Ui::grow(),
               Kr::separator(),
               Ui::vflow(
                   4_au,
                   Ui::titleMedium("Swap"),
                   graph(Gfx::CYAN, s.swapData) | Ui::grow(),
                   Ui::labelMedium("{} / {}", DataSize{s.memoryInfo.swapUsed()}, DataSize{s.memoryInfo.swapTotal})
               ) | Ui::insets(16_au) |
                   Ui::grow()
           ) |
           Kr::scaffoldContent();
}

// MARK: App -------------------------------------------------------------------

export Ui::Child app() {
    return Ui::reducer<Model>({}, [](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::VIEW_DASHBOARD,
            .title = "Resources"s,
            .sidebar = Some([&] {
                return sidebar(s) | Kr::resizable(Kr::ResizeHandlePosition::END);
            }),
            .body = [s] {
                switch (s.page) {
                case Page::PROCESSES:
                    return processPage(s);
                case Page::PROCESSORS:
                    return processorPage(s);
                case Page::MEMORY:
                    return memoryPage(s);
                }
            },
        });
    });
}

} // namespace Hideo::Resources
