export module Hideo.Text;

import Mdi;
import Karm.Ui;
import Karm.Kira;
import Karm.Sys;
import Karm.Ref;
import Karm.Math;
import Karm.Logger;
import Karm.Core;
import Karm.App;

import Hideo.Files;

using namespace Karm;

namespace Hideo::Text {

struct State {
    Opt<Ref::Url> url;
    Opt<Error> error;
    Rc<Ui::TextModel> text;
};

struct New {
};

struct Open {
    Ref::Url url;
    String content;
};

struct Save {
    bool prompt = false;
};

struct SaveAs {
    Ref::Url url;
};

using Action = Union<Ui::TextAction, New, Open, Save, SaveAs>;

Ui::Task<Action> reduce(State& s, Action a) {
    a.visit(::Visitor{
        [&](Ui::TextAction& t) {
            s.text->reduce(t);
        },
        [&](New&) {
            s.url = NONE;
            s.error = NONE;
            s.text = makeRc<Ui::TextModel>();
        },
        [&](Open& o) {
            s.url = o.url;
            s.error = NONE;
            s.text = makeRc<Ui::TextModel>();
            s.text->load(o.content);
        },
        [&](Save& save) {
            if (save.prompt or not s.url) {
                // NOTE: Save-as is handled by the UI showing a dialog;
                //       once the user picks a url, a SaveAs is dispatched.
                return;
            }
            auto file = Sys::File::create(*s.url);
            if (not file) {
                s.error = file.none();
                return;
            }
            Io::TextEncoder<> enc{file.unwrap()};
            (void)enc.writeStr(s.text->string().str());
            s.text->flush();
        },
        [&](SaveAs& sa) {
            s.url = sa.url;
            auto file = Sys::File::create(*s.url);
            if (not file) {
                s.error = file.none();
                return;
            }
            Io::TextEncoder<> enc{file.unwrap()};
            (void)enc.writeStr(s.text->string().str());
            s.text->flush();
        },
    });

    return NONE;
}

using Model = Ui::Model<State, Action, reduce>;

Ui::Child editor(Rc<Ui::TextModel> text) {
    return Ui::input(
               text,
               [](Ui::Node& n, Action a) {
                   Model::bubble(n, a);
               }
           ) |
           Ui::focusable({.visual = false, .steal = true}) | Ui::insets(16) |
           Ui::vscroll() |
           Ui::grow();
}

// MARK: Toolbar ---------------------------------------------------------------

Ui::Children appToolbar(State const& s) {
    Ui::Send<> openAction = [](Ui::Node& n) {
        Ui::showDialog(
            n,
            Files::openDialog([](auto& n, auto url) {
                Ui::closeDialog(n);
                auto content = Sys::readAllUtf8(url);
                if (content)
                    Model::bubble<Open>(n, Open{url, content.unwrap()});
            })
        );
    };

    Ui::Send<> saveAsAction = [](auto& n) {
        Ui::showDialog(
            n,
            Files::saveDialog([](auto& n, auto url) {
                Ui::closeDialog(n);
                Model::bubble<SaveAs>(n, SaveAs{url});
            })
        );
    };

    return {
        Ui::button(Model::bind<New>(), Ui::ButtonStyle::subtle(), Mdi::FILE) | Ui::keyboardShortcut(App::Key::N, App::KeyMod::CTRL),
        Ui::button(openAction, Ui::ButtonStyle::subtle(), Mdi::FOLDER) | Ui::keyboardShortcut(App::Key::O, App::KeyMod::CTRL),
        Ui::button(Model::bindIf<Save>(s.text->dirty() and s.url), Ui::ButtonStyle::subtle(), Mdi::CONTENT_SAVE) | Ui::keyboardShortcut(App::Key::S, App::KeyMod::CTRL),
        Ui::button(saveAsAction, Ui::ButtonStyle::subtle(), Mdi::CONTENT_SAVE_PLUS) | Ui::keyboardShortcut(App::Key::S, {App::KeyMod::CTRL, App::KeyMod::SHIFT}),
    };
}

export Ui::Child app(Opt<Ref::Url> url, Res<String> str) {
    auto text = makeRc<Ui::TextModel>();
    Opt<Error> error = NONE;

    if (str) {
        text->load(str.unwrap());
    } else {
        error = str.none();
    }

    return Ui::reducer<Model>(
        State{
            url,
            error,
            text,
        },
        [](State const& s) {
            return Kr::scaffold({
                .icon = Mdi::PEN,
                .title = "Text"s,
                .startTools = [&] -> Ui::Children {
                    return appToolbar(s);
                },
                .endTools = [&] -> Ui::Children {
                    return {
                        Ui::button(
                            Model::bindIf<Ui::TextAction>(s.text->canUndo(), Ui::TextAction::UNDO),
                            Ui::ButtonStyle::subtle(),
                            Mdi::UNDO
                        ),
                        Ui::button(
                            Model::bindIf<Ui::TextAction>(s.text->canRedo(), Ui::TextAction::REDO),
                            Ui::ButtonStyle::subtle(),
                            Mdi::REDO
                        )
                    };
                },
                .body = [=] {
                    usize ln = 1, col = 1;
                    auto head = s.text->_cur.head;
                    auto runes = s.text->runes();
                    for (usize i = 0; i < head and i < runes.len(); i++) {
                        if (runes[i] == '\n') {
                            ln++;
                            col = 1;
                        } else {
                            col++;
                        }
                    }

                    return Ui::vflow(
                        Ui::hflow(
                            0,
                            Math::Align::CENTER,
                            Ui::labelSmall("{}{}", s.url ? s.url->basename() : "Untitled", s.text->dirty() ? "*" : ""),
                            Ui::icon(Mdi::CIRCLE_SMALL, Ui::GRAY700) | Ui::insets({0, -3}),
                            Ui::text(Ui::TextStyles::labelSmall().withColor(Ui::GRAY500), "{}", s.url)
                        ) | Ui::insets({6, 16}),
                        Kr::separator(),

                        s.error
                            ? Kr::errorPage(Mdi::ALERT_DECAGRAM, "Unable to load text"s, Io::toStr(s.error)) | Ui::grow()
                            : editor(s.text),
                        Kr::separator(),
                        Ui::hflow(
                            6,
                            Math::Align::CENTER,
                            Ui::labelSmall("{}", s.text->dirty() ? "Edited" : ""),
                            Ui::grow(NONE),
                            Ui::labelSmall("Ln {}, Col {}", ln, col),
                            Kr::separator(),
                            Ui::labelSmall("UTF-8"),
                            Kr::separator(),
                            Ui::labelSmall("LF")
                        ) | Ui::box({
                                .padding = {6, 12},
                                .backgroundFill = Ui::GRAY900,
                            })
                    );
                },
            });
        }
    );
}

} // namespace Hideo::Text
