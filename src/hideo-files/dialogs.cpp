export module Hideo.Files:dialogs;

import Karm.Kira;
import Karm.Ui;
import Karm.Sys;
import Karm.Math;

import :widgets;

namespace Hideo::Files {

export Ui::Child openDialog(Ui::Send<Ref::Url> onFile) {
    return Ui::reducer<Model>(
        {"location://home"_url},
        [onFile](State const& s) {
            auto maybeDir = Sys::Dir::open(s.currentUrl());
            bool hasSelection = s.inputFilename.len() > 0;

            return Kr::dialogContent({
                Kr::dialogTitleBar("Open File…"s),
                toolbar(s),
                (maybeDir
                     ? dialogDirectoryListing(s, maybeDir.unwrap())
                     : alert(
                           s,
                           "Can't access this location"s,
                           Io::toStr(maybeDir.none())
                       )) |
                    Ui::pinSize({500, 300}),
                Kr::separator(),
                Kr::dialogFooter({
                    Ui::grow(NONE),
                    Kr::dialogCancel(),
                    Ui::button(
                        hasSelection
                            ? Opt<Ui::Send<>>{[&, onFile](auto& n) {
                                  auto url = s.currentUrl();
                                  url.append(s.inputFilename);
                                  onFile(n, url);
                              }}
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
            auto maybeDir = Sys::Dir::open(s.currentUrl());
            bool hasFilename = s.inputFilename.len() > 0;

            return Kr::dialogContent({
                Kr::dialogTitleBar("Save As…"s),
                toolbar(s),
                (maybeDir
                     ? dialogDirectoryListing(s, maybeDir.unwrap())
                     : alert(
                           s,
                           "Can't access this location"s,
                           Io::toStr(maybeDir.none())
                       )) |
                    Ui::pinSize({400, 260}),
                Kr::separator(),
                Kr::dialogFooter({
                    Kr::input("Filename"s, s.inputFilename, Model::map<SetFilename>()) | Ui::grow(),
                    Kr::dialogCancel(),
                    Ui::button(
                        hasFilename
                            ? Opt<Ui::Send<>>{[&, onFile](auto& n) {
                                  auto url = s.currentUrl();
                                  url.append(s.inputFilename);
                                  onFile(n, url);
                              }}
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
            auto maybeDir = Sys::Dir::open(d.currentUrl());

            return Kr::dialogContent({
                Kr::dialogTitleBar("Select Directory…"s),
                toolbar(d),
                (maybeDir
                     ? directoryListing(d, maybeDir.unwrap())
                     : alert(
                           d,
                           "Can't access this location"s,
                           Io::toStr(maybeDir.none())
                       )) |
                    Ui::pinSize({400, 260}),
                Kr::separator(),
                Kr::dialogFooter({
                    Ui::grow(NONE),
                    Kr::dialogCancel(),
                    Kr::dialogAction(
                        [&, onFile](auto& n) {
                            onFile(n, d.currentUrl());
                        },
                        "Select"s
                    ),
                }),
            });
        }
    );
}

} // namespace Hideo::Files
