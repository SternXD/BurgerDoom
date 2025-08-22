#pragma once

#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Engine fixed point defines:
// Doom uses 16.16 signed fixed point numbers (in twos complement format) throughout a lot of the game.
// In some places the format is different however, but generally when you see 'Fixed' it means 16.16.
//------------------------------------------------------------------------------------------------------------------------------------------

// Typedef for a fixed point number - mainly for code readability.
// IMPORTANT: does *NOT* have to be 16.16, even though in most places throughout the game it will be!
typedef int32_t Fixed;

// atsb: straight from Becky's 3DO source release

static const uint32_t FRACBITS = 16;
static const Fixed    FRACUNIT = (Fixed)(1) << FRACBITS;
static const Fixed    FRACMASK = FRACUNIT - 1;
static const Fixed    FRACMIN = INT32_MIN;
static const Fixed    FRACMAX = INT32_MAX;

//------------------------------------------------------------------------------------------------------------------------------------------
// Multiply and divide Doom format fixed point numbers (in 16.16 format).
// On a 32-bit CPU this would have been much trickier (due to overflow) but on a modern 64-bit system we can simply
// use native 64-bit types to do this very quickly.
//------------------------------------------------------------------------------------------------------------------------------------------
inline constexpr Fixed fixed16Mul(const Fixed num1, const Fixed num2) noexcept {
    const int64_t result64 = (int64_t) num1 * (int64_t) num2;
    return (Fixed) (result64 >> 16);
}

inline constexpr Fixed fixed16Div(const Fixed num1, const Fixed num2) noexcept {
    const int64_t result64 = (((int64_t) num1) << 16) / (int64_t) num2;
    return (Fixed) result64;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Conversions to and from Doom format fixed point numbers (in 16.16 format) to 32-bit integers
//------------------------------------------------------------------------------------------------------------------------------------------
inline constexpr Fixed intToFixed16(const int32_t num) noexcept {
    return num << 16;
}

inline constexpr int32_t fixed16ToInt(const Fixed num) noexcept {
    return num >> 16;
}

//------------------------------------------------------------------------------------------------------------------
// Conversions to and from Doom format fixed point numbers (16.16 and 26.6) to floats
//------------------------------------------------------------------------------------------------------------------
inline constexpr float fixed16ToFloat(const Fixed fixed) noexcept {
    return float((double) fixed * (1.0 / 65536.0));
}

inline constexpr float fixed6ToFloat(const Fixed fixed) noexcept {
    return float((double) fixed * (1.0 / 64.0));
}

inline constexpr Fixed floatToFixed16(const float value) noexcept {
    return Fixed((double) value * 65536.0);
}

inline constexpr Fixed floatToFixed6(const float value) noexcept {
    return Fixed((double) value * 64.0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Yields a reciprocal of the given Doom format fixed point 16.16 number
//------------------------------------------------------------------------------------------------------------------------------------------
inline constexpr Fixed Fixed16Invert(const Fixed num) noexcept {
    return fixed16Div(FRACUNIT, num);
}
