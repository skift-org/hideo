module;

#include <karm-gfx/buffer.h>

export module Hideo.Images:kernel;

using namespace Karm;

namespace Hideo::Images {

// MARK: Utilites --------------------------------------------------------------

using Linear = Math::Vec4<f32>;

static f32 srgbToLinear(f32 v) {
    v = clamp(v, 0.0f, 1.0f);
    return (v <= 0.04045f)
               ? v / 12.92f
               : Math::pow((v + 0.055f) / 1.055f, 2.4f);
}

static f32 linearToSrgb(f32 v) {
    if (v <= 0.0f)
        return 0.0f;
    return (v <= 0.0031308f)
               ? 12.92f * v
               : 1.055f * Math::pow(v, 1.0f / 2.4f) - 0.055f;
}

static Linear toLinear(Gfx::Color c) {
    return {
        srgbToLinear(c.red / 255.0f),
        srgbToLinear(c.green / 255.0f),
        srgbToLinear(c.blue / 255.0f),
        c.alpha / 255.0f
    };
}

static Gfx::Color toSrgb(Linear l) {
    auto enc = [&](f32 v) -> u8 {
        v = clamp(linearToSrgb(v), 0.0f, 1.0f);
        return static_cast<u8>(Math::round(v * 255.0f));
    };
    return {
        enc(l.x), enc(l.y), enc(l.z),
        static_cast<u8>(Math::round(clamp(l.w, 0.0f, 1.0f) * 255.0f))
    };
}

// Rec.709 luma for scene-linear
static constexpr Math::Vec3<f32> LUMA_COEFF{0.2126f, 0.7152f, 0.0722f};

static inline f32 dot3(Math::Vec3<f32> a, Math::Vec3<f32> b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline f32 mix(f32 a, f32 b, f32 t) {
    return a * (1.0f - t) + b * t;
}

// Simple component-wise mix for Vec3 since we don't support vector ops here
static inline Math::Vec3<f32> mix3(Math::Vec3<f32> a, Math::Vec3<f32> b, f32 t) {
    return {
        a.x * (1.0f - t) + b.x * t,
        a.y * (1.0f - t) + b.y * t,
        a.z * (1.0f - t) + b.z * t,
    };
}

static inline f32 getLuma(Linear v) {
    return dot3({v.x, v.y, v.z}, LUMA_COEFF);
}

static inline f32 smoothstep01(f32 a, f32 b, f32 x) {
    f32 t = clamp01((x - a) / (b - a));
    return t * t * (3.0f - 2.0f * t);
}

static f32 map01ToRange(f32 t, f32 minV, f32 maxV) {
    return minV + (maxV - minV) * clamp01(t);
}

static f32 mapRangeTo01(f32 v, f32 minV, f32 maxV) {
    f32 span = maxV - minV;
    if (span == 0.0f)
        return 0.0f;
    return (v - minV) / span;
}
// ---- Adjustments ------------------------------------------------------------

enum struct Adjustment {
    EXPOSURE,
    CONTRAST,
    HIGHLIGHTS,
    SHADOWS,
    WHITES,
    BLACKS,
    TEMPERATURE,
    TINT,
    VIBRANCE,
    SATURATION,
    VIGNETTE_AMOUNT,
    VIGNETTE_MIDPOINT,
    VIGNETTE_ROUNDNESS,
    VIGNETTE_FEATHER,

    // Lens distortion
    LENS_RADIAL1,
    LENS_RADIAL2,
    LENS_RADIAL3,
    LENS_TANGENT_X,
    LENS_TANGENT_Y,
    LENS_CENTER_X,
    LENS_CENTER_Y,
    LENS_SCALE,

    _LEN,
};

enum struct KernelFlags {
    HIGHLIGHT_CLIP = 1 << 0,
    SHADOW_CLIP = 1 << 1,
};

// ---- Kernel -----------------------------------------------------------------

struct Kernel {
    // Tone/color
    f32 exposure = 0.0f;
    f32 contrast = 0.0f;
    f32 highlights = 0.0f;
    f32 shadows = 0.0f;
    f32 whites = 0.0f;
    f32 blacks = 0.0f;
    f32 temperature = 0.0f;
    f32 tint = 0.0f;
    f32 vibrance = 0.0f;
    f32 saturation = 0.0f;

    // Vignette
    f32 vignetteAmount = 0.0f;
    f32 vignetteMidpoint = 0.5f;
    f32 vignetteRoundness = 0.0f;
    f32 vignetteFeather = 0.5f;

    // Lens distortion (Brown–Conrady), off by default
    f32 lensK1 = 0.0f, lensK2 = 0.0f, lensK3 = 0.0f; // radial
    f32 lensP1 = 0.0f, lensP2 = 0.0f;                // tangential
    f32 lensCx = 0.5f, lensCy = 0.5f;                // normalized principal point
    f32 lensScale = 1.0f;                            // normalization scale (1 = baseline)

    // ---- Configuration ------------------------------------------------------

    void reset(Adjustment a) {
        switch (a) {
        case Adjustment::EXPOSURE:
            exposure = 0.0f;
            break;
        case Adjustment::CONTRAST:
            contrast = 0.0f;
            break;
        case Adjustment::HIGHLIGHTS:
            highlights = 0.0f;
            break;
        case Adjustment::SHADOWS:
            shadows = 0.0f;
            break;
        case Adjustment::WHITES:
            whites = 0.0f;
            break;
        case Adjustment::BLACKS:
            blacks = 0.0f;
            break;
        case Adjustment::TEMPERATURE:
            temperature = 0.0f;
            break;
        case Adjustment::TINT:
            tint = 0.0f;
            break;
        case Adjustment::VIBRANCE:
            vibrance = 0.0f;
            break;
        case Adjustment::SATURATION:
            saturation = 0.0f;
            break;
        case Adjustment::VIGNETTE_AMOUNT:
            vignetteAmount = 0.0f;
            break;
        case Adjustment::VIGNETTE_MIDPOINT:
            vignetteMidpoint = 0.5f;
            break;
        case Adjustment::VIGNETTE_ROUNDNESS:
            vignetteRoundness = 0.0f;
            break;
        case Adjustment::VIGNETTE_FEATHER:
            vignetteFeather = 0.5f;
            break;

        case Adjustment::LENS_RADIAL1:
            lensK1 = 0.0f;
            break;
        case Adjustment::LENS_RADIAL2:
            lensK2 = 0.0f;
            break;
        case Adjustment::LENS_RADIAL3:
            lensK3 = 0.0f;
            break;
        case Adjustment::LENS_TANGENT_X:
            lensP1 = 0.0f;
            break;
        case Adjustment::LENS_TANGENT_Y:
            lensP2 = 0.0f;
            break;
        case Adjustment::LENS_CENTER_X:
            lensCx = 0.5f;
            break;
        case Adjustment::LENS_CENTER_Y:
            lensCy = 0.5f;
            break;
        case Adjustment::LENS_SCALE:
            lensScale = 1.0f;
            break;

        default:
            unreachable();
        }
    }

    void change(Adjustment a, f32 v) {
        switch (a) {
        case Adjustment::EXPOSURE:
            exposure = map01ToRange(v, -3.0f, +3.0f);
            break;
        case Adjustment::CONTRAST:
            contrast = map01ToRange(v, -1.0f, +1.0f);
            break;
        case Adjustment::HIGHLIGHTS:
            highlights = map01ToRange(v, -1.0f, +1.0f);
            break;
        case Adjustment::SHADOWS:
            shadows = map01ToRange(v, -1.0f, +1.0f);
            break;
        case Adjustment::WHITES:
            whites = map01ToRange(v, -1.0f, +1.0f);
            break;
        case Adjustment::BLACKS:
            blacks = map01ToRange(v, -1.0f, +1.0f);
            break;
        case Adjustment::TEMPERATURE:
            temperature = map01ToRange(v, -1.0f, +1.0f);
            break;
        case Adjustment::TINT:
            tint = map01ToRange(v, -1.0f, +1.0f);
            break;
        case Adjustment::VIBRANCE:
            vibrance = map01ToRange(v, -1.0f, +1.0f);
            break;
        case Adjustment::SATURATION:
            saturation = map01ToRange(v, -1.0f, +1.0f);
            break;
        case Adjustment::VIGNETTE_AMOUNT:
            vignetteAmount = map01ToRange(v, -1.0f, +1.0f);
            break;
        case Adjustment::VIGNETTE_MIDPOINT:
            vignetteMidpoint = clamp01(v);
            break;
        case Adjustment::VIGNETTE_ROUNDNESS:
            vignetteRoundness = clamp01(v);
            break;
        case Adjustment::VIGNETTE_FEATHER:
            vignetteFeather = clamp01(v);
            break;

        // Lens ranges chosen to be useful, not chaotic
        case Adjustment::LENS_RADIAL1:
            lensK1 = map01ToRange(v, -0.1f, +0.1f);
            break;
        case Adjustment::LENS_RADIAL2:
            lensK2 = map01ToRange(v, -0.1f, +0.1f);
            break;
        case Adjustment::LENS_RADIAL3:
            lensK3 = map01ToRange(v, -0.1f, +0.1f);
            break;
        case Adjustment::LENS_TANGENT_X:
            lensP1 = map01ToRange(v, -0.01f, +0.01f);
            break;
        case Adjustment::LENS_TANGENT_Y:
            lensP2 = map01ToRange(v, -0.01f, +0.01f);
            break;
        case Adjustment::LENS_CENTER_X:
            lensCx = clamp01(v);
            break;
        case Adjustment::LENS_CENTER_Y:
            lensCy = clamp01(v);
            break;
        case Adjustment::LENS_SCALE:
            lensScale = map01ToRange(v, 0.5f, 2.0f);
            break;

        default:
            unreachable();
        }
    }

    f32 sliderValue(Adjustment a) const {
        switch (a) {
        case Adjustment::EXPOSURE:
            return clamp01(mapRangeTo01(exposure, -3.0f, +3.0f));
        case Adjustment::CONTRAST:
            return clamp01(mapRangeTo01(contrast, -1.0f, +1.0f));
        case Adjustment::HIGHLIGHTS:
            return clamp01(mapRangeTo01(highlights, -1.0f, +1.0f));
        case Adjustment::SHADOWS:
            return clamp01(mapRangeTo01(shadows, -1.0f, +1.0f));
        case Adjustment::WHITES:
            return clamp01(mapRangeTo01(whites, -1.0f, +1.0f));
        case Adjustment::BLACKS:
            return clamp01(mapRangeTo01(blacks, -1.0f, +1.0f));
        case Adjustment::TEMPERATURE:
            return clamp01(mapRangeTo01(temperature, -1.0f, +1.0f));
        case Adjustment::TINT:
            return clamp01(mapRangeTo01(tint, -1.0f, +1.0f));
        case Adjustment::VIBRANCE:
            return clamp01(mapRangeTo01(vibrance, -1.0f, +1.0f));
        case Adjustment::SATURATION:
            return clamp01(mapRangeTo01(saturation, -1.0f, +1.0f));
        case Adjustment::VIGNETTE_AMOUNT:
            return clamp01(mapRangeTo01(vignetteAmount, -1.0f, +1.0f));
        case Adjustment::VIGNETTE_MIDPOINT:
            return clamp01(vignetteMidpoint);
        case Adjustment::VIGNETTE_ROUNDNESS:
            return clamp01(vignetteRoundness);
        case Adjustment::VIGNETTE_FEATHER:
            return clamp01(vignetteFeather);

        case Adjustment::LENS_RADIAL1:
            return clamp01(mapRangeTo01(lensK1, -0.1f, +0.1f));
        case Adjustment::LENS_RADIAL2:
            return clamp01(mapRangeTo01(lensK2, -0.1f, +0.1f));
        case Adjustment::LENS_RADIAL3:
            return clamp01(mapRangeTo01(lensK3, -0.1f, +0.1f));
        case Adjustment::LENS_TANGENT_X:
            return clamp01(mapRangeTo01(lensP1, -0.01f, +0.01f));
        case Adjustment::LENS_TANGENT_Y:
            return clamp01(mapRangeTo01(lensP2, -0.01f, +0.01f));
        case Adjustment::LENS_CENTER_X:
            return clamp01(lensCx);
        case Adjustment::LENS_CENTER_Y:
            return clamp01(lensCy);
        case Adjustment::LENS_SCALE:
            return clamp01(mapRangeTo01(lensScale, 0.5f, 2.0f));

        default:
            unreachable();
        }
    }

    f32 value(Adjustment a) const {
        switch (a) {
        case Adjustment::EXPOSURE:
            return exposure;
        case Adjustment::CONTRAST:
            return contrast;
        case Adjustment::HIGHLIGHTS:
            return highlights;
        case Adjustment::SHADOWS:
            return shadows;
        case Adjustment::WHITES:
            return whites;
        case Adjustment::BLACKS:
            return blacks;
        case Adjustment::TEMPERATURE:
            return temperature;
        case Adjustment::TINT:
            return tint;
        case Adjustment::VIBRANCE:
            return vibrance;
        case Adjustment::SATURATION:
            return saturation;
        case Adjustment::VIGNETTE_AMOUNT:
            return vignetteAmount;
        case Adjustment::VIGNETTE_MIDPOINT:
            return vignetteMidpoint;
        case Adjustment::VIGNETTE_ROUNDNESS:
            return vignetteRoundness;
        case Adjustment::VIGNETTE_FEATHER:
            return vignetteFeather;

        case Adjustment::LENS_RADIAL1:
            return lensK1;
        case Adjustment::LENS_RADIAL2:
            return lensK2;
        case Adjustment::LENS_RADIAL3:
            return lensK3;
        case Adjustment::LENS_TANGENT_X:
            return lensP1;
        case Adjustment::LENS_TANGENT_Y:
            return lensP2;
        case Adjustment::LENS_CENTER_X:
            return lensCx;
        case Adjustment::LENS_CENTER_Y:
            return lensCy;
        case Adjustment::LENS_SCALE:
            return lensScale;

        default:
            unreachable();
        }
    }

    // ---- Tone/color pipeline ------------------------------------------------

    Linear exposureApply(Linear v) const {
        if (exposure == 0.0f)
            return v;
        f32 s = Math::pow(2.0f, exposure);
        return {v.x * s, v.y * s, v.z * s, v.w};
    }

    Linear whiteBalanceApply(Linear v) const {
        if (temperature == 0.0f and tint == 0.0f)
            return v;
        Math::Vec3<f32> tempMult{
            1.0f + temperature * 0.20f,
            1.0f + temperature * 0.05f,
            1.0f - temperature * 0.20f
        };
        Math::Vec3<f32> tintMult{
            1.0f + tint * 0.25f,
            1.0f - tint * 0.25f,
            1.0f + tint * 0.25f
        };
        f32 r = v.x * tempMult.x * tintMult.x;
        f32 g = v.y * tempMult.y * tintMult.y;
        f32 b = v.z * tempMult.z * tintMult.z;
        return {max(0.0f, r), max(0.0f, g), max(0.0f, b), v.w};
    }

    Linear contrastApply(Linear v) const {
        if (contrast == 0.0f)
            return v;
        constexpr f32 PIVOT = 0.18f;
        f32 slope = Math::pow(2.0f, contrast);
        auto applyC = [&](f32 c) {
            return (c - PIVOT) * slope + PIVOT;
        };
        return {applyC(v.x), applyC(v.y), applyC(v.z), v.w};
    }

    Linear whitesBlacksApply(Linear v) const {
        if (whites == 0.0f and blacks == 0.0f)
            return v;
        Linear rgb = v;
        if (whites != 0.0f) {
            f32 whiteLevel = 1.0f - whites * 0.25f;
            f32 denom = max(whiteLevel, 0.01f);
            rgb.x /= denom;
            rgb.y /= denom;
            rgb.z /= denom;
        }
        if (blacks != 0.0f) {
            f32 l = max(0.0f, getLuma(rgb));
            f32 mask = 1.0f - smoothstep01(0.0f, 0.25f, l);
            if (mask > 0.001f) {
                f32 adjustment = blacks * 0.75f;
                f32 factor = Math::pow(2.0f, adjustment);
                Linear adjusted{rgb.x * factor, rgb.y * factor, rgb.z * factor, rgb.w};
                rgb.x = mix(rgb.x, adjusted.x, mask);
                rgb.y = mix(rgb.y, adjusted.y, mask);
                rgb.z = mix(rgb.z, adjusted.z, mask);
            }
        }
        return rgb;
    }

    Linear hiShApply(Linear v) const {
        if (highlights == 0.0f and shadows == 0.0f)
            return v;
        f32 l = clamp(getLuma(v), 0.0f, 1.0f);
        Linear rgb = v;
        f32 hiMask = smoothstep01(0.20f, 0.80f, l);
        f32 shMaskBase = 1.0f - smoothstep01(0.0f, 0.40f, l);
        f32 shMask = shMaskBase * shMaskBase * shMaskBase;
        auto scaleBy = [&](f32 gain, f32 m, f32 amt) {
            if (amt == 0.0f or m <= 0.0f)
                return gain;
            f32 f = Math::pow(2.0f, amt);
            return gain * (1.0f - m) + gain * f * m;
        };
        f32 r = rgb.x, g = rgb.y, b = rgb.z;
        f32 hiAmt = highlights * 1.5f;
        f32 shAmt = shadows * 1.5f;
        r = scaleBy(r, hiMask, hiAmt);
        g = scaleBy(g, hiMask, hiAmt);
        b = scaleBy(b, hiMask, hiAmt);
        r = scaleBy(r, shMask, shAmt);
        g = scaleBy(g, shMask, shAmt);
        b = scaleBy(b, shMask, shAmt);
        return {r, g, b, v.w};
    }

    Linear vibranceSaturationApply(Linear v) const {
        if (vibrance == 0.0f and saturation == 0.0f)
            return v;
        f32 luma = getLuma(v);
        Math::Vec3<f32> base{luma, luma, luma};
        Math::Vec3<f32> color{v.x, v.y, v.z};
        if (saturation != 0.0f) {
            f32 sAmt = 1.0f + saturation;
            color = mix3(base, color, sAmt);
        }
        if (vibrance != 0.0f) {
            f32 satCurrent = Math::abs(color.x - luma) + Math::abs(color.y - luma) + Math::abs(color.z - luma);
            f32 vibMask = 1.0f - clamp01(satCurrent * 0.5f);
            f32 vAmt = 1.0f + vibrance * vibMask;
            color = mix3(base, color, vAmt);
        }
        return {color.x, color.y, color.z, v.w};
    }

    Linear vignetteApply(Linear v, f32 x, f32 y, f32 w, f32 h) const {
        if (vignetteAmount == 0.0f)
            return v;

        f32 nx = (x + 0.5f) / w;
        f32 ny = (y + 0.5f) / h;
        f32 cx = (nx - 0.5f) * 2.0f;
        f32 cy = (ny - 0.5f) * 2.0f;

        f32 aspect = h / max(1.0f, w);

        f32 rExp = 1.0f - clamp01(vignetteRoundness);
        auto shape = [&](f32 q) {
            return __builtin_copysign(Math::pow(Math::abs(q), rExp), q);
        };
        f32 sx = shape(cx);
        f32 sy = shape(cy);

        f32 d = Math::sqrt(sx * sx + (sy * aspect) * (sy * aspect)) * 0.5f;
        f32 mid = clamp01(vignetteMidpoint);
        f32 feather = clamp01(vignetteFeather) * 0.5f;
        f32 mask = smoothstep01(mid - feather, mid + feather, d);

        Linear out = v;
        if (vignetteAmount < 0.0f) {
            f32 k = 1.0f + vignetteAmount * mask;
            out.x *= k;
            out.y *= k;
            out.z *= k;
        } else {
            f32 t = vignetteAmount * mask;
            out.x = mix(out.x, 1.0f, t);
            out.y = mix(out.y, 1.0f, t);
            out.z = mix(out.z, 1.0f, t);
        }
        return out;
    }

    Linear clippingOverlay(Linear v, Flags<KernelFlags> flags) const {
        bool hi = (v.x > 1.0f) or (v.y > 1.0f) or (v.z > 1.0f);
        bool sh = (v.x < 0.0f) or (v.y < 0.0f) or (v.z < 0.0f);

        if ((flags.has(KernelFlags::HIGHLIGHT_CLIP) and hi) and
            (flags.has(KernelFlags::SHADOW_CLIP) and sh))
            return {1.0f, 0.0f, 1.0f, v.w};

        if (flags.has(KernelFlags::HIGHLIGHT_CLIP) and hi)
            return {1.0f, 0.0f, 0.0f, v.w};

        if (flags.has(KernelFlags::SHADOW_CLIP) and sh)
            return {0.0f, 0.0f, 1.0f, v.w};

        return v;
    }

    [[gnu::flatten]] Linear apply(Linear v, isize x, isize y, isize w, isize h, Flags<KernelFlags> flags) const {
        v = exposureApply(v);
        v = whiteBalanceApply(v);
        v = contrastApply(v);
        v = whitesBlacksApply(v);
        v = hiShApply(v);
        v = vibranceSaturationApply(v);
        v = vignetteApply(v, static_cast<f32>(x), static_cast<f32>(y), static_cast<f32>(w), static_cast<f32>(h));
        v = clippingOverlay(v, flags);
        return v;
    }

    // ---- Lens warp path -----------------------------------------------------

    bool hasLensCorrection() const {
        // lensScale != 1.0 should NOT force a warp if all coeffs are zero
        bool anyCoeff = lensK1 or lensK2 or lensK3 or lensP1 or lensP2;
        bool centerMoved = (lensCx != 0.5f) or (lensCy != 0.5f);
        return anyCoeff or centerMoved;
    }

    static inline Linear sampleBilinear(Gfx::Pixels src, f32 xs, f32 ys) {
        f32 x = clamp(xs, 0.0f, (f32)src.width() - 1.001f);
        f32 y = clamp(ys, 0.0f, (f32)src.height() - 1.001f);

        isize x0 = (isize)Math::floor(x);
        isize y0 = (isize)Math::floor(y);
        isize x1 = min(x0 + 1, src.width() - 1);
        isize y1 = min(y0 + 1, src.height() - 1);

        f32 tx = x - (f32)x0;
        f32 ty = y - (f32)y0;

        auto c00 = toLinear(src.loadUnsafe({x0, y0}));
        auto c10 = toLinear(src.loadUnsafe({x1, y0}));
        auto c01 = toLinear(src.loadUnsafe({x0, y1}));
        auto c11 = toLinear(src.loadUnsafe({x1, y1}));

        auto lerp = [](Linear a, Linear b, f32 t) {
            return Linear{
                mix(a.x, b.x, t),
                mix(a.y, b.y, t),
                mix(a.z, b.z, t),
                mix(a.w, b.w, t),
            };
        };

        Linear cx0 = lerp(c00, c10, tx);
        Linear cx1 = lerp(c01, c11, tx);
        return lerp(cx0, cx1, ty);
    }

    inline void distortUv(f32 u, f32 v, f32& ud, f32& vd) const {
        f32 r2 = u * u + v * v;
        f32 r4 = r2 * r2;
        f32 r6 = r4 * r2;

        f32 radial = 1.0f + lensK1 * r2 + lensK2 * r4 + lensK3 * r6;

        f32 two_uv = 2.0f * u * v;
        f32 r2p2u2 = r2 + 2.0f * u * u;
        f32 r2p2v2 = r2 + 2.0f * v * v;

        ud = u * radial + lensP1 * r2p2u2 + lensP2 * two_uv;
        vd = v * radial + lensP1 * two_uv + lensP2 * r2p2v2;
    }

    void applyWithLens(Gfx::Pixels in, Gfx::MutPixels out, Flags<KernelFlags> flags) const {
        isize w = in.width();
        isize h = in.height();

        // Focal length in pixels: edge of the shorter side ~ 1.0 radius when lensScale == 1
        f32 fPix = max(1.0f, 0.5f * (f32)min(w, h) / max(1e-6f, lensScale));

        // Principal point in pixels
        f32 cxPix = clamp(lensCx, 0.0f, 1.0f) * (f32)w;
        f32 cyPix = clamp(lensCy, 0.0f, 1.0f) * (f32)h;

        for (isize y : range(h)) {
            for (isize x : range(w)) {
                // 1) undistorted camera coords (in pixels -> normalized by fPix)
                f32 dx = (x + 0.5f) - cxPix;
                f32 dy = (y + 0.5f) - cyPix;
                f32 u = dx / fPix;
                f32 v = dy / fPix;

                // 2) forward distort (Brown–Conrady)
                f32 r2 = u * u + v * v;
                // Bail early if absurd radius to avoid NaNs and edge clamping soup
                if (r2 > 1e6f) {
                    // fall back to identity sample to keep output sane
                    auto lin = toLinear(in.loadUnsafe({x, y}));
                    auto adj = apply(lin, x, y, w, h, flags);
                    out.storeUnsafe({x, y}, toSrgb(adj));
                    continue;
                }

                f32 r4 = r2 * r2;
                f32 r6 = r4 * r2;
                f32 radial = 1.0f + lensK1 * r2 + lensK2 * r4 + lensK3 * r6;

                f32 two_uv = 2.0f * u * v;
                f32 r2p2u2 = r2 + 2.0f * u * u;
                f32 r2p2v2 = r2 + 2.0f * v * v;

                f32 ud = u * radial + lensP1 * r2p2u2 + lensP2 * two_uv;
                f32 vd = v * radial + lensP1 * two_uv + lensP2 * r2p2v2;

                // 3) back to source pixels
                f32 sx = ud * fPix + cxPix - 0.5f;
                f32 sy = vd * fPix + cyPix - 0.5f;

                // 4) sample + color pipeline
                Linear lin = sampleBilinear(in, sx, sy);
                Linear adj = apply(lin, x, y, w, h, flags);
                out.storeUnsafe({x, y}, toSrgb(adj));
            }
        }
    }

    // ---- Dispatch -----------------------------------------------------------

    void apply(Gfx::Pixels in, Gfx::MutPixels out, Flags<KernelFlags> flags) const {
        if (hasLensCorrection()) {
            applyWithLens(in, out, flags);
            return;
        }

        isize w = in.width();
        isize h = in.height();
        for (isize y : range(h)) {
            for (isize x : range(w)) {
                auto c = in.loadUnsafe({x, y});
                auto lin = toLinear(c);
                auto adj = apply(lin, x, y, w, h, flags);
                out.storeUnsafe({x, y}, toSrgb(adj));
            }
        }
    }
};

// ---- Analysis object ---------------------------------------------------------

struct ImageAnalysis {
    static constexpr int HN = 1024;

    // Histograms + accumulators
    Array<f64, HN> histAll{};
    Array<f64, HN> histContent{};
    f64 nAll = 0.0, nContent = 0.0;

    // Content-weighted midtone WB accumulators
    f64 accR = 0.0, accG = 0.0, accB = 0.0;
    usize accN = 0;

    // Content-weighted percentiles
    f32 p01 = 0, p25 = 0, p50 = 0, p75 = 0, p99 = 0;

    // Overall DR anchors
    f32 a01 = 0, a99 = 0, dr = 0;

    // Blue dominance proxy
    f32 blueDom = 1.0f;  // >1 means blue-heavy
    f32 blueDamp = 1.0f; // damping factor derived from blueDom

    // Convenience spread
    f32 iqr = 0;
};

export ImageAnalysis analyze(Gfx::Pixels const& in) {
    ImageAnalysis a{};

    auto binOf = [&](f32 l) -> int {
        int b = static_cast<int>(clamp(l, 0.0f, 1.0f) * (ImageAnalysis::HN - 1));
        return clamp(b, 0, ImageAnalysis::HN - 1);
    };

    // Heuristic: blue-sky detector and "content" weighting.
    auto contentWeight = [&](Linear v, f32 l) -> f32 {
        bool blueDominant = (v.z > v.x + 0.05f) and (v.z > v.y + 0.05f) and (l > 0.12f) and (l < 0.85f);
        bool nearWhite = (v.x > 0.92f) and (v.y > 0.92f) and (v.z > 0.92f);
        bool deepShadow = l < 0.03f;

        f32 w = 1.0f;
        if (blueDominant)
            w *= 0.25f;
        if (nearWhite)
            w *= 0.25f;
        if (deepShadow)
            w *= 0.5f;
        return w;
    };

    isize w = in.width();
    isize h = in.height();

    for (isize y : range(h)) {
        for (isize x : range(w)) {
            auto c = in.loadUnsafe({x, y});
            Linear v = toLinear(c);
            f32 l = clamp(getLuma(v), 0.0f, 1.0f);

            int b = binOf(l);
            a.histAll[b] += 1.0;
            a.nAll += 1.0;

            f32 wgt = contentWeight(v, l);
            a.histContent[b] += wgt;
            a.nContent += wgt;

            if (l >= 0.15f and l <= 0.85f) {
                a.accR += v.x * wgt;
                a.accG += v.y * wgt;
                a.accB += v.z * wgt;
                a.accN += (wgt > 0.0f);
            }
        }
    }

    auto percentile = [&](Slice<f64> H, f64 N, f64 q) -> f32 {
        if (N <= 0.0)
            return (f32)q;
        f64 target = q * (N - 1.0);
        f64 run = 0.0;
        for (int i = 0; i < ImageAnalysis::HN; ++i) {
            run += H[i];
            if (run > target)
                return (f32)i / (f32)(ImageAnalysis::HN - 1);
        }
        return 1.0f;
    };

    // Content-weighted percentiles
    a.p01 = percentile(a.histContent, a.nContent, 0.01);
    a.p25 = percentile(a.histContent, a.nContent, 0.25);
    a.p50 = percentile(a.histContent, a.nContent, 0.50);
    a.p75 = percentile(a.histContent, a.nContent, 0.75);
    a.p99 = percentile(a.histContent, a.nContent, 0.99);

    // Overall DR for damping
    a.a01 = percentile(a.histAll, a.nAll, 0.01);
    a.a99 = percentile(a.histAll, a.nAll, 0.99);
    a.dr = clamp(a.a99 - a.a01, 0.0f, 1.0f);

    // Blue dominance proxy from midtones
    f64 sumRGB = a.accR + a.accG + a.accB;
    a.blueDom = (sumRGB > 0.0) ? (f32)(a.accB / (sumRGB / 3.0)) : 1.0f;
    a.blueDamp = clamp(1.5f - a.blueDom, 0.4f, 1.0f);

    // Spread
    a.iqr = max(1e-4f, a.p75 - a.p25);

    return a;
}

export Kernel autoAdjust(ImageAnalysis a) {
    Kernel p{};

    // ---- Exposure -----------------------------------------------------------
    {
        f32 med = max(1e-5f, a.p50);
        f32 targetMid = 0.18f;
        f32 raw = Math::log2(targetMid / med);
        f32 drD = 1.0f - smoothstep01(0.85f, 0.95f, a.dr);
        f32 medD = 1.0f - clamp(Math::abs(med - targetMid) * 1.5f, 0.0f, 0.5f);
        f32 strength = clamp(drD * a.blueDamp * medD, 0.35f, 1.0f);
        p.exposure = clamp(raw * strength, -0.40f, +0.40f);
    }

    // ---- Contrast -----------------------------------------------------------
    {
        f32 targetIqr = 0.30f;
        f32 rawScale = targetIqr / a.iqr;
        f32 rawContrast = Math::log2(rawScale);

        f32 iqrD = 1.0f - smoothstep01(0.26f, 0.45f, a.iqr);
        f32 drD = 1.0f - smoothstep01(0.85f, 0.97f, a.dr);
        f32 exD = 1.0f - clamp(Math::abs(p.exposure) * 0.9f, 0.0f, 0.5f);
        f32 strength = clamp(iqrD * drD * exD * a.blueDamp, 0.0f, 1.0f);

        p.contrast = clamp(rawContrast * strength, -0.50f, +0.50f);
    }

    // ---- Blacks / Whites ----------------------------------------------------
    {
        if (a.p01 > 0.012f and a.p01 < 0.08f) {
            f32 s = clamp(0.012f / a.p01, 0.6f, 1.2f);
            f32 blacksAdj = Math::log2(s) / 0.75f;
            f32 damp = min(1.0f - smoothstep01(0.88f, 0.98f, a.dr), a.blueDamp);
            p.blacks = clamp(blacksAdj * damp, -0.30f, +0.30f);
        } else {
            p.blacks = 0.0f;
        }

        if (a.p99 < 0.985f and a.p99 > 0.70f) {
            f32 s = clamp(0.985f / max(1e-5f, a.p99), 0.7f, 1.4f);
            f32 denom = 1.0f / s;
            f32 whitesAdj = (1.0f - denom) / 0.25f;
            f32 damp = min(1.0f - smoothstep01(0.88f, 0.98f, a.dr), a.blueDamp);
            p.whites = clamp(whitesAdj * damp, -0.30f, +0.30f);
        } else {
            p.whites = 0.0f;
        }
    }

    // ---- Gray-world WB on midtones -----------------------------------------
    if (a.accN > 100) {
        f64 mr = a.accR / (f64)a.accN;
        f64 mg = a.accG / (f64)a.accN;
        f64 mb = a.accB / (f64)a.accN;
        f64 m = (mr + mg + mb) / 3.0;

        f64 sR = (mr > 1e-8 ? m / mr : 1.0);
        f64 sG = (mg > 1e-8 ? m / mg : 1.0);
        f64 sB = (mb > 1e-8 ? m / mb : 1.0);

        f64 aR = sR - 1.0, aG = sG - 1.0, aB = sB - 1.0;
        f64 T = (aR - aB) / 0.40;
        f64 U = ((aR - aG) - 0.15 * T) / 0.50;

        p.temperature = clamp((f32)T * a.blueDamp, -0.6f, +0.6f);
        p.tint = clamp((f32)U * a.blueDamp, -0.6f, +0.6f);
    }

    // ---- Subtle pop if truly flat -------------------------------------------
    if (max(1e-4f, a.p75 - a.p25) < 0.18f) {
        p.vibrance = +0.10f;
        p.saturation = +0.04f;
    }

    p.vignetteAmount = 0.0f;
    return p;
}

} // namespace Hideo::Images

namespace Hideo::Images::Presets {

// Utility
export Kernel const NEUTRAL = {};
export Kernel const PUNCHY = {.contrast = +0.5f, .vibrance = +0.2f, .saturation = +0.2f};
export Kernel const FLAT = {.contrast = -0.5f, .saturation = -0.2f};

// Color moods
export Kernel const WARM_SUNSET = {.highlights = +0.1f, .blacks = +0.2f, .temperature = +0.3f, .tint = +0.1f};
export Kernel const COOL_MIST = {.shadows = +0.2f, .temperature = -0.3f, .tint = +0.1f, .vignetteAmount = +0.2f};
export Kernel const RETRO_FADE = {.contrast = -0.2f, .blacks = -0.3f, .vignetteAmount = +0.1f};

// Stylized
export Kernel const HIGH_KEY = {.exposure = +1.0f, .contrast = -0.3f, .highlights = +0.3f, .vignetteAmount = -0.3f};
export Kernel const TEAL_ORANGE = {.contrast = +0.3f, .temperature = +0.2f, .tint = -0.2f, .vibrance = +0.3f, .vignetteAmount = +0.2f};
export Kernel const MOODY_LOW_KEY = {.exposure = -0.7f, .contrast = +0.5f, .blacks = -0.3f, .vignetteAmount = +0.4f};

// Film-lite
export Kernel const KODACHROME = {.contrast = +0.4f, .blacks = -0.2f, .temperature = +0.2f, .vibrance = +0.2f};
export Kernel const MATTE_FILM = {.contrast = -0.2f, .blacks = +0.2f, .vignetteAmount = -0.1f};

// Artsy
export Kernel const BLEACH_BYPASS = {.contrast = +0.5f, .highlights = +0.2f, .blacks = -0.2f, .saturation = -0.4f};
export Kernel const SEPIA_FADE = {.blacks = +0.2f, .temperature = +0.6f, .tint = +0.2f, .saturation = -0.4f, .vignetteAmount = +0.3f};
export Kernel const DREAMY_GLOW = {.exposure = +0.3f, .contrast = -0.3f, .highlights = +0.4f, .vignetteAmount = -0.2f};
export Kernel const CROSS_PROCESS = {.contrast = +0.2f, .temperature = -0.4f, .tint = -0.3f, .vibrance = +0.3f};
export Kernel const NOIR_CRUSH = {.contrast = +0.6f, .blacks = +0.2f, .saturation = -1.0f, .vignetteAmount = +0.5f};
export Kernel const POP_ART_PUNCH = {.exposure = +0.3f, .contrast = +0.3f, .vibrance = +0.5f, .saturation = +0.5f};
export Kernel const PASTEL_DREAM = {.exposure = +0.2f, .contrast = -0.3f, .temperature = +0.3f, .vibrance = +0.3f, .vignetteAmount = -0.3f};
export Kernel const CYBERPUNK = {.contrast = +0.5f, .temperature = -0.2f, .tint = +0.4f, .vibrance = +0.5f, .vignetteAmount = +0.2f};

} // namespace Hideo::Images::Presets
