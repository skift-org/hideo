export module Hideo.Files:widgets;

import Karm.App;
import Karm.Ui;
import Karm.Kira;
import Karm.Ref;
import Karm.Sys;
import Karm.Gfx;
import Karm.Math;
import Mdi;

import :model;

namespace Hideo::Files {

struct Type2Icon {
    Ref::Uti type;
    Gfx::Icon icon;
};

static Array _type2Icon = {
    Type2Icon{Ref::Uti::PUBLIC_ITEM, Mdi::FILE},
    Type2Icon{Ref::Uti::PUBLIC_DIRECTORY, Mdi::FOLDER},
    Type2Icon{Ref::Uti::PUBLIC_HTML, Mdi::LANGUAGE_HTML5},
    Type2Icon{Ref::Uti::PUBLIC_CSS, Mdi::LANGUAGE_CSS3},
    Type2Icon{Ref::Uti::PUBLIC_JAVASCRIPT, Mdi::LANGUAGE_JAVASCRIPT},
    Type2Icon{Ref::Uti::PUBLIC_TEXT, Mdi::FILE_DOCUMENT},
    Type2Icon{Ref::Uti::PUBLIC_IMAGE, Mdi::IMAGE},
    Type2Icon{Ref::Uti::PUBLIC_JPEG, Mdi::FILE_JPG_BOX},
    Type2Icon{Ref::Uti::PUBLIC_PNG, Mdi::FILE_PNG_BOX},
    Type2Icon{Ref::Uti::PUBLIC_AV, Mdi::FILMSTRIP},
    Type2Icon{Ref::Uti::PUBLIC_FONT, Mdi::FORMAT_FONT},
    Type2Icon{Ref::Uti::PUBLIC_PDF, Mdi::FILE_PDF_BOX},
    Type2Icon{Ref::Uti::PUBLIC_JSON, Mdi::CODE_JSON},
    Type2Icon{Ref::Uti::PUBLIC_ARCHIVE, Mdi::ZIP_BOX}
};

Gfx::Icon iconFor(Ref::Uti type) {
    Gfx::Icon best = Mdi::FILE;
    u64 bestRank = 0;
    for (auto const& m : _type2Icon) {
        if (type == m.type)
            return m.icon;

        if (bestRank < m.type.rank() and type.conformsTo(m.type)) {
            best = m.icon;
            bestRank = m.type.rank();
        }
    }
    return best;
}

Gfx::Icon iconFor(Sys::DirEntry const& entry) {
    if (entry.hidden()) {
        return entry.type == Sys::Type::DIR ? Mdi::FOLDER_HIDDEN : Mdi::FILE_HIDDEN;
    } else {
        return iconFor(entry.uti());
    }
}

// MARK: Common Widgets --------------------------------------------------------

export Ui::Child alert(State const& state, String title, String body) {
    return Kr::errorPageContent({
        Kr::errorPageTitle(Mdi::ALERT_DECAGRAM, title),
        Kr::errorPageBody(body),
        Kr::errorPageFooter({
            Ui::button(Model::bindIf<GoBack>(state.canGoBack()), "Go Back"),
            Ui::button(Model::bind<Refresh>(), Ui::ButtonStyle::primary(), "Retry"),
        }),
    });
}

Ui::Child directoryContextMenu() {
    return Kr::contextMenuContent({
        Kr::contextMenuDock({
            Kr::contextMenuIcon(Ui::SINK<>, Mdi::CONTENT_COPY),
            Kr::contextMenuIcon(Ui::SINK<>, Mdi::CONTENT_CUT),
            // Kr::contextMenuIcon(Ui::SINK, Mdi::CONTENT_PASTE),
            Kr::contextMenuIcon(Ui::SINK<>, Mdi::FORM_TEXTBOX),
            Ui::grow(NONE),
            Kr::separator(),
            Kr::contextMenuIcon(Ui::SINK<>, Mdi::DELETE_OUTLINE),
        }),
        Kr::separator(),
        Kr::contextMenuItem(Ui::SINK<>, Mdi::MAGNIFY, "Preview"),
        Kr::contextMenuItem(Ui::SINK<>, Mdi::PENCIL, "Modify"),
        Kr::contextMenuItem(Ui::SINK<>, Mdi::SHARE, "Interact…"),
        Kr::separator(),
        Kr::contextMenuItem(Ui::SINK<>, Mdi::INFORMATION_OUTLINE, "Properties"),
    });
}

Ui::Child directorEntry(Sys::DirEntry const& entry) {
    return Ui::button(
               Model::bind<Navigate>(entry.name),
               Ui::ButtonStyle::subtle(),
               iconFor(entry),
               entry.name
           ) |
           Kr::selectionItem() |
           Kr::contextMenu(directoryContextMenu);
}

Ui::Child directoryListing(State const& s, Sys::Dir const& dir) {
    if (dir.entries().len() == 0)
        return Ui::bodyMedium(Ui::GRAY500, "This directory is empty.") | Ui::center();

    Ui::Children children;
    for (auto const& entry : dir.entries()) {
        if (entry.hidden() and not s.showHidden)
            continue;
        children.pushBack(directorEntry(entry));
    }

    return Ui::vflow(8, children) |
           Ui::insets(16) |
           Kr::selectionArea() |
           Ui::vscroll() | Ui::key(s.currentIndex);
}

// MARK: Dialog Widgets --------------------------------------------------------

Ui::Child dialogEntry(State const& s, Sys::DirEntry const& entry) {
    auto isDir = entry.type == Sys::Type::DIR;
    auto isSelected = not isDir and s.inputFilename == entry.name;
    auto style = isSelected ? Ui::ButtonStyle::regular() : Ui::ButtonStyle::subtle();
    return Ui::button(
        Model::bind<SetFilename>(entry.name),
        style,
        iconFor(entry),
        entry.name
    );
}

export Ui::Child dialogDirectoryListing(State const& s, Sys::Dir const& dir) {
    if (dir.entries().len() == 0)
        return Ui::bodyMedium(Ui::GRAY500, "This directory is empty.") | Ui::center();

    Ui::Children children;
    for (auto const& entry : dir.entries()) {
        if (entry.hidden() and not s.showHidden)
            continue;
        children.pushBack(dialogEntry(s, entry));
    }

    return Ui::vflow(8, children) |
           Ui::insets(16) |
           Ui::vscroll() | Ui::key(s.currentIndex);
}

Ui::Child breadcrumbItem(Str text, isize index) {
    return Ui::hflow(
        0,
        Math::Align::CENTER,
        Ui::icon(Mdi::CHEVRON_RIGHT),
        Ui::button(
            Model::bind<GoParent>(index),
            Ui::ButtonStyle::text().withPadding({2, 0}),
            Ui::text(text)
        )
    );
}

Gfx::Icon iconForLocation(Str loc) {
    if (eqCi(loc, "home"s))
        return Mdi::HOME;

    if (eqCi(loc, "documents"s))
        return Mdi::FILE_DOCUMENT;

    if (eqCi(loc, "pictures"s))
        return Mdi::IMAGE;

    if (eqCi(loc, "music"s))
        return Mdi::MUSIC;

    if (eqCi(loc, "videos"s))
        return Mdi::FILM;

    if (eqCi(loc, "downloads"s))
        return Mdi::DOWNLOAD;

    return Mdi::FOLDER;
}

Gfx::Icon iconForUrl(Ref::Url const& url) {
    if (url.scheme == "location")
        return iconForLocation(url.host.str());

    if (url.scheme == "device")
        return Mdi::HARDDISK;

    return Mdi::LAPTOP;
}

String textForUrl(Ref::Url const& url) {
    if (url.scheme == "location")
        return Io::toTitleCase(url.host.str()).unwrap();

    if (url.scheme == "device")
        return url.host.str();

    return "This Device"s;
}

Ui::Child breadcrumbRoot(Ref::Url const& url) {
    return Ui::button(
        Model::bind<GoRoot>(),
        Ui::ButtonStyle::text(),
        Ui::hflow(
            8,
            Math::Align::CENTER,
            Ui::icon(iconForUrl(url)),
            Ui::text(textForUrl(url))
        )
    );
}

export Ui::Child refreshTool() {
    return Ui::button(
               Model::bind<Refresh>(),
               Ui::ButtonStyle::subtle(),
               Mdi::REFRESH
           ) |
           Ui::keyboardShortcut(App::Key::R, App::KeyMod::ALT);
}

export Ui::Child breadcrumb(State const& s) {
    Ui::Children items;
    items.pushBack(breadcrumbRoot(s.currentUrl()));

    s.currentUrl().iter() |
        ForEachi([&](auto const& text, usize i) {
            items.pushBack(breadcrumbItem(text, s.currentUrl().len() - i - 1));
            return true;
        });

    return Ui::box(
               {
                   .borderRadii = 4,
                   .backgroundFill = Ui::GRAY800,
               },
               Ui::hflow(
                   Ui::empty(12),
                   Ui::hflow(items) |
                       Ui::hscroll() |
                       Ui::grow(),
                   refreshTool()
               )
           ) |
           Ui::focusable() |
           Ui::keyboardShortcut(App::Key::L, App::KeyMod::CTRL);
}

export Ui::Child goBackTool(State const& s) {
    return Ui::button(
               Model::bindIf<GoBack>(s.canGoBack()),
               Ui::ButtonStyle::subtle(),
               Mdi::ARROW_LEFT
           ) |
           Ui::keyboardShortcut(App::Key::LEFT, App::KeyMod::ALT);
}

export Ui::Child goForwardTool(State const& s) {
    return Ui::button(
               Model::bindIf<GoForward>(s.canGoForward()),
               Ui::ButtonStyle::subtle(),
               Mdi::ARROW_RIGHT
           ) |
           Ui::keyboardShortcut(App::Key::RIGHT, App::KeyMod::ALT);
}

export Ui::Child goParentTool(State const& s) {
    return Ui::button(
               Model::bindIf<GoParent>(s.canGoParent(), 1),
               Ui::ButtonStyle::subtle(),
               Mdi::ARROW_UP
           ) |
           Ui::keyboardShortcut(App::Key::UP, App::KeyMod::ALT);
}

export Ui::Child mainMenu([[maybe_unused]] State const& s) {
    return Kr::contextMenuContent({
        Kr::contextMenuItem(
            Ui::SINK<>,
            Mdi::BOOKMARK_OUTLINE, "Add bookmark..."
        ),
        Kr::contextMenuItem(Ui::SINK<>, Mdi::BOOKMARK, "Bookmarks"),
        Kr::separator(),
        Kr::contextMenuCheck(Model::bind<ToggleHidden>(), s.showHidden, "Show hidden"),
    });
}

export Ui::Child moreTool(State const& s) {
    return Ui::button(
        [&](Ui::Node& n) {
            Ui::showPopover(n, n.bound().bottomEnd(), mainMenu(s));
        },
        Ui::ButtonStyle::subtle(),
        Mdi::DOTS_HORIZONTAL
    );
}

export Ui::Child toolbar(State const& s) {
    return Kr::toolbar({
        goBackTool(s),
        goForwardTool(s),
        goParentTool(s),
        breadcrumb(s) | Ui::grow(),
        moreTool(s),
    });
}

} // namespace Hideo::Files
