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

export auto pageScaffold() {
    return [](auto inner) {
        return inner |
               Ui::insets({32_au, 8_au, 32_au, 8_au}) |
               Ui::maxSize({460_au, Ui::UNCONSTRAINED}) |
               Ui::grow() |
               Ui::hcenter() |
               Ui::vscroll() |
               Ui::grow();
    };
}

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

Ui::Child sidebarItem(bool selected, Ui::Send<> onPress, Chart::Serie serie, Gfx::Color color, Gfx::Icon icon, String title) {
    return Kr::sidenavItemContent(
        selected,
        Some(onPress),
        Ui::vflow(
            8_au,
            Kr::sidenavItemIconAndText(icon, title),
            graph(color, serie) |
                Ui::minSize({Ui::UNCONSTRAINED, 72_au})
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
            s.cpuData,
            Gfx::BLUE,
            Mdi::MEMORY,
            "Processor"s
        ),
        sidebarItem(
            s.page == Page::MEMORY,
            Model::bind(Page::MEMORY),
            s.memoryData,
            Gfx::CYAN,
            Mdi::CHIP,
            "Memory"s
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
           Ui::insets(8_au) |
           Kr::card();
}

Ui::Child processPage(State const& s) {
    return Ui::vflow(
        processListContent(s) | pageScaffold(),
        Kr::separator(),
        Ui::hflow(
            4_au, Math::Align::END | Math::Align::VFILL,
            Ui::button(
                Model::bindIf<DetailProcess>(s.selectedProcess().has()),
                Ui::ButtonStyle::subtle(),
                Mdi::INFORMATION_OUTLINE
            ),
            Ui::button(
                Model::bindIf<KillProcess>(s.selectedProcess().has()),
                Ui::ButtonStyle::destructive(),
                "End Task"
            )
        ) | Ui::end() |
            Ui::insets(8_au)
    );
}

// MARK: Processor -------------------------------------------------------------

Ui::Child processorPage(State const& s) {
    return Ui::vflow(
               16_au,
               Ui::vflow(
                   8_au,
                   Ui::titleSmall("Processor"),
                   graph(Gfx::CYAN, s.cpuData) | Ui::minSize({Ui::UNCONSTRAINED, 160_au}),
                   Ui::labelMedium("{:.1}%", s.cpuData.lastValue().unwrapOr(0.0) * 100)
               ) |
                   Ui::insets(16_au) |
                   Kr::card(),

               Kr::treeRow(
                   NONE,
                   "Hardware Threads"s,
                   NONE,
                   Ui::Slot{
                       [&] -> Ui::Child {
                           return Ui::grid(
                                      Ui::GridStyle{
                                          Ui::GridUnit::fixed(160_au).repeated((s.hartData.len() + 1) / 2),
                                          Ui::GridUnit::grow().repeated(2),
                                          8_au,
                                          Math::Flow::LEFT_TO_RIGHT,
                                          Math::Align::FILL,
                                      },
                                      iter(s.hartData) | Select([](Chart::Serie const& s) {
                                          return Ui::vflow(
                                              4_au,
                                              graph(Gfx::BLUE, s) | Ui::grow(),
                                              Ui::labelMedium("{:.1}%", s.lastValue().unwrapOr(0.0) * 100)

                                          );
                                      }) |
                                          Collect<Ui::Children>()
                                  ) |
                                  Ui::insets(16_au);
                       }
                   }
               ) |
                   Kr::card()
           ) |
           pageScaffold();
}

// MARK: Memory ----------------------------------------------------------------

Ui::Child memoryPage(State const& s) {
    return Ui::vflow(
               Ui::vflow(
                   8_au,
                   Ui::titleSmall("Memory"),
                   graph(Gfx::CYAN, s.memoryData) | Ui::minSize({Ui::UNCONSTRAINED, 160_au}),
                   Ui::labelMedium("{} / {}", DataSize{s.memoryInfo.physicalUsed()}, DataSize{s.memoryInfo.physicalTotal})
               ) | Ui::insets(16_au),
               Kr::separator(),
               Ui::vflow(
                   8_au,
                   Ui::titleSmall("Swap"),
                   graph(Gfx::GREEN, s.swapData) | Ui::minSize({Ui::UNCONSTRAINED, 160_au}),
                   Ui::labelMedium("{} / {}", DataSize{s.memoryInfo.swapUsed()}, DataSize{s.memoryInfo.swapTotal})
               ) |
                   Ui::insets(16_au)
           ) |
           Kr::card() |
           pageScaffold();
}

// MARK: Page ------------------------------------------------------------------

Ui::Child pageContent(State const& s) {
    switch (s.page) {
    case Page::PROCESSES:
        return processPage(s);
    case Page::PROCESSORS:
        return processorPage(s);
    case Page::MEMORY:
        return memoryPage(s);
    }
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
                return pageContent(s) |
                       Kr::scaffoldContent();
            },
        });
    });
}

} // namespace Hideo::Resources
