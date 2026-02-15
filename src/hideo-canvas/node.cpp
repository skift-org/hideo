export module Hideo.Canvas:node;

import Karm.Core;
import :bound;

using namespace Karm;

namespace Hideo::Canvas {

export using Ref = u64;

export enum struct Kind {
    RECT,
    FRAME,
    TEXT,
    GROUP,
};

export struct Node {
    Ref ref;
    Opt<Ref> parent;
    Kind kind;
    Bound bound;

    bool topLevel() const {
        return not parent;
    }
};

} // namespace Hideo::Canvas
