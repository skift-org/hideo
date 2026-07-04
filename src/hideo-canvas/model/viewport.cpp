export module Hideo.Canvas.Model:viewport;

import Karm.Core;
import Karm.Math;

using namespace Karm;

namespace Hideo::Canvas {

struct Viewport {
    f64 zoom = 1;
    Math::Vec2f pan;
};
} // namespace Hideo::Canvas
