module;

#include <karm/macros>

export module Hideo.Otp;

import Mdi;
import Karm.Core;
import Karm.Crypto;
import Karm.Sys;
import Karm.Ui;
import Karm.Kira;
import Karm.Logger;

using namespace Karm;
using namespace Karm::Literals;

namespace Hideo::Otp {

struct Otp {
    String name;
    String secret;
    u8 ndigit = 6;
    Duration step = 30_s;
    float countDown = 0.75;
    String code = "000000"s;
};

struct State {
    Vec<Otp> otps;
};

struct Update {};

export using Action = Union<Update>;

Ui::Task<Action> reduce(State& s, Action a) {
    a.visit([&](Update) {
        auto now = Sys::now();
        for (auto& otp : s.otps) {
            auto secret = Crypto::base32Decode(otp.secret.str()).unwrap();
            otp.code = Crypto::totp<Crypto::Sha1>(secret, now, otp.ndigit, otp.step);
            otp.countDown = (now.val() % otp.step.val()) / (f64)otp.step.val();
        }
    });

    return NONE;
}

export using Model = Ui::Model<State, Action, reduce>;

Ui::Child otpCard(Otp const& otp) {
    return Ui::hflow(
               Ui::vflow(
                   Ui::titleSmall(otp.name),
                   Ui::displaySmall(otp.code)
               ) | Ui::grow(),
               Kr::pieCountDown(otp.countDown, 26) | Ui::center()
           ) |
           Ui::insets(8) |
           Kr::card();
}

export Ui::Child app() {
    State state;
    state.otps.pushBack({
        "Google: exemple@gmail.com"s,
        "LZOYN77QAVSZ3WNW3NXQET3KCUCKAMGP"s,
        6,
        30_s,
    });

    return Ui::reducer<Model>(std::move(state), [](State const& s) {
        return Kr::scaffold({
            .icon = Mdi::LOCK,
            .title = "Authenticator"s,
            .startTools = Some([&] -> Ui::Children {
                return {
                    Ui::button(
                        Some(Model::bind<Update>()),
                        Ui::ButtonStyle::primary(),
                        Mdi::PLUS,
                        "Add"
                    )
                };
            }),
            .body = [&] {
                return Ui::vflow(
                           6,
                           iter(s.otps) | Select(otpCard) | Collect<Ui::Children>()
                       ) |
                       Ui::insets(8) | Kr::scaffoldContent();
            },
            .size = {480, 500},
        });
    });
}

export Async::Task<> updateTask(Ui::Child app, Async::CancellationToken ct) {
    while (not ct.cancelled()) {
        Model::event<Update>(*app);
        co_trya$(Sys::globalSched().sleepAsync(Sys::instant() + Duration::fromSecs(1), ct));
    }
    co_return Ok();
}

} // namespace Hideo::Otp
