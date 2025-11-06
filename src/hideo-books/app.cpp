export module Hideo.Books;

import Mdi;
import Karm.Core;
import Karm.Kira;
import Karm.Ui;

using namespace Karm;

namespace Hideo::Books {

export Ui::Child app() {
    return Kr::scaffold({
        .icon = Mdi::BOOK,
        .title = "Books"s,
        .body = [] {
            return Ui::empty();
        },
    });
}

} // namespace Hideo::Books
