export module Hideo.Shell:notifications;

import Mdi;
import Karm.Ui;
import Karm.Math;
import Karm.Kira;
import Karm.Core;

import :model;

using namespace Karm;
using namespace Karm::Math::Literals;

namespace Hideo::Shell {

Ui::Child noti(Noti const& noti, usize i) {
    return Ui::vflow(
               8_au,
               Ui::hflow(
                   4_au,
                   Ui::icon(Mdi::INFORMATION, 12_au) | Ui::box({.foregroundFill = Ui::ACCENT400}),
                   Ui::text(Ui::TextStyles::labelMedium().withColor(Ui::GRAY400), "Hideo Shell")
               ),
               Ui::vflow(
                   6_au,
                   Ui::labelLarge(noti.title),
                   Ui::labelMedium(noti.body)
               )
           ) |
           Ui::box({
               .padding = 12_au,
               .borderRadii = 4,
               .backgroundFill = Some(Ui::GRAY900),
           }) |
           Ui::dragRegion() |
           Ui::dismisable(
               Model::bind<DimisNoti>(i),
               Ui::DismisDir::HORIZONTAL,
               0.3
           ) |
           Ui::key(noti.id);
}

export Ui::Child notifications(State const& state) {
    if (not state.noti.len()) {
        return Ui::text(
                   Ui::TextStyles::labelMedium()
                       .withColor(Ui::GRAY400),
                   "No notifications"
               ) |
               Ui::center();
    }

    return Ui::vflow(
               8_au,
               iter(state.noti) |
                   Selecti(noti) |
                   Collect<Ui::Children>()
           ) |
           Ui::vscroll();
}

} // namespace Hideo::Shell
