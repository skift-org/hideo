export module Hideo.Files:dialogs;

import Karm.Kira;
import Karm.Ui;
import Karm.Sys;
import Karm.Math;

import :widgets;

using namespace Karm::Literals;
using namespace Karm::Ref::Literals;

namespace Hideo::Files {

export Ui::Child openDialog(Ui::Send<Ref::Url> onFile) {
    return Ui::reducer<Model>(
        {"location://home"_url},
        [onFile](State const& s) {
            bool hasSelection = s.inputFilename.len() > 0;

            return Kr::dialogContent({
                Kr::dialogTitleBar("Open File…"s),
                toolbar(s),
                (not s.directoryError
                     ? dialogDirectoryListing(s)
                     : alert(
                           s,
                           "Can't access this location"s,
                           Io::toStr(*s.directoryError)
                       )) |
                    Ui::pinSize({500, 300}) |
                    Kr::scaffoldContent() |
                    Ui::insets({0, 6}),
                Kr::dialogFooter({
                    Ui::grow(NONE),
                    Kr::dialogCancel(),
                    Ui::button(
                        hasSelection
                            ? Opt<Ui::Send<>>{Some([&, onFile](auto& n) {
                                  auto url = s.currentUrl();
                                  url.append(s.inputFilename);
                                  onFile(n, url);
                              })}
                            : NONE,
                        Ui::ButtonStyle::primary(),
                        "Open"s
                    ),
                }),
            });
        }
    );
}

export Ui::Child saveDialog(Ui::Send<Ref::Url> onFile) {
    return Ui::reducer<Model>(
        {"location://home"_url},
        [onFile](State const& s) {
            bool hasFilename = s.inputFilename.len() > 0;

            return Kr::dialogContent({
                Kr::dialogTitleBar("Save As…"s),
                toolbar(s),
                (not s.directoryError
                     ? dialogDirectoryListing(s)
                     : alert(
                           s,
                           "Can't access this location"s,
                           Io::toStr(*s.directoryError)
                       )) |
                    Ui::pinSize({500, 300}) |
                    Kr::scaffoldContent() |
                    Ui::insets({0, 6}),
                Kr::dialogFooter({
                    Kr::input("Filename"s, s.inputFilename, Model::map<SetFilename>()) | Ui::grow(),
                    Kr::dialogCancel(),
                    Ui::button(
                        hasFilename
                            ? Opt<Ui::Send<>>{Some([&, onFile](auto& n) {
                                  auto url = s.currentUrl();
                                  url.append(s.inputFilename);
                                  onFile(n, url);
                              })}
                            : NONE,
                        Ui::ButtonStyle::primary(),
                        "Save"s
                    ),
                }),
            });
        }
    );
}

export Ui::Child directoryDialog(Ui::Send<Ref::Url> onFile) {
    return Ui::reducer<Model>(
        {"location://home"_url},
        [onFile](auto const& d) {
            return Kr::dialogContent({
                Kr::dialogTitleBar("Select Directory…"s),
                toolbar(d),
                (not d.directoryError
                     ? directoryListing(d)
                     : alert(
                           d,
                           "Can't access this location"s,
                           Io::toStr(*d.directoryError)
                       )) |
                    Ui::pinSize({500, 300}) |
                    Kr::scaffoldContent() |
                    Ui::insets({0, 6}),
                Kr::dialogFooter({
                    Ui::grow(NONE),
                    Kr::dialogCancel(),
                    Kr::dialogAction(
                        Some([&, onFile](auto& n) {
                            onFile(n, d.currentUrl());
                        }),
                        "Select"s
                    ),
                }),
            });
        }
    );
}

} // namespace Hideo::Files
