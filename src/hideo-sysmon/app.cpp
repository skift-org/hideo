export module Hideo.Sysmon:app;

import Mdi;
import Karm.Ui;
import Karm.Kira;
import Karm.Core;
import Karm.Gfx;
import Karm.Math;

import :model;

using namespace Karm::Literals;
using namespace Karm::Math::Literals;

namespace Hideo::Sysmon {

// MARK: Sidebar ---------------------------------------------------------------

Ui::Child graph(Gfx::Color color) {
    return Ui::empty({72_au, 0_au}) |
           Ui::box({
               .borderRadii = 4,
               .borderWidth = 1,
               .borderFill = Some(color),
               .backgroundFill = Some(color.withOpacity(0.25)),
           });
}

Ui::Child sidebarItem(bool selected, Ui::Send<> onPress, Gfx::Color color, String title, String description) {
    return Kr::sidenavItem(
        selected,
        Some(onPress),
        Ui::hflow(
            4_au,
            graph(color),
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
            Gfx::BLUE,
            "Processors"s,
            "AMD Ryzen 7 7700X"s
        ),
        sidebarItem(
            s.page == Page::MEMORY,
            Model::bind(Page::MEMORY),
            Gfx::CYAN,
            "Memory"s,
            "32GB DDR5 3200MHz"s
        ),
        sidebarItem(
            s.page == Page::DRIVES,
            Model::bind(Page::DRIVES),
            Gfx::GREEN,
            "Drives"s,
            "1TB NVMe SSD"s
        ),
        sidebarItem(
            s.page == Page::NETWORK,
            Model::bind(Page::NETWORK),
            Gfx::PINK,
            "Network"s,
            "10GbE NIC"s
        ),
        sidebarItem(
            s.page == Page::GRAPHICS,
            Model::bind(Page::GRAPHICS),
            Gfx::ORANGE,
            "Graphics"s,
            "AMD Radeon RX 7900 XT"s
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

// MARK: App -------------------------------------------------------------------

export Ui::Child app() {
    return Ui::reducer<Model>({}, [](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::VIEW_DASHBOARD,
            .title = "System Monitor"s,
            .sidebar = Some([&] {
                return sidebar(s) | Kr::resizable(Kr::ResizeHandlePosition::END);
            }),
            .body = [s] {
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
            },
        });
    });
}

} // namespace Hideo::Sysmon
