export module Hideo.Files:model;

import Karm.Core;
import Karm.Ui;
import Karm.Ref;
import Karm.Sys;

using namespace Karm;
using namespace Karm::Literals;

namespace Hideo::Files {

struct State {
    Vec<Ref::Url> history;
    usize currentIndex = 0;
    bool showHidden = false;
    String inputFilename;
    Vec<Sys::DirEntry> directoryListing;
    Opt<Error> directoryError = NONE;

    State(Ref::Url path)
        : history({path}) {
        reloadDirectoryListing();
    }

    Ref::Url currentUrl() const {
        return history[currentIndex];
    }

    void reloadDirectoryListing() {
        auto dir = Sys::Dir::open(currentUrl());
        directoryListing.clear();
        directoryError = NONE;

        if (not dir) {
            directoryError = dir.none();
            return;
        }

        for (auto const& entry : dir.unwrap().entries()) {
            if (entry.hidden() and not showHidden)
                continue;
            directoryListing.pushBack(entry);
        }
    }

    bool canGoBack() const {
        return currentIndex > 0;
    }

    bool canGoForward() const {
        return currentIndex + 1 < history.len();
    }

    bool canGoParent() const {
        return currentUrl().path.len() > 0;
    }
};

struct GoRoot {};

struct GoHome {};

struct GoBack {};

struct GoForward {};

struct GoParent {
    isize index;
};

struct GoTo {
    Ref::Url url;
};

struct Navigate {
    String item;
};

struct Refresh {
};

struct AddBookmark {};

struct ToggleHidden {};

struct SetFilename {
    String name;
};

using Action = Union<
    GoRoot,
    GoBack,
    GoForward,
    GoParent,
    Navigate,
    GoTo,
    Refresh,
    AddBookmark,
    ToggleHidden,
    SetFilename>;

Ui::Task<Action> reduce(State& s, Action a) {
    return a.visit(Visitor{
        [&](GoRoot) {
            return reduce(s, GoTo{s.currentUrl().origin()});
        },
        [&](GoBack) {
            if (s.canGoBack()) {
                s.currentIndex--;
                s.inputFilename = ""s;
                s.reloadDirectoryListing();
            }
            return NONE;
        },
        [&](GoForward) {
            if (s.canGoForward()) {
                s.currentIndex++;
                s.inputFilename = ""s;
                s.reloadDirectoryListing();
            }
            return NONE;
        },
        [&](GoParent p) {
            auto parent = s.currentUrl().parent(p.index);
            return reduce(s, GoTo{parent});
        },
        [&](Navigate navigate) -> Ui::Task<Action> {
            auto dest = s.currentUrl();
            dest.append(navigate.item);

            auto stat = Sys::stat(dest).unwrap();
            if (stat.type == Sys::Type::FILE) {
                (void)Sys::launch({
                    .action = Ref::Uti::PUBLIC_PREVIEW,
                    .objects = {dest},
                });
            } else {
                s.inputFilename = ""s;
                return reduce(s, GoTo{dest});
            }
            return NONE;
        },
        [&](GoTo goTo) {
            if (s.currentUrl() == goTo.url)
                return NONE;

            s.history.trunc(s.currentIndex + 1);
            s.history.pushBack(goTo.url);
            s.currentIndex++;
            s.inputFilename = ""s;
            s.reloadDirectoryListing();
            return NONE;
        },
        [&](Refresh) {
            s.reloadDirectoryListing();
            return NONE;
        },
        [&](AddBookmark) {
            return NONE;
        },
        [&](ToggleHidden) {
            s.showHidden = not s.showHidden;
            s.reloadDirectoryListing();
            return NONE;
        },
        [&](SetFilename sf) {
            s.inputFilename = sf.name;
            return NONE;
        },
    });
}

using Model = Ui::Model<State, Action, reduce>;

} // namespace Hideo::Files
