// PS2 FPU semantics in NEON registers: the same results as ps2FpuClamp /
// ps2FpuAlignOperands without moving the value to the core registers.
#pragma once
#include <arm_neon.h>
#include <cstdint>

static inline __attribute__((always_inline)) float32x2_t ps2NeonClamp(float32x2_t x)
{
    // +-Inf -> +-max; NaN -> its sign | max (the PS2 has neither).
    const float32x2_t max = vreinterpret_f32_u32(vdup_n_u32(0x7F7FFFFFu));
    const float32x2_t clamped = vmin_f32(vmax_f32(x, vneg_f32(max)), max);
    const uint32x2_t bits = vreinterpret_u32_f32(x);
    const uint32x2_t nanFix = vorr_u32(vand_u32(bits, vdup_n_u32(0x80000000u)), vdup_n_u32(0x7F7FFFFFu));
    const uint32x2_t isNumber = vceq_f32(x, x);
    return vreinterpret_f32_u32(vbsl_u32(isNumber, vreinterpret_u32_f32(clamped), nanFix));
}

// The PS2 adder: the smaller operand loses the bits below the larger one's
// precision (diff >= 25: only its sign is left).
static inline __attribute__((always_inline)) void ps2NeonAlign(float32x2_t &a, float32x2_t &b)
{
    const uint32x2_t ua = vreinterpret_u32_f32(a), ub = vreinterpret_u32_f32(b);
    const int32x2_t ea = vreinterpret_s32_u32(vand_u32(vshr_n_u32(ua, 23), vdup_n_u32(0xFFu)));
    const int32x2_t eb = vreinterpret_s32_u32(vand_u32(vshr_n_u32(ub, 23), vdup_n_u32(0xFFu)));
    const int32x2_t diff = vsub_s32(ea, eb);
    const uint32x2_t ones = vdup_n_u32(0xFFFFFFFFu), sign = vdup_n_u32(0x80000000u);
    const int32x2_t one = vdup_n_s32(1), zero = vdup_n_s32(0), big = vdup_n_s32(25);
    // b's mask: diff >= 25 sign only; 0 < diff < 25 ones << (diff - 1); else all.
    const uint32x2_t maskB = vbsl_u32(vcge_s32(diff, big), sign,
                                      vbsl_u32(vcgt_s32(diff, zero), vshl_u32(ones, vmax_s32(vsub_s32(diff, one), zero)), ones));
    const int32x2_t nd = vneg_s32(diff);
    const uint32x2_t maskA = vbsl_u32(vcge_s32(nd, big), sign,
                                      vbsl_u32(vcgt_s32(nd, zero), vshl_u32(ones, vmax_s32(vsub_s32(nd, one), zero)), ones));
    a = vreinterpret_f32_u32(vand_u32(ua, maskA));
    b = vreinterpret_f32_u32(vand_u32(ub, maskB));
}

static inline __attribute__((always_inline)) float ps2NeonAdd(float a, float b)
{
    float32x2_t va = vdup_n_f32(a), vb = vdup_n_f32(b);
    ps2NeonAlign(va, vb);
    // The add itself in VFP (the game's rounding mode; NEON rounds to nearest).
    const float sum = vget_lane_f32(va, 0) + vget_lane_f32(vb, 0);
    return vget_lane_f32(ps2NeonClamp(vdup_n_f32(sum)), 0);
}
static inline __attribute__((always_inline)) float ps2NeonSub(float a, float b)
{
    float32x2_t va = vdup_n_f32(a), vb = vdup_n_f32(b);
    ps2NeonAlign(va, vb);
    const float diff = vget_lane_f32(va, 0) - vget_lane_f32(vb, 0);
    return vget_lane_f32(ps2NeonClamp(vdup_n_f32(diff)), 0);
}
static inline __attribute__((always_inline)) float ps2NeonMul(float a, float b)
{
    return vget_lane_f32(ps2NeonClamp(vdup_n_f32(a * b)), 0);
}
