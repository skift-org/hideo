export module Hideo.Canvas:node;

import Karm.Core;
import :bound;
import :freehand;

using namespace Karm;

namespace Hideo::Canvas {

export using Ref = u64;

export enum struct Kind {
    RECT,
    FRAME,
    FREEHAND,
    TEXT,
    GROUP,
};

export struct Node {
    Ref ref;
    Opt<Ref> parent;
    Kind kind;
    Obb bound;
    Vec<InputPoint> freehand = {};

    bool topLevel() const {
        return not parent;
    }
};

} // namespace Hideo::Canvas
