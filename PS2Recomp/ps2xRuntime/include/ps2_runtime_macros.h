#ifndef PS2_RUNTIME_MACROS_H
#define PS2_RUNTIME_MACROS_H
#ifndef PS2_FORCE_INLINE
// The recompiled game is built for size (-Os), which turns these one-line
// helpers (every guest load/store goes through them) into calls.
#if defined(_MSC_VER)
#define PS2_FORCE_INLINE __forceinline
#else
#define PS2_FORCE_INLINE __attribute__((always_inline)) inline
#endif
#endif
#include <cstdint>
#include <cmath>
#include <cstring>
#include <bit>
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(USE_SSE2NEON)
#include "sse2neon.h"
#else
#include <immintrin.h> // For SSE/AVX intrinsics
#endif

#include "ps2_runtime.h"

static PS2_FORCE_INLINE int32_t Ps2ExtractEpi32(__m128i v, int index)
{
    switch (index & 3)
    {
    case 0:
        return _mm_extract_epi32(v, 0);
    case 1:
        return _mm_extract_epi32(v, 1);
    case 2:
        return _mm_extract_epi32(v, 2);
    default:
        return _mm_extract_epi32(v, 3);
    }
}

static PS2_FORCE_INLINE int64_t Ps2ExtractEpi64(__m128i v, int index)
{
    if ((index & 1) == 0)
    {
        return _mm_cvtsi128_si64(v);
    }
    else
    {
        return _mm_extract_epi64(v, 1);
    }
}

static PS2_FORCE_INLINE uint32_t ps2_clz32(uint32_t x)
{
    return static_cast<uint32_t>(std::countl_zero(x));
}

static PS2_FORCE_INLINE uint64_t Ps2HiLoToU64(uint64_t hi, uint64_t lo)
{
    return ((hi & 0xFFFFFFFFull) << 32) | (lo & 0xFFFFFFFFull);
}

static PS2_FORCE_INLINE uint64_t Ps2SignExt32ToU64(uint32_t v)
{
    return (uint64_t)(int64_t)(int32_t)v;
}

// PLZCW: Count leading bits that match the sign bit, minus 1.
// For positive values: count leading zeros minus 1 (excludes sign bit).
// For negative values: count leading ones minus 1 (excludes sign bit).
// Special cases: 0x00000000 -> 31, 0xFFFFFFFF -> 31.
static PS2_FORCE_INLINE uint32_t ps2_plzcw32(uint32_t x)
{
    if (x == 0 || x == 0xFFFFFFFF)
        return 31;
    if (x & 0x80000000u)
        x = ~x; // If sign bit set, invert to count leading ones as zeros
    return static_cast<uint32_t>(std::countl_zero(x)) - 1;
}

#define PS2_BLENDV_PS(a, b, mask) _mm_blendv_ps((a), (b), (mask))
#define PS2_MIN_EPI32(a, b) _mm_min_epi32((a), (b))
#define PS2_MAX_EPI32(a, b) _mm_max_epi32((a), (b))
#define PS2_SHUFFLE_EPI8(v, mask) _mm_shuffle_epi8((v), (mask))

#define PS2_EXTRACT_EPI32(v, i) Ps2ExtractEpi32((v), (i))
#define PS2_EXTRACT_EPI64(v, i) Ps2ExtractEpi64((v), (i))

#define PS2_EXTRACT_EPI32_0(v) Ps2ExtractEpi32((v), 0)
#define PS2_EXTRACT_EPI32_1(v) Ps2ExtractEpi32((v), 1)
#define PS2_EXTRACT_EPI32_2(v) Ps2ExtractEpi32((v), 2)
#define PS2_EXTRACT_EPI32_3(v) Ps2ExtractEpi32((v), 3)

#define PS2_EXTRACT_EPI64_0(v) Ps2ExtractEpi64((v), 0)
#define PS2_EXTRACT_EPI64_1(v) Ps2ExtractEpi64((v), 1)

// Basic MIPS arithmetic operations
#define ADD32(a, b) ((uint32_t)((a) + (b)))
#define ADD32_OV(rs, rt, result32, overflow)              \
    do                                                    \
    {                                                     \
        int32_t _a = (int32_t)(rs);                       \
        int32_t _b = (int32_t)(rt);                       \
        int32_t _r = _a + _b;                             \
        overflow = (((_a ^ _b) >= 0) && ((_a ^ _r) < 0)); \
        result32 = (uint32_t)_r;                          \
    } while (0);
#define SUB32(a, b) ((uint32_t)((a) - (b)))
#define SUB32_OV(rs, rt, result32, overflow)             \
    do                                                   \
    {                                                    \
        int32_t _a = (int32_t)(rs);                      \
        int32_t _b = (int32_t)(rt);                      \
        int32_t _r = _a - _b;                            \
        overflow = (((_a ^ _b) < 0) && ((_a ^ _r) < 0)); \
        result32 = (uint32_t)_r;                         \
    } while (0);
#define MUL32(a, b) ((uint32_t)((a) * (b)))
#define DIV32(a, b) ((uint32_t)((a) / (b)))
#define AND32(a, b) ((uint32_t)((a) & (b)))
#define OR32(a, b) ((uint32_t)((a) | (b)))
#define XOR32(a, b) ((uint32_t)((a) ^ (b)))
#define NOR32(a, b) ((uint32_t)(~((a) | (b))))
#define SLL32(a, b) ((uint32_t)((a) << (b)))
#define SRL32(a, b) ((uint32_t)((a) >> (b)))
#define SRA32(a, b) ((uint32_t)((int32_t)(a) >> (b)))
#define SLT32(a, b) ((uint32_t)((int32_t)(a) < (int32_t)(b) ? 1 : 0))
#define SLTU32(a, b) ((uint32_t)((a) < (b) ? 1 : 0))

// PS2-specific 128-bit MMI operations
#define PS2_PEXTLW(a, b) _mm_unpacklo_epi32((__m128i)(b), (__m128i)(a))
#define PS2_PEXTUW(a, b) _mm_unpackhi_epi32((__m128i)(b), (__m128i)(a))
#define PS2_PEXTLH(a, b) _mm_unpacklo_epi16((__m128i)(b), (__m128i)(a))
#define PS2_PEXTUH(a, b) _mm_unpackhi_epi16((__m128i)(b), (__m128i)(a))
#define PS2_PEXTLB(a, b) _mm_unpacklo_epi8((__m128i)(b), (__m128i)(a))
#define PS2_PEXTUB(a, b) _mm_unpackhi_epi8((__m128i)(b), (__m128i)(a))
#define PS2_PADDW(a, b) _mm_add_epi32((__m128i)(a), (__m128i)(b))
#define PS2_PSUBW(a, b) _mm_sub_epi32((__m128i)(a), (__m128i)(b))
#define PS2_PMAXW(a, b) PS2_MAX_EPI32((__m128i)(a), (__m128i)(b))
#define PS2_PMINW(a, b) PS2_MIN_EPI32((__m128i)(a), (__m128i)(b))
#define PS2_PADDH(a, b) _mm_add_epi16((__m128i)(a), (__m128i)(b))
#define PS2_PSUBH(a, b) _mm_sub_epi16((__m128i)(a), (__m128i)(b))
#define PS2_PMAXH(a, b) _mm_max_epi16((__m128i)(a), (__m128i)(b))
#define PS2_PMINH(a, b) _mm_min_epi16((__m128i)(a), (__m128i)(b))
#define PS2_PADDB(a, b) _mm_add_epi8((__m128i)(a), (__m128i)(b))
#define PS2_PSUBB(a, b) _mm_sub_epi8((__m128i)(a), (__m128i)(b))
#define PS2_PAND(a, b) _mm_and_si128((__m128i)(a), (__m128i)(b))
#define PS2_POR(a, b) _mm_or_si128((__m128i)(a), (__m128i)(b))
#define PS2_PXOR(a, b) _mm_xor_si128((__m128i)(a), (__m128i)(b))
#define PS2_PNOR(a, b) _mm_xor_si128(_mm_or_si128((__m128i)(a), (__m128i)(b)), _mm_set1_epi32(0xFFFFFFFF))

// PS2 VU (Vector Unit) operations
// VU floats have no denormals, infinities or NaNs: denormals read and
// produce signed zero, Inf/NaN read as signed FLT_MAX and overflow clamps to
// signed FLT_MAX. Without this, NaN spreads through game math (bone matrices).
#if defined(__arm__) && !defined(__aarch64__)
// 32-bit ARM (the Vita): the same results in fewer NEON instructions (6
// instead of ~10, three times per VU0 multiply). NEON flushes denormals to
// signed zero itself, as below; min/max take +-Inf to +-max; what NEON keeps
// as NaN (exponent 255 with a mantissa: integers moved into VF registers,
// 0xFFFFFFFF masks) becomes its sign | max as below. Bit-exact against the
// SSE version over 10M patterns (with flush to zero on, as ARMv7 NEON is).
static PS2_FORCE_INLINE __m128 ps2VuNormalize(__m128 v)
{
    const float32x4_t max = vreinterpretq_f32_u32(vdupq_n_u32(0x7F7FFFFFu));
    const float32x4_t clamped = vminq_f32(vmaxq_f32(v, vnegq_f32(max)), max);
    const uint32x4_t bits = vreinterpretq_u32_f32(v);
    const uint32x4_t nanFix = vorrq_u32(vandq_u32(bits, vdupq_n_u32(0x80000000u)), vdupq_n_u32(0x7F7FFFFFu));
    return vreinterpretq_f32_u32(vbslq_u32(vceqq_f32(v, v), vreinterpretq_u32_f32(clamped), nanFix));
}
#else
static PS2_FORCE_INLINE __m128 ps2VuNormalize(__m128 v)
{
    const __m128i bits = _mm_castps_si128(v);
    const __m128i sign = _mm_and_si128(bits, _mm_set1_epi32(static_cast<int>(0x80000000u)));
    const __m128i exponent = _mm_and_si128(bits, _mm_set1_epi32(0x7F800000));
    const __m128i isZeroExp = _mm_cmpeq_epi32(exponent, _mm_setzero_si128());
    const __m128i isMaxExp = _mm_cmpeq_epi32(exponent, _mm_set1_epi32(0x7F800000));
    __m128i result = _mm_or_si128(_mm_andnot_si128(isZeroExp, bits), _mm_and_si128(isZeroExp, sign));
    const __m128i clamped = _mm_or_si128(sign, _mm_set1_epi32(0x7F7FFFFF));
    result = _mm_or_si128(_mm_andnot_si128(isMaxExp, result), _mm_and_si128(isMaxExp, clamped));
    return _mm_castsi128_ps(result);
}
#endif
static PS2_FORCE_INLINE float ps2VuNormalizeScalar(float v)
{
    return _mm_cvtss_f32(ps2VuNormalize(_mm_set1_ps(v)));
}
// VU CLIP judgement (same as the VU1 interpreter's CLIP): x, y, z of fs against
// +|w| and -|w| of ft, compared as ordered integers so denormals behave like
// the VU. Bits: +x 0, -x 1, +y 2, -y 3, +z 4, -z 5.
static PS2_FORCE_INLINE uint32_t ps2VuClipFlags(__m128 fsv, __m128 ftv)
{
    alignas(16) float fs[4];
    alignas(16) float ft[4];
    _mm_store_ps(fs, fsv);
    _mm_store_ps(ft, ftv);
    uint32_t wBits = 0u;
    std::memcpy(&wBits, &ft[3], sizeof(wBits));
    const int32_t limit = (wBits & 0x7F800000u) != 0u ? static_cast<int32_t>(wBits & 0x7FFFFFFFu) : 0x007FFFFF;
    auto exceeds = [limit](float value, uint32_t signMask)
    {
        uint32_t bits = 0u;
        std::memcpy(&bits, &value, sizeof(bits));
        bits ^= signMask;
        int32_t ordered = 0;
        std::memcpy(&ordered, &bits, sizeof(ordered));
        return ordered > limit;
    };
    uint32_t flags = 0u;
    for (int c = 0; c < 3; ++c)
    {
        if (exceeds(fs[c], 0x00000000u))
            flags |= 1u << (2 * c);
        if (exceeds(fs[c], 0x80000000u))
            flags |= 2u << (2 * c);
    }
    return flags;
}

#define PS2_VADD(a, b) ps2VuNormalize(_mm_add_ps(ps2VuNormalize((__m128)(a)), ps2VuNormalize((__m128)(b))))
#define PS2_VSUB(a, b) ps2VuNormalize(_mm_sub_ps(ps2VuNormalize((__m128)(a)), ps2VuNormalize((__m128)(b))))
#define PS2_VMUL(a, b) ps2VuNormalize(_mm_mul_ps(ps2VuNormalize((__m128)(a)), ps2VuNormalize((__m128)(b))))
#define PS2_VDIV(a, b) _mm_div_ps((__m128)(a), (__m128)(b))
#define PS2_VMULQ(a, q) _mm_mul_ps((__m128)(a), _mm_set1_ps(q))
#define PS2_VBLEND(a, b, mask) PS2_BLENDV_PS((__m128)(a), (__m128)(b), (__m128)(mask))

// Memory access helpers - Hybrid Fast/Slow Path
// Fast path: Direct RDRAM access (masked).
// Slow path: Full runtime->Load/Store

// A shuffle of one vector with itself (every broadcast and swizzle the VU0
// code makes; the recompiler emits it for _mm_shuffle_ps(v, v, imm)). On
// 32-bit ARM GCC both sse2neon's _mm_shuffle_ps and a generic permute are
// table lookups (vtbl): a broadcast is one vdup.
#if defined(__arm__) && !defined(__aarch64__) && defined(__GNUC__) && !defined(__clang__)
template <int Imm>
static PS2_FORCE_INLINE __m128 ps2ShuffleSame(__m128 v)
{
    if constexpr (Imm == 0x00)
        return vdupq_lane_f32(vget_low_f32(v), 0);
    else if constexpr (Imm == 0x55)
        return vdupq_lane_f32(vget_low_f32(v), 1);
    else if constexpr (Imm == 0xAA)
        return vdupq_lane_f32(vget_high_f32(v), 0);
    else if constexpr (Imm == 0xFF)
        return vdupq_lane_f32(vget_high_f32(v), 1);
    else if constexpr (Imm == 0xE4)
        return v;
    else
        return (__m128)__builtin_shuffle((float32x4_t)v, (uint32x4_t){Imm & 3u, (Imm >> 2) & 3u, (Imm >> 4) & 3u, (Imm >> 6) & 3u});
}
#define PS2_SHUFFLE_SAME(v, imm) ps2ShuffleSame<(imm)>(v)
#else
#define PS2_SHUFFLE_SAME(v, imm) _mm_shuffle_ps((v), (v), (imm))
#endif

static PS2_FORCE_INLINE bool Ps2FastRangeIsContiguous(uint32_t offset, uint32_t bytes)
{
    return offset <= (PS2_RAM_SIZE - bytes);
}

static PS2_FORCE_INLINE uint8_t Ps2FastRead8(const uint8_t *rdram, uint32_t addr)
{
    return rdram[addr & PS2_RAM_MASK];
}

// The access that wraps at the end of RAM, out of line: inlined at every
// guest load and store it made the hot code larger (the Vita's caches hold
// little of the game's ~60 MB of code).
[[gnu::noinline, gnu::cold]] static uint16_t Ps2FastRead16Wrapped(const uint8_t *rdram, uint32_t offset)
{
    uint8_t wrapped[sizeof(uint16_t)];
    for (uint32_t i = 0; i < sizeof(uint16_t); ++i)
        wrapped[i] = rdram[(offset + i) & PS2_RAM_MASK];
    uint16_t value;
    std::memcpy(&value, wrapped, sizeof(value));
    return value;
}

static PS2_FORCE_INLINE uint16_t Ps2FastRead16(const uint8_t *rdram, uint32_t addr)
{
    const uint32_t offset = addr & PS2_RAM_MASK;
    if (__builtin_expect(!Ps2FastRangeIsContiguous(offset, sizeof(uint16_t)), 0))
        return Ps2FastRead16Wrapped(rdram, offset);
    uint16_t value;
    std::memcpy(&value, rdram + offset, sizeof(value));
    return value;
}

// The access that wraps at the end of RAM, out of line: inlined at every
// guest load and store it made the hot code larger (the Vita's caches hold
// little of the game's ~60 MB of code).
[[gnu::noinline, gnu::cold]] static uint32_t Ps2FastRead32Wrapped(const uint8_t *rdram, uint32_t offset)
{
    uint8_t wrapped[sizeof(uint32_t)];
    for (uint32_t i = 0; i < sizeof(uint32_t); ++i)
        wrapped[i] = rdram[(offset + i) & PS2_RAM_MASK];
    uint32_t value;
    std::memcpy(&value, wrapped, sizeof(value));
    return value;
}

static PS2_FORCE_INLINE uint32_t Ps2FastRead32(const uint8_t *rdram, uint32_t addr)
{
    const uint32_t offset = addr & PS2_RAM_MASK;
    if (__builtin_expect(!Ps2FastRangeIsContiguous(offset, sizeof(uint32_t)), 0))
        return Ps2FastRead32Wrapped(rdram, offset);
    uint32_t value;
    std::memcpy(&value, rdram + offset, sizeof(value));
    return value;
}

// The access that wraps at the end of RAM, out of line: inlined at every
// guest load and store it made the hot code larger (the Vita's caches hold
// little of the game's ~60 MB of code).
[[gnu::noinline, gnu::cold]] static uint64_t Ps2FastRead64Wrapped(const uint8_t *rdram, uint32_t offset)
{
    uint8_t wrapped[sizeof(uint64_t)];
    for (uint32_t i = 0; i < sizeof(uint64_t); ++i)
        wrapped[i] = rdram[(offset + i) & PS2_RAM_MASK];
    uint64_t value;
    std::memcpy(&value, wrapped, sizeof(value));
    return value;
}

static PS2_FORCE_INLINE uint64_t Ps2FastRead64(const uint8_t *rdram, uint32_t addr)
{
    const uint32_t offset = addr & PS2_RAM_MASK;
    if (__builtin_expect(!Ps2FastRangeIsContiguous(offset, sizeof(uint64_t)), 0))
        return Ps2FastRead64Wrapped(rdram, offset);
    uint64_t value;
    std::memcpy(&value, rdram + offset, sizeof(value));
    return value;
}

static PS2_FORCE_INLINE __m128i Ps2FastRead128(const uint8_t *rdram, uint32_t addr)
{
    const uint32_t offset = addr & PS2_RAM_MASK;
    if (!Ps2FastRangeIsContiguous(offset, sizeof(__m128i)))
    {
        alignas(16) uint8_t wrapped[sizeof(__m128i)];
        for (uint32_t i = 0; i < sizeof(__m128i); ++i)
        {
            wrapped[i] = rdram[(offset + i) & PS2_RAM_MASK];
        }
        __m128i value;
        std::memcpy(&value, wrapped, sizeof(value));
        return value;
    }

    __m128i value;
    std::memcpy(&value, rdram + offset, sizeof(value));
    return value;
}

static PS2_FORCE_INLINE void Ps2FastWrite8(uint8_t *rdram, uint32_t addr, uint8_t value)
{
    rdram[addr & PS2_RAM_MASK] = value;
}

[[gnu::noinline, gnu::cold]] static void Ps2FastWrite16Wrapped(uint8_t *rdram, uint32_t offset, uint16_t value)
{
    uint8_t wrapped[sizeof(uint16_t)];
    std::memcpy(wrapped, &value, sizeof(value));
    for (uint32_t i = 0; i < sizeof(uint16_t); ++i)
        rdram[(offset + i) & PS2_RAM_MASK] = wrapped[i];
}

static PS2_FORCE_INLINE void Ps2FastWrite16(uint8_t *rdram, uint32_t addr, uint16_t value)
{
    const uint32_t offset = addr & PS2_RAM_MASK;
    if (__builtin_expect(!Ps2FastRangeIsContiguous(offset, sizeof(uint16_t)), 0))
        return Ps2FastWrite16Wrapped(rdram, offset, value);
    std::memcpy(rdram + offset, &value, sizeof(value));
}

[[gnu::noinline, gnu::cold]] static void Ps2FastWrite32Wrapped(uint8_t *rdram, uint32_t offset, uint32_t value)
{
    uint8_t wrapped[sizeof(uint32_t)];
    std::memcpy(wrapped, &value, sizeof(value));
    for (uint32_t i = 0; i < sizeof(uint32_t); ++i)
        rdram[(offset + i) & PS2_RAM_MASK] = wrapped[i];
}

static PS2_FORCE_INLINE void Ps2FastWrite32(uint8_t *rdram, uint32_t addr, uint32_t value)
{
    const uint32_t offset = addr & PS2_RAM_MASK;
    if (__builtin_expect(!Ps2FastRangeIsContiguous(offset, sizeof(uint32_t)), 0))
        return Ps2FastWrite32Wrapped(rdram, offset, value);
    std::memcpy(rdram + offset, &value, sizeof(value));
}

[[gnu::noinline, gnu::cold]] static void Ps2FastWrite64Wrapped(uint8_t *rdram, uint32_t offset, uint64_t value)
{
    uint8_t wrapped[sizeof(uint64_t)];
    std::memcpy(wrapped, &value, sizeof(value));
    for (uint32_t i = 0; i < sizeof(uint64_t); ++i)
        rdram[(offset + i) & PS2_RAM_MASK] = wrapped[i];
}

static PS2_FORCE_INLINE void Ps2FastWrite64(uint8_t *rdram, uint32_t addr, uint64_t value)
{
    const uint32_t offset = addr & PS2_RAM_MASK;
    if (__builtin_expect(!Ps2FastRangeIsContiguous(offset, sizeof(uint64_t)), 0))
        return Ps2FastWrite64Wrapped(rdram, offset, value);
    std::memcpy(rdram + offset, &value, sizeof(value));
}

static PS2_FORCE_INLINE void Ps2FastWrite128(uint8_t *rdram, uint32_t addr, __m128i value)
{
    const uint32_t offset = addr & PS2_RAM_MASK;
    if (!Ps2FastRangeIsContiguous(offset, sizeof(__m128i)))
    {
        alignas(16) uint8_t wrapped[sizeof(__m128i)];
        std::memcpy(wrapped, &value, sizeof(value));
        for (uint32_t i = 0; i < sizeof(__m128i); ++i)
        {
            rdram[(offset + i) & PS2_RAM_MASK] = wrapped[i];
        }
        return;
    }
    std::memcpy(rdram + offset, &value, sizeof(value));
}

#define FAST_READ8(addr) Ps2FastRead8(rdram, (uint32_t)(addr))
#define FAST_READ16(addr) Ps2FastRead16(rdram, (uint32_t)(addr))
#define FAST_READ32(addr) Ps2FastRead32(rdram, (uint32_t)(addr))
#define FAST_READ64(addr) Ps2FastRead64(rdram, (uint32_t)(addr))
#define FAST_READ128(addr) Ps2FastRead128(rdram, (uint32_t)(addr))

#define FAST_WRITE8(addr, val) Ps2FastWrite8(rdram, (uint32_t)(addr), (uint8_t)(val))
#define FAST_WRITE16(addr, val) Ps2FastWrite16(rdram, (uint32_t)(addr), (uint16_t)(val))
#define FAST_WRITE32(addr, val) Ps2FastWrite32(rdram, (uint32_t)(addr), (uint32_t)(val))
#define FAST_WRITE64(addr, val) Ps2FastWrite64(rdram, (uint32_t)(addr), (uint64_t)(val))
#define FAST_WRITE128(addr, val) Ps2FastWrite128(rdram, (uint32_t)(addr), (val))

#if defined(__arm__) && !defined(__aarch64__)
// 32-bit ARM (the Vita): a guest load or store is one compare and the access
// inline; everything that is not plain RAM below 32 MB (mirrors, scratchpad,
// hardware registers) goes through one shared out-of-line helper with the
// exact old behaviour. The inline special-address chain, the wrap check and a
// 4-argument call at every access made the code ~19x the size of the MIPS
// (the Vita's caches then miss on most of it). RAM carries 64 bytes of slack
// past 32 MB for an unaligned access at the very end.
template <typename T>
[[gnu::noinline, gnu::cold]] T ps2SlowRead(uint8_t *rdram, R5900Context *ctx, PS2Runtime *runtime, uint32_t addr);
template <typename T>
[[gnu::noinline, gnu::cold]] void ps2SlowWrite(uint8_t *rdram, R5900Context *ctx, PS2Runtime *runtime, uint32_t addr, T value);
template <typename T>
static PS2_FORCE_INLINE T ps2Read(uint8_t *rdram, R5900Context *ctx, PS2Runtime *runtime, uint32_t addr)
{
    if (__builtin_expect(addr < PS2_RAM_SIZE, 1))
    {
        T v;
        std::memcpy(&v, rdram + addr, sizeof(T));
        return v;
    }
    return ps2SlowRead<T>(rdram, ctx, runtime, addr);
}
template <typename T>
static PS2_FORCE_INLINE void ps2Write(uint8_t *rdram, R5900Context *ctx, PS2Runtime *runtime, uint32_t addr, T value)
{
    if (__builtin_expect(addr < PS2_RAM_SIZE, 1))
        std::memcpy(rdram + addr, &value, sizeof(T));
    else
        ps2SlowWrite<T>(rdram, ctx, runtime, addr, value);
}
#define READ8(addr) ps2Read<uint8_t>(rdram, ctx, runtime, (uint32_t)(addr))
#define READ16(addr) ps2Read<uint16_t>(rdram, ctx, runtime, (uint32_t)(addr))
#define READ32(addr) ps2Read<uint32_t>(rdram, ctx, runtime, (uint32_t)(addr))
#define READ64(addr) ps2Read<uint64_t>(rdram, ctx, runtime, (uint32_t)(addr))
#define READ128(addr) ps2Read<__m128i>(rdram, ctx, runtime, (uint32_t)(addr) & ~0xFu)
#define WRITE8(addr, val) ps2Write<uint8_t>(rdram, ctx, runtime, (uint32_t)(addr), (uint8_t)(val))
#define WRITE16(addr, val) ps2Write<uint16_t>(rdram, ctx, runtime, (uint32_t)(addr), (uint16_t)(val))
#define WRITE32(addr, val) ps2Write<uint32_t>(rdram, ctx, runtime, (uint32_t)(addr), (uint32_t)(val))
#define WRITE64(addr, val) ps2Write<uint64_t>(rdram, ctx, runtime, (uint32_t)(addr), (uint64_t)(val))
#define WRITE128(addr, val) ps2Write<__m128i>(rdram, ctx, runtime, (uint32_t)(addr) & ~0xFu, (val))
#else
#define READ8(addr) ([&]() -> uint8_t {                       \
    uint32_t _addr = (uint32_t)(addr);                        \
    return PS2Runtime::isSpecialAddress(_addr)                \
        ? runtime->Load8(rdram, ctx, _addr)                   \
        : FAST_READ8(_addr); }())

#define READ16(addr) ([&]() -> uint16_t {                     \
    uint32_t _addr = (uint32_t)(addr);                        \
    return PS2Runtime::isSpecialAddress(_addr)                \
        ? runtime->Load16(rdram, ctx, _addr)                  \
        : FAST_READ16(_addr); }())

#define READ32(addr) ([&]() -> uint32_t {                     \
    uint32_t _addr = (uint32_t)(addr);                        \
    return PS2Runtime::isSpecialAddress(_addr)                \
        ? runtime->Load32(rdram, ctx, _addr)                  \
        : FAST_READ32(_addr); }())

#define READ64(addr) ([&]() -> uint64_t {                     \
    uint32_t _addr = (uint32_t)(addr);                        \
    return PS2Runtime::isSpecialAddress(_addr)                \
        ? runtime->Load64(rdram, ctx, _addr)                  \
        : FAST_READ64(_addr); }())

#define READ128(addr) ([&]() -> __m128i {                     \
    uint32_t _addr = (uint32_t)(addr) & ~0xFu;                \
    return PS2Runtime::isSpecialAddress(_addr)                \
        ? runtime->Load128(rdram, ctx, _addr)                 \
        : FAST_READ128(_addr); }())

#define WRITE8(addr, val)                                                            \
    do                                                                               \
    {                                                                                \
        uint32_t _addr = (addr);                                                     \
        if (PS2Runtime::isSpecialAddress(_addr))                                     \
            runtime->Store8(rdram, ctx, _addr, (val));                               \
        else                                                                         \
        {                                                                            \
            ps2TraceGuestWrite(rdram, _addr, 1u, (uint8_t)(val), 0u, "WRITE8", ctx); \
            FAST_WRITE8(_addr, (val));                                               \
        }                                                                            \
    } while (0)

#define WRITE16(addr, val)                                                             \
    do                                                                                 \
    {                                                                                  \
        uint32_t _addr = (addr);                                                       \
        if (PS2Runtime::isSpecialAddress(_addr))                                       \
            runtime->Store16(rdram, ctx, _addr, (val));                                \
        else                                                                           \
        {                                                                              \
            ps2TraceGuestWrite(rdram, _addr, 2u, (uint16_t)(val), 0u, "WRITE16", ctx); \
            FAST_WRITE16(_addr, (val));                                                \
        }                                                                              \
    } while (0)

#define WRITE32(addr, val)                                                             \
    do                                                                                 \
    {                                                                                  \
        uint32_t _addr = (addr);                                                       \
        if (PS2Runtime::isSpecialAddress(_addr))                                       \
            runtime->Store32(rdram, ctx, _addr, (val));                                \
        else                                                                           \
        {                                                                              \
            ps2TraceGuestWrite(rdram, _addr, 4u, (uint32_t)(val), 0u, "WRITE32", ctx); \
            FAST_WRITE32(_addr, (val));                                                \
        }                                                                              \
    } while (0)

#define WRITE64(addr, val)                                                             \
    do                                                                                 \
    {                                                                                  \
        uint32_t _addr = (addr);                                                       \
        if (PS2Runtime::isSpecialAddress(_addr))                                       \
            runtime->Store64(rdram, ctx, _addr, (val));                                \
        else                                                                           \
        {                                                                              \
            ps2TraceGuestWrite(rdram, _addr, 8u, (uint64_t)(val), 0u, "WRITE64", ctx); \
            FAST_WRITE64(_addr, (val));                                                \
        }                                                                              \
    } while (0)

#define WRITE128(addr, val)                                                          \
    do                                                                               \
    {                                                                                \
        uint32_t _addr = (uint32_t)(addr) & ~0xFu;                                   \
        __m128i _value = (val);                                                      \
        if (PS2Runtime::isSpecialAddress(_addr))                                     \
            runtime->Store128(rdram, ctx, _addr, _value);                            \
        else                                                                         \
        {                                                                            \
            const uint64_t _lo = static_cast<uint64_t>(PS2_EXTRACT_EPI64_0(_value)); \
            const uint64_t _hi = static_cast<uint64_t>(PS2_EXTRACT_EPI64_1(_value)); \
            ps2TraceGuestWrite(rdram, _addr, 16u, _lo, _hi, "WRITE128", ctx);        \
            FAST_WRITE128(_addr, _value);                                            \
        }                                                                            \
    } while (0)

#endif

// Packed Compare Greater Than (PCGT)
#define PS2_PCGTW(a, b) _mm_cmpgt_epi32((__m128i)(a), (__m128i)(b))
#define PS2_PCGTH(a, b) _mm_cmpgt_epi16((__m128i)(a), (__m128i)(b))
#define PS2_PCGTB(a, b) _mm_cmpgt_epi8((__m128i)(a), (__m128i)(b))

// Packed Add with Signed Saturation Word (PADDSW)
inline __m128i ps2_paddsw(__m128i a, __m128i b)
{

    __m128i sum = _mm_add_epi32(a, b);
    // Check for over/underflow. Clamp to either INT32_MIN/INT32_MAX.
    __m128i overflow = _mm_and_si128(_mm_xor_si128(a, sum),
                                     _mm_xor_si128(b, sum));
    // Extract input sign.
    overflow = _mm_srai_epi32(overflow, 31);
    __m128i input_sign = _mm_srai_epi32(a, 31);
    // Select saturation value based on overflow sign.
    #if defined(__SSE4_1__)
    __m128i sat = _mm_blendv_epi8(
        _mm_set1_epi32(INT32_MAX),
        _mm_set1_epi32(INT32_MIN),
        input_sign);
    return _mm_blendv_epi8(sum, sat, overflow);
    #else
    __m128i sat = _mm_or_si128(_mm_and_si128(input_sign, _mm_set1_epi32(INT32_MIN)),
                               _mm_andnot_si128(input_sign, _mm_set1_epi32(INT32_MAX)));
    return _mm_or_si128(_mm_and_si128(overflow, sat),
                        _mm_andnot_si128(overflow, sum));
    #endif
}
#define PS2_PADDSW(a, b) ps2_paddsw((__m128i)(a), (__m128i)(b))

// Packed Subtract with Signed Saturation Word (PSUBSW)
inline __m128i ps2_psubsw(__m128i a, __m128i b)
{
    __m128i diff = _mm_sub_epi32(a, b);
    // Check for over/underflow. Clamp to either INT32_MIN/INT32_MAX.
    __m128i overflow = _mm_and_si128(_mm_xor_si128(a, b),
                                     _mm_xor_si128(a, diff));
    // Extract input sign.
    overflow = _mm_srai_epi32(overflow, 31);
    __m128i input_sign = _mm_srai_epi32(a, 31);
    // Select saturation value based on overflow sign.
    #if defined(__SSE4_1__)
    __m128i sat = _mm_blendv_epi8(
        _mm_set1_epi32(INT32_MAX),
        _mm_set1_epi32(INT32_MIN),
        input_sign);
    return _mm_blendv_epi8(diff, sat, overflow);
    #else
    __m128i sat = _mm_or_si128(_mm_and_si128(input_sign, _mm_set1_epi32(INT32_MIN)),
                               _mm_andnot_si128(input_sign, _mm_set1_epi32(INT32_MAX)));
    return _mm_or_si128(_mm_and_si128(overflow, sat),
                        _mm_andnot_si128(overflow, diff));
    #endif

}
#define PS2_PSUBSW(a, b) ps2_psubsw((__m128i)(a), (__m128i)(b))

// Packed Compare Equal (PCEQ)
#define PS2_PCEQW(a, b) _mm_cmpeq_epi32((__m128i)(a), (__m128i)(b))
#define PS2_PCEQH(a, b) _mm_cmpeq_epi16((__m128i)(a), (__m128i)(b))
#define PS2_PCEQB(a, b) _mm_cmpeq_epi8((__m128i)(a), (__m128i)(b))

// Packed Absolute (PABS)
#define PS2_PABSW(a) _mm_abs_epi32((__m128i)(a))
#define PS2_PABSH(a) _mm_abs_epi16((__m128i)(a))
#define PS2_PABSB(a) _mm_abs_epi8((__m128i)(a))

// Packed Pack (PPAC) - Packs larger elements into smaller ones
inline __m128i ps2_paddu32(__m128i a, __m128i b)
{
    __m128i sum = _mm_add_epi32(a, b);
    __m128i overflow = _mm_cmpgt_epi32(_mm_xor_si128(a, _mm_set1_epi32(INT32_MIN)),
                                       _mm_xor_si128(sum, _mm_set1_epi32(INT32_MIN)));
    return _mm_or_si128(sum, overflow); // overflow lanes become all-1s
}
inline __m128i ps2_psubu32(__m128i a, __m128i b)
{
    __m128i diff = _mm_sub_epi32(a, b);
    // Underflow if a < b (unsigned). Clamp to 0.
    __m128i underflow = _mm_cmpgt_epi32(_mm_xor_si128(b, _mm_set1_epi32(INT32_MIN)),
                                        _mm_xor_si128(a, _mm_set1_epi32(INT32_MIN)));
    return _mm_andnot_si128(underflow, diff); // underflow lanes become 0
}

inline __m128i ps2_ppacw(__m128i rs, __m128i rt)
{
    // rs = [rs3 rs2 rs1 rs0], rt = [rt3 rt2 rt1 rt0]
    return _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(rt), _mm_castsi128_ps(rs), _MM_SHUFFLE(2, 0, 2, 0)));
}
#define PS2_PPACW(a, b) ps2_ppacw((__m128i)(a), (__m128i)(b))

inline __m128i ps2_ppach(__m128i rs, __m128i rt)
{
    const __m128i mask = _mm_setr_epi8(
        0, 1, 4, 5, 8, 9, 12, 13,  // from rt: halfwords 0,2,4,6
        0, 1, 4, 5, 8, 9, 12, 13); // from rs: halfwords 0,2,4,6
    __m128i lo = _mm_shuffle_epi8(rt, mask);
    __m128i hi = _mm_shuffle_epi8(rs, mask);
    return _mm_unpacklo_epi64(lo, hi);
}
#define PS2_PPACH(a, b) ps2_ppach((__m128i)(a), (__m128i)(b))

inline __m128i ps2_ppacb(__m128i rs, __m128i rt)
{
    const __m128i mask = _mm_setr_epi8(
        0, 2, 4, 6, 8, 10, 12, 14,  // from rt: bytes 0,2,4,6,8,10,12,14
        0, 2, 4, 6, 8, 10, 12, 14); // from rs
    __m128i lo = _mm_shuffle_epi8(rt, mask);
    __m128i hi = _mm_shuffle_epi8(rs, mask);
    return _mm_unpacklo_epi64(lo, hi);
}
#define PS2_PPACB(a, b) ps2_ppacb((__m128i)(a), (__m128i)(b))

// Packed Interleave (PINT)
#define PS2_PINTH(a, b) _mm_unpacklo_epi16(_mm_shuffle_epi32((__m128i)(b), _MM_SHUFFLE(3, 2, 1, 0)), _mm_shuffle_epi32((__m128i)(a), _MM_SHUFFLE(3, 2, 1, 0)))
#define PS2_PINTEH(a, b) _mm_unpackhi_epi16(_mm_shuffle_epi32((__m128i)(b), _MM_SHUFFLE(3, 2, 1, 0)), _mm_shuffle_epi32((__m128i)(a), _MM_SHUFFLE(3, 2, 1, 0)))

// Packed Multiply-Add (PMADD)
#define PS2_PMADDW(a, b) _mm_add_epi32(_mm_mullo_epi32(_mm_shuffle_epi32((__m128i)(a), _MM_SHUFFLE(1, 0, 3, 2)), _mm_shuffle_epi32((__m128i)(b), _MM_SHUFFLE(1, 0, 3, 2))), _mm_mullo_epi32(_mm_shuffle_epi32((__m128i)(a), _MM_SHUFFLE(3, 2, 1, 0)), _mm_shuffle_epi32((__m128i)(b), _MM_SHUFFLE(3, 2, 1, 0))))

// Packed Variable Shifts
#define PS2_PSLLVW(a, b) _mm_custom_sllv_epi32((__m128i)(a), (__m128i)(b))
#define PS2_PSRLVW(a, b) _mm_custom_srlv_epi32((__m128i)(a), (__m128i)(b))
#define PS2_PSRAVW(a, b) _mm_custom_srav_epi32((__m128i)(a), (__m128i)(b))

inline __m128i _mm_custom_sllv_epi32(__m128i a, __m128i count)
{
    alignas(16) int32_t a_arr[4];
    alignas(16) int32_t count_arr[4];
    alignas(16) int32_t result[4];

    std::memcpy(a_arr, &a, sizeof(a));
    std::memcpy(count_arr, &count, sizeof(count));

    for (int i = 0; i < 4; i++)
    {
        result[i] = a_arr[i] << (count_arr[i] & 0x1F);
    }

    __m128i out;
    std::memcpy(&out, result, sizeof(out));
    return out;
}

inline __m128i _mm_custom_srlv_epi32(__m128i a, __m128i count)
{
    int32_t a_arr[4], count_arr[4], result[4];
    _mm_storeu_si128((__m128i *)a_arr, a);
    _mm_storeu_si128((__m128i *)count_arr, count);
    for (int i = 0; i < 4; i++)
    {
        result[i] = (uint32_t)a_arr[i] >> (count_arr[i] & 0x1F);
    }
    return _mm_loadu_si128((__m128i *)result);
}

inline __m128i _mm_custom_srav_epi32(__m128i a, __m128i count)
{
    int32_t a_arr[4], count_arr[4], result[4];
    _mm_storeu_si128((__m128i *)a_arr, a);
    _mm_storeu_si128((__m128i *)count_arr, count);
    for (int i = 0; i < 4; i++)
    {
        result[i] = a_arr[i] >> (count_arr[i] & 0x1F);
    }
    return _mm_loadu_si128((__m128i *)result);
}

// PMFHL function implementations
inline __m128i ps2_u64_to_epi64_pair(uint64_t value)
{
    return _mm_set1_epi64x(static_cast<long long>(value));
}

#define PS2_PMFHL_LW(hi, lo) _mm_unpacklo_epi64(ps2_u64_to_epi64_pair(lo), ps2_u64_to_epi64_pair(hi))
#define PS2_PMFHL_UW(hi, lo) _mm_unpackhi_epi64(ps2_u64_to_epi64_pair(lo), ps2_u64_to_epi64_pair(hi))
#define PS2_PMFHL_SLW(hi, lo) _mm_packs_epi32(ps2_u64_to_epi64_pair(lo), ps2_u64_to_epi64_pair(hi))
#define PS2_PMFHL_LH(hi, lo) _mm_shuffle_epi32(_mm_packs_epi32(ps2_u64_to_epi64_pair(lo), ps2_u64_to_epi64_pair(hi)), _MM_SHUFFLE(3, 1, 2, 0))
#define PS2_PMFHL_SH(hi, lo) _mm_shufflehi_epi16(_mm_shufflelo_epi16(_mm_packs_epi32(ps2_u64_to_epi64_pair(lo), ps2_u64_to_epi64_pair(hi)), _MM_SHUFFLE(3, 1, 2, 0)), _MM_SHUFFLE(3, 1, 2, 0))

// EE FPU semantics (ported from the sotc-pc fork of PS2Recomp,
// github.com/TaylorNAlbarnaz/PS2Recomp, GPL-3.0): no Inf/NaN (results clamp to
// +-max), operand alignment like the PS2 adder, sqrt(|x|), saturating
// conversion. The game thread runs with round-toward-zero and denormals
// flushed (ps2SetEeFloatingPointMode); DIV/SQRT/RSQRT round to nearest.
static PS2_FORCE_INLINE float ps2FpuClamp(float value)
{
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    if ((bits & 0x7F800000u) == 0x7F800000u)
    {
        bits = (bits & 0x80000000u) | 0x7F7FFFFFu;
        std::memcpy(&value, &bits, sizeof(value));
    }
    return value;
}

static PS2_FORCE_INLINE void ps2FpuAlignOperands(float &a, float &b)
{
    uint32_t ua;
    uint32_t ub;
    std::memcpy(&ua, &a, sizeof(ua));
    std::memcpy(&ub, &b, sizeof(ub));
    const int32_t diff = static_cast<int32_t>((ua >> 23) & 0xFFu) - static_cast<int32_t>((ub >> 23) & 0xFFu);
    if (diff >= 25)
        ub &= 0x80000000u;
    else if (diff > 0)
        ub &= 0xFFFFFFFFu << (diff - 1);
    else if (diff <= -25)
        ua &= 0x80000000u;
    else if (diff < 0)
        ua &= 0xFFFFFFFFu << (-diff - 1);
    std::memcpy(&a, &ua, sizeof(a));
    std::memcpy(&b, &ub, sizeof(b));
}

#if defined(__arm__) && !defined(__aarch64__)
// 32-bit ARM (the Vita): the exact add, sub and mul are ~20 instructions
// each with the value moved between the float and core registers: one
// shared copy out of line keeps the generated code small (its size, not its
// instruction count, is what the Vita's caches pay for).
#define PS2_FPU_HELPER [[gnu::noinline]] inline
#else
#define PS2_FPU_HELPER static PS2_FORCE_INLINE
#endif
PS2_FPU_HELPER float ps2FpuAdd(float a, float b)
{
    ps2FpuAlignOperands(a, b);
    return ps2FpuClamp(a + b);
}

PS2_FPU_HELPER float ps2FpuSub(float a, float b)
{
    ps2FpuAlignOperands(a, b);
    return ps2FpuClamp(a - b);
}

PS2_FPU_HELPER float ps2FpuMul(float a, float b)
{
    return ps2FpuClamp(a * b);
}

static PS2_FORCE_INLINE float ps2SignedFloatMax(uint32_t signSource)
{
    const uint32_t bits = (signSource & 0x80000000u) | 0x7F7FFFFFu;
    float out;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

static PS2_FORCE_INLINE uint32_t ps2FloatBits(float value)
{
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

static PS2_FORCE_INLINE uint32_t ps2FpuCompareKey(float value)
{
    uint32_t bits = ps2FloatBits(value);
    if ((bits & 0x7F800000u) == 0u)
        bits = 0u;
    return (bits & 0x80000000u) != 0u ? ~bits : bits ^ 0x80000000u;
}

static PS2_FORCE_INLINE bool ps2FloatIsZero(float value)
{
    return (ps2FloatBits(value) & 0x7F800000u) == 0u;
}

static constexpr uint32_t PS2_FCR31_I = 0x00020000u;
static constexpr uint32_t PS2_FCR31_D = 0x00010000u;
static constexpr uint32_t PS2_FCR31_SI = 0x00000040u;
static constexpr uint32_t PS2_FCR31_SD = 0x00000020u;

static PS2_FORCE_INLINE float ps2FpuDivideNearest(float dividend, float divisor)
{
    const uint32_t csr = _mm_getcsr();
    _mm_setcsr(csr & ~0x6000u);
    const float result = _mm_cvtss_f32(_mm_div_ss(_mm_set_ss(dividend), _mm_set_ss(divisor)));
    _mm_setcsr(csr);
    return result;
}

static PS2_FORCE_INLINE float ps2FpuSqrtNearest(float value)
{
    const uint32_t csr = _mm_getcsr();
    _mm_setcsr(csr & ~0x6000u);
    const float result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(value)));
    _mm_setcsr(csr);
    return result;
}

static PS2_FORCE_INLINE float ps2FpuDivide(uint32_t &fcr31, float dividend, float divisor)
{
    if (ps2FloatIsZero(divisor))
    {
        if (ps2FloatIsZero(dividend))
            fcr31 |= PS2_FCR31_I | PS2_FCR31_SI;
        else
            fcr31 |= PS2_FCR31_D | PS2_FCR31_SD;
        return ps2SignedFloatMax(ps2FloatBits(dividend) ^ ps2FloatBits(divisor));
    }
    return ps2FpuClamp(ps2FpuDivideNearest(dividend, divisor));
}

static PS2_FORCE_INLINE float ps2FpuSqrt(uint32_t &fcr31, float value)
{
    if ((ps2FloatBits(value) & 0x80000000u) != 0u && !ps2FloatIsZero(value))
        fcr31 |= PS2_FCR31_I | PS2_FCR31_SI;
    return ps2FpuClamp(ps2FpuSqrtNearest(std::fabs(value)));
}

static PS2_FORCE_INLINE float ps2FpuRsqrt(uint32_t &fcr31, float numerator, float value)
{
    if (ps2FloatIsZero(value))
    {
        fcr31 |= PS2_FCR31_D | PS2_FCR31_SD;
        return ps2SignedFloatMax(ps2FloatBits(numerator) ^ ps2FloatBits(value));
    }
    if ((ps2FloatBits(value) & 0x80000000u) != 0u)
        fcr31 |= PS2_FCR31_I | PS2_FCR31_SI;
    return ps2FpuClamp(ps2FpuDivideNearest(numerator, ps2FpuSqrtNearest(std::fabs(value))));
}

static PS2_FORCE_INLINE int32_t ps2FpuConvertToWord(float value)
{
    const uint32_t bits = ps2FloatBits(value);
    if ((bits & 0x7F800000u) >= 0x4F000000u)
        return (bits & 0x80000000u) ? INT32_MIN : INT32_MAX;
    return static_cast<int32_t>(value);
}

static constexpr uint16_t PS2_VU_STATUS_I = 0x0010u;
static constexpr uint16_t PS2_VU_STATUS_D = 0x0020u;
static constexpr uint16_t PS2_VU_STATUS_IS = 0x0400u;
static constexpr uint16_t PS2_VU_STATUS_DS = 0x0800u;

static PS2_FORCE_INLINE float ps2VuDivide(uint16_t &status, float dividend, float divisor)
{
    status &= static_cast<uint16_t>(~(PS2_VU_STATUS_I | PS2_VU_STATUS_D));
    if (ps2FloatIsZero(divisor))
    {
        if (ps2FloatIsZero(dividend))
            status |= PS2_VU_STATUS_I | PS2_VU_STATUS_IS;
        else
            status |= PS2_VU_STATUS_D | PS2_VU_STATUS_DS;
        return ps2SignedFloatMax(ps2FloatBits(dividend) ^ ps2FloatBits(divisor));
    }
    return ps2FpuClamp(dividend / divisor);
}

static PS2_FORCE_INLINE float ps2VuSqrt(uint16_t &status, float value)
{
    status &= static_cast<uint16_t>(~(PS2_VU_STATUS_I | PS2_VU_STATUS_D));
    if ((ps2FloatBits(value) & 0x80000000u) != 0u && !ps2FloatIsZero(value))
        status |= PS2_VU_STATUS_I | PS2_VU_STATUS_IS;
    return ps2FpuClamp(std::sqrt(std::fabs(value)));
}

static PS2_FORCE_INLINE float ps2VuRsqrt(uint16_t &status, float numerator, float value)
{
    status &= static_cast<uint16_t>(~(PS2_VU_STATUS_I | PS2_VU_STATUS_D));
    if (ps2FloatIsZero(value))
    {
        if (ps2FloatIsZero(numerator))
            status |= PS2_VU_STATUS_I | PS2_VU_STATUS_IS;
        else
            status |= PS2_VU_STATUS_D | PS2_VU_STATUS_DS;
        return ps2SignedFloatMax(ps2FloatBits(numerator) ^ ps2FloatBits(value));
    }
    if ((ps2FloatBits(value) & 0x80000000u) != 0u)
        status |= PS2_VU_STATUS_I | PS2_VU_STATUS_IS;
    return ps2FpuClamp(numerator / std::sqrt(std::fabs(value)));
}

static PS2_FORCE_INLINE void ps2SetEeFloatingPointMode()
{
    _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
}

// FPU (COP1) operations
#define FPU_SET_ACC(ctx, res) (ctx->f_acc = res)
#define FPU_ADD_S(a, b) ps2FpuAdd((float)(a), (float)(b))
#define FPU_SUB_S(a, b) ps2FpuSub((float)(a), (float)(b))
#define FPU_MUL_S(a, b) ps2FpuMul((float)(a), (float)(b))
#define FPU_DIV_S(a, b) ps2FpuClamp((float)(a) / (float)(b))
#define FPU_SQRT_S(a) ps2FpuClamp(std::sqrt(std::fabs((float)(a))))
#define FPU_ABS_S(a) fabsf((float)(a))
#define FPU_MOV_S(a) ((float)(a))
#define FPU_NEG_S(a) (-(float)(a))
#define FPU_ROUND_L_S(a) ((int64_t)roundf((float)(a)))
#define FPU_TRUNC_L_S(a) ((int64_t)(float)(a))
#define FPU_CEIL_L_S(a) ((int64_t)ceilf((float)(a)))
#define FPU_FLOOR_L_S(a) ((int64_t)floorf((float)(a)))
#define FPU_ROUND_W_S(a) ((int32_t)nearbyintf((float)(a)))
#define FPU_TRUNC_W_S(a) ((int32_t)(float)(a))
#define FPU_CEIL_W_S(a) ((int32_t)ceilf((float)(a)))
#define FPU_FLOOR_W_S(a) ((int32_t)floorf((float)(a)))
#define FPU_CVT_S_W(a) ((float)(int32_t)(a))
#define FPU_CVT_S_L(a) ((float)(int64_t)(a))
#define FPU_CVT_W_S(a) ps2FpuConvertToWord((float)(a))
#define FPU_CVT_L_S(a) ((int64_t)(float)(a))
#define FPU_C_F_S(a, b) (0)
#define FPU_C_UN_S(a, b) (0)
#define FPU_C_EQ_S(a, b) (ps2FpuCompareKey((float)(a)) == ps2FpuCompareKey((float)(b)))
#define FPU_C_UEQ_S(a, b) FPU_C_EQ_S(a, b)
#define FPU_C_OLT_S(a, b) (ps2FpuCompareKey((float)(a)) < ps2FpuCompareKey((float)(b)))
#define FPU_C_ULT_S(a, b) FPU_C_OLT_S(a, b)
#define FPU_C_OLE_S(a, b) (ps2FpuCompareKey((float)(a)) <= ps2FpuCompareKey((float)(b)))
#define FPU_C_ULE_S(a, b) FPU_C_OLE_S(a, b)
#define FPU_C_SF_S(a, b) (0)
#define FPU_C_NGLE_S(a, b) (0)
#define FPU_C_SEQ_S(a, b) FPU_C_EQ_S(a, b)
#define FPU_C_NGL_S(a, b) FPU_C_EQ_S(a, b)
#define FPU_C_LT_S(a, b) FPU_C_OLT_S(a, b)
#define FPU_C_NGE_S(a, b) FPU_C_OLT_S(a, b)
#define FPU_C_LE_S(a, b) FPU_C_OLE_S(a, b)
#define FPU_C_NGT_S(a, b) FPU_C_OLE_S(a, b)

// QFSRV: Quadword Funnel Shift Right Variable
// Concatenates rs || rt (256 bits) and right-shifts by SA bits, taking lower 128 bits.
inline __m128i ps2_qfsrv(__m128i rs, __m128i rt, uint32_t sa)
{
    if (sa == 0)
        return rt;
    if (sa >= 128)
    {
        if (sa >= 256)
            return _mm_setzero_si128();
        uint32_t shift = sa - 128;
        if (shift == 0)
            return rs;
        // Shift rs right by (sa-128) bits
        uint32_t byteShift = shift / 8;
        uint32_t bitShift = shift % 8;
        // Byte shift rs right
        alignas(16) uint8_t buf[16] = {};
        alignas(16) uint8_t src[16];
        _mm_store_si128((__m128i *)src, rs);
        for (uint32_t i = 0; i + byteShift < 16; i++)
            buf[i] = src[i + byteShift];
        __m128i result = _mm_load_si128((__m128i *)buf);
        if (bitShift > 0)
            result = _mm_or_si128(_mm_srli_epi64(result, bitShift),
                                  _mm_slli_epi64(_mm_bsrli_si128(result, 8), 64 - bitShift));
        return result;
    }
    // sa is 1..127: result = (rs || rt) >> sa, lower 128 bits
    uint32_t byteShift = sa / 8;
    uint32_t bitShift = sa % 8;
    alignas(16) uint8_t combined[32];
    _mm_store_si128((__m128i *)(combined), rt);      // low 128 bits
    _mm_store_si128((__m128i *)(combined + 16), rs); // high 128 bits
    // Shift right by byteShift bytes
    alignas(16) uint8_t shifted[16];
    for (uint32_t i = 0; i < 16; i++)
        shifted[i] = (i + byteShift < 32) ? combined[i + byteShift] : 0;
    __m128i result = _mm_load_si128((__m128i *)shifted);
    if (bitShift > 0)
    {
        uint8_t extra = (byteShift + 16 < 32) ? combined[byteShift + 16] : 0;
        __m128i hi_byte = _mm_insert_epi8(_mm_setzero_si128(), extra, 15);
        alignas(16) uint8_t src32[32];
        for (uint32_t i = 0; i < 32; i++)
            src32[i] = combined[i];
        uint64_t lo0, lo1, hi0, hi1;
        std::memcpy(&lo0, src32, 8);
        std::memcpy(&lo1, src32 + 8, 8);
        std::memcpy(&hi0, src32 + 16, 8);
        std::memcpy(&hi1, src32 + 24, 8);
        // 256-bit right shift by sa bits
        uint64_t r0, r1;
        if (sa < 64)
        {
            r0 = (lo0 >> sa) | (lo1 << (64 - sa));
            r1 = (lo1 >> sa) | (hi0 << (64 - sa));
        }
        else if (sa < 128)
        {
            uint32_t s = sa - 64;
            if (s == 0)
            {
                r0 = lo1;
                r1 = hi0;
            }
            else
            {
                r0 = (lo1 >> s) | (hi0 << (64 - s));
                r1 = (hi0 >> s) | (hi1 << (64 - s));
            }
        }
        else
        {
            r0 = 0;
            r1 = 0; // handled above
        }
        result = _mm_set_epi64x((long long)r1, (long long)r0);
    }
    return result;
}
#define PS2_QFSRV(rs, rt, sa) ps2_qfsrv((__m128i)(rs), (__m128i)(rt), (uint32_t)(sa))
#define PS2_PCPYLD(rs, rt) _mm_unpacklo_epi64(rt, rs)
#define PS2_PEXEH(rs) _mm_shufflelo_epi16(_mm_shufflehi_epi16(rs, _MM_SHUFFLE(2, 3, 0, 1)), _MM_SHUFFLE(2, 3, 0, 1))
#define PS2_PEXEW(rs) _mm_shuffle_epi32(rs, _MM_SHUFFLE(2, 3, 0, 1))
#define PS2_PROT3W(rs) _mm_shuffle_epi32(rs, _MM_SHUFFLE(0, 3, 2, 1))

// Additional VU0 operations
#define PS2_VSQRT(x) sqrtf(x)
#define PS2_VRSQRT(x) (1.0f / sqrtf(x))

// VU DIV / SQRT / RSQRT into Q, including the divide-by-zero results
// (signed FLT_MAX) and SQRT/RSQRT taking the magnitude of a negative input.
static PS2_FORCE_INLINE float ps2VuDivQ(float fs, float ft)
{
    fs = ps2VuNormalizeScalar(fs);
    ft = ps2VuNormalizeScalar(ft);
    if (ft == 0.0f)
    {
        uint32_t a, b;
        std::memcpy(&a, &fs, 4);
        std::memcpy(&b, &ft, 4);
        const uint32_t r = ((a ^ b) & 0x80000000u) | 0x7F7FFFFFu;
        float out;
        std::memcpy(&out, &r, 4);
        return out;
    }
    return ps2VuNormalizeScalar(fs / ft);
}
static PS2_FORCE_INLINE float ps2VuSqrtQ(float ft)
{
    return ps2VuNormalizeScalar(sqrtf(std::fabs(ps2VuNormalizeScalar(ft))));
}
static PS2_FORCE_INLINE float ps2VuRsqrtQ(float fs, float ft)
{
    fs = ps2VuNormalizeScalar(fs);
    ft = ps2VuNormalizeScalar(ft);
    if (ft == 0.0f)
    {
        uint32_t a, b;
        std::memcpy(&a, &fs, 4);
        std::memcpy(&b, &ft, 4);
        const uint32_t r = ((a ^ b) & 0x80000000u) | 0x7F7FFFFFu;
        float out;
        std::memcpy(&out, &r, 4);
        return out;
    }
    return ps2VuNormalizeScalar(fs / sqrtf(std::fabs(ft)));
}

// Lean generated code (CELOSIA_LEAN_GPRS at recompile time) keeps a
// function's guest registers in locals and moves them to and from the
// context only around calls and returns: the low 64 bits, as plain integers
// on every platform (the upper 64 bits are only ever MMI's, which stay in the
// context).
static PS2_FORCE_INLINE uint64_t Ps2LoadGpr64(const R5900Context *ctx, int reg)
{
    uint64_t v;
    std::memcpy(&v, &ctx->r[reg], sizeof(v));
    return v;
}
static PS2_FORCE_INLINE void Ps2StoreGpr64(R5900Context *ctx, int reg, uint64_t v)
{
    std::memcpy(&ctx->r[reg], &v, sizeof(v));
}
// CELOSIA_COUNT_INSTRS=1 at recompile time: every guest instruction counts
// here (measuring how much MIPS code a frame runs; one thread).
inline uint64_t g_ps2GuestInstructions = 0;
#define PS2_COUNT_INSTRUCTIONS(n) (g_ps2GuestInstructions += (n))
#define PS2_SX32(v) static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(v)))
#define PS2_ZX32(v) static_cast<uint64_t>(static_cast<uint32_t>(v))

#if defined(__arm__) && !defined(__aarch64__)
// 32-bit ARM (the Vita): a GPR's low 64 bits read and written as plain
// integers. Through the SSE emulation every access moved the value between
// the NEON and the core registers, which stalls a Cortex-A9 each time (most
// of the recompiled code is GPR traffic). Same results: the upper 64 bits
// (MMI) stay as they are.
static PS2_FORCE_INLINE uint32_t Ps2GprLow32(const R5900Context *ctx, int reg)
{
    uint32_t v;
    std::memcpy(&v, &ctx->r[reg], sizeof(v));
    return v;
}
static PS2_FORCE_INLINE uint64_t Ps2GprLow64(const R5900Context *ctx, int reg)
{
    uint64_t v;
    std::memcpy(&v, &ctx->r[reg], sizeof(v));
    return v;
}
static PS2_FORCE_INLINE void Ps2SetGprLow64Scalar(R5900Context *ctx, int reg, uint64_t v)
{
    if (reg != 0)
        std::memcpy(&ctx->r[reg], &v, sizeof(v));
}
#define GPR_U32(ctx_ptr, reg_idx) ((reg_idx == 0) ? 0U : Ps2GprLow32(ctx_ptr, reg_idx))
#define GPR_S32(ctx_ptr, reg_idx) ((reg_idx == 0) ? 0 : static_cast<int32_t>(Ps2GprLow32(ctx_ptr, reg_idx)))
#define GPR_U64(ctx_ptr, reg_idx) ((reg_idx == 0) ? 0ULL : Ps2GprLow64(ctx_ptr, reg_idx))
#define GPR_S64(ctx_ptr, reg_idx) ((reg_idx == 0) ? 0LL : static_cast<int64_t>(Ps2GprLow64(ctx_ptr, reg_idx)))
#define GPR_VEC(ctx_ptr, reg_idx) ((reg_idx == 0) ? _mm_setzero_si128() : ctx_ptr->r[reg_idx])

static PS2_FORCE_INLINE void Ps2SetGprLow64(R5900Context *ctx, int reg, __m128i new_low)
{
    if (reg != 0)
    {
        ctx->r[reg] = _mm_castpd_si128(_mm_move_sd(_mm_castsi128_pd(ctx->r[reg]), _mm_castsi128_pd(new_low)));
    }
}

// As the SSE versions: the value is not evaluated for $zero.
#define SET_GPR_U32(ctx_ptr, reg_idx, val)                                                                                   \
    do                                                                                                                       \
    {                                                                                                                        \
        if ((reg_idx) != 0)                                                                                                  \
            Ps2SetGprLow64Scalar(ctx_ptr, reg_idx, static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(val)))); \
    } while (0)
#define SET_GPR_ZE32(ctx_ptr, reg_idx, val)                                                            \
    do                                                                                                 \
    {                                                                                                  \
        if ((reg_idx) != 0)                                                                            \
            Ps2SetGprLow64Scalar(ctx_ptr, reg_idx, static_cast<uint64_t>(static_cast<uint32_t>(val))); \
    } while (0)
#define SET_GPR_S32(ctx_ptr, reg_idx, val) SET_GPR_U32(ctx_ptr, reg_idx, val)
#define SET_GPR_U64(ctx_ptr, reg_idx, val)                                      \
    do                                                                          \
    {                                                                           \
        if ((reg_idx) != 0)                                                     \
            Ps2SetGprLow64Scalar(ctx_ptr, reg_idx, static_cast<uint64_t>(val)); \
    } while (0)
#else
#define GPR_U32(ctx_ptr, reg_idx) ((reg_idx == 0) ? 0U : static_cast<uint32_t>(PS2_EXTRACT_EPI32_0(ctx_ptr->r[reg_idx])))
#define GPR_S32(ctx_ptr, reg_idx) ((reg_idx == 0) ? 0 : PS2_EXTRACT_EPI32_0(ctx_ptr->r[reg_idx]))
#define GPR_U64(ctx_ptr, reg_idx) ((reg_idx == 0) ? 0ULL : static_cast<uint64_t>(PS2_EXTRACT_EPI64_0(ctx_ptr->r[reg_idx])))
#define GPR_S64(ctx_ptr, reg_idx) ((reg_idx == 0) ? 0LL : PS2_EXTRACT_EPI64_0(ctx_ptr->r[reg_idx]))
#define GPR_VEC(ctx_ptr, reg_idx) ((reg_idx == 0) ? _mm_setzero_si128() : ctx_ptr->r[reg_idx])

static PS2_FORCE_INLINE void Ps2SetGprLow64(R5900Context *ctx, int reg, __m128i new_low)
{
    if (reg != 0)
    {
        ctx->r[reg] = _mm_castpd_si128(_mm_move_sd(_mm_castsi128_pd(ctx->r[reg]), _mm_castsi128_pd(new_low)));
    }
}

#define SET_GPR_U32(ctx_ptr, reg_idx, val)                                \
    do                                                                    \
    {                                                                     \
        if ((reg_idx) != 0)                                               \
        {                                                                 \
            __m128i _newVal = _mm_cvtsi64_si128((int64_t)(int32_t)(val)); \
                                                                          \
            Ps2SetGprLow64(ctx_ptr, reg_idx, _newVal);                    \
        }                                                                 \
    } while (0)


#define SET_GPR_ZE32(ctx_ptr, reg_idx, val)                               \
    do                                                                    \
    {                                                                     \
        if ((reg_idx) != 0)                                               \
        {                                                                 \
            __m128i _newVal = _mm_cvtsi64_si128((int64_t)(uint32_t)(val)); \
            Ps2SetGprLow64(ctx_ptr, reg_idx, _newVal);                    \
        }                                                                 \
    } while (0)

#define SET_GPR_S32(ctx_ptr, reg_idx, val)                                \
    do                                                                    \
    {                                                                     \
        if ((reg_idx) != 0)                                               \
        {                                                                 \
            __m128i _newVal = _mm_cvtsi64_si128((int64_t)(int32_t)(val)); \
            Ps2SetGprLow64(ctx_ptr, reg_idx, _newVal);                    \
        }                                                                 \
    } while (0)

#define SET_GPR_U64(ctx_ptr, reg_idx, val)                       \
    do                                                           \
    {                                                            \
        if ((reg_idx) != 0)                                      \
        {                                                        \
            __m128i _newVal = _mm_cvtsi64_si128((int64_t)(val)); \
            Ps2SetGprLow64(ctx_ptr, reg_idx, _newVal);           \
        }                                                        \
    } while (0)

#endif

#define SET_GPR_S64(ctx_ptr, reg_idx, val) SET_GPR_U64(ctx_ptr, reg_idx, val)

#define SET_GPR_VEC(ctx_ptr, reg_idx, val) \
    do                                     \
    {                                      \
        if (reg_idx != 0)                  \
            ctx_ptr->r[reg_idx] = (val);   \
    } while (0)

#endif // PS2_RUNTIME_MACROS_H
