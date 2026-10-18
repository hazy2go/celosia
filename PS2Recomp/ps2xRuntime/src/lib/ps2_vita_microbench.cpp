// CELOSIA_MICROBENCH=1 (env.txt): times the building blocks of the
// recompiled code on the Vita itself, built as the game is (-Os, the same
// macros), and logs nanoseconds per operation before the game starts. What
// costs what on the Cortex-A9 decides which code generation to change.
#if defined(PLATFORM_VITA)

#include "ps2_runtime_macros.h"
#include "ps2_fpu_neon.h"

#include <psp2/kernel/processmgr.h>

#include <cstdio>
#include <cstdlib>

namespace
{
    constexpr uint32_t kOps = 2u * 1024u * 1024u;
    constexpr uint32_t kMask = 1023u;

    struct Bench
    {
        const char *name;
        double nsPerOp;
    };

    uint64_t nowUs() { return sceKernelGetProcessTimeWide(); }

    // Keep the compiler from folding a loop away.
    template <typename T>
    __attribute__((noinline)) void sink(T value)
    {
        static volatile T kept;
        kept = value;
    }

    // The SSE emulation's GPR access, as before the scalar macros.
    PS2_FORCE_INLINE uint32_t oldGprU32(const R5900Context *ctx, int reg) { return static_cast<uint32_t>(PS2_EXTRACT_EPI32_0(ctx->r[reg])); }
    PS2_FORCE_INLINE void oldSetGprS32(R5900Context *ctx, int reg, int32_t value)
    {
        Ps2SetGprLow64(ctx, reg, _mm_cvtsi64_si128(static_cast<int64_t>(value)));
    }

    __attribute__((noinline)) void callee(R5900Context *ctx) { ctx->pc += 4u; }
}

void runVitaMicrobench()
{
    if (!std::getenv("CELOSIA_MICROBENCH"))
        return;
    ps2SetEeFloatingPointMode(); // as the game thread
    static float data[kMask + 1];
    static uint32_t words[kMask + 1];
    for (uint32_t i = 0; i <= kMask; ++i)
    {
        data[i] = static_cast<float>(std::rand() % 20000 - 10000) / static_cast<float>(1 + std::rand() % 977);
        words[i] = (static_cast<uint32_t>(std::rand()) << 8) & (PS2_RAM_MASK & ~3u);
    }
    static R5900Context context{};
    R5900Context *ctx = &context;
    static uint8_t *rdram = static_cast<uint8_t *>(std::calloc(PS2_RAM_SIZE, 1));
    if (!rdram)
        return;
    void (*volatile indirect)(R5900Context *) = callee;

    Bench results[24];
    int count = 0;
    auto run = [&](const char *name, auto &&body)
    {
        const uint64_t start = nowUs();
        body();
        const uint64_t us = nowUs() - start;
        results[count++] = {name, us * 1000.0 / kOps};
    };

    run("loop + float add (baseline)", [&]
        { float s = 0.0f; for (uint32_t i = 0; i < kOps; ++i) s = s + data[i & kMask]; sink(s); });
    run("FPU_ADD_S (PS2 exact)", [&]
        { float s = 0.0f; for (uint32_t i = 0; i < kOps; ++i) s = FPU_ADD_S(s, data[i & kMask]); sink(s); });
    run("loop + float mul", [&]
        { float s = 1.0f; for (uint32_t i = 0; i < kOps; ++i) s = s * data[i & kMask]; sink(s); });
    run("FPU_MUL_S (PS2 exact)", [&]
        { float s = 1.0f; for (uint32_t i = 0; i < kOps; ++i) s = FPU_MUL_S(s, data[i & kMask]); sink(s); });
    run("FPU_ADD_S in NEON (exact)", [&]
        { float s = 0.0f; for (uint32_t i = 0; i < kOps; ++i) s = ps2NeonAdd(s, data[i & kMask]); sink(s); });
    run("FPU_MUL_S in NEON (exact)", [&]
        { float s = 1.0f; for (uint32_t i = 0; i < kOps; ++i) s = ps2NeonMul(s, data[i & kMask]); sink(s); });
    run("ctx->f add (as recompiled)", [&]
        { for (uint32_t i = 0; i < kOps; ++i) { ctx->f[1] = data[i & kMask]; ctx->f[0] = FPU_ADD_S(ctx->f[0], ctx->f[1]); } sink(ctx->f[0]); });
    run("GPR read+write, scalar (new)", [&]
        { for (uint32_t i = 0; i < kOps; ++i) SET_GPR_S32(ctx, 2, static_cast<int32_t>(GPR_U32(ctx, 3) + GPR_U32(ctx, 2) + i)); sink(GPR_U32(ctx, 2)); });
    run("GPR read+write, SSE (old)", [&]
        { for (uint32_t i = 0; i < kOps; ++i) oldSetGprS32(ctx, 2, static_cast<int32_t>(oldGprU32(ctx, 3) + oldGprU32(ctx, 2) + i)); sink(oldGprU32(ctx, 2)); });
    run("guest lw (READ32 fast path)", [&]
        { uint32_t s = 0; for (uint32_t i = 0; i < kOps; ++i) s += Ps2FastRead32(rdram, words[i & kMask]); sink(s); });
    run("guest sw (WRITE32 fast path)", [&]
        { for (uint32_t i = 0; i < kOps; ++i) Ps2FastWrite32(rdram, words[i & kMask], i); sink(rdram[0]); });
    run("ctx->pc store", [&]
        { for (uint32_t i = 0; i < kOps; ++i) { ctx->pc = 0x100000u + i; sink(i); } });
    run("indirect call (dispatch)", [&]
        { for (uint32_t i = 0; i < kOps; ++i) indirect(ctx); sink(ctx->pc); });
    run("dmb (memory barrier)", [&]
        { for (uint32_t i = 0; i < kOps; ++i) __sync_synchronize(); sink(0); });
    run("float->int->float (vmov round trip)", [&]
        { float s = 1.0f; for (uint32_t i = 0; i < kOps; ++i) { uint32_t b; std::memcpy(&b, &s, 4); b ^= (i & 1u); std::memcpy(&s, &b, 4); s += data[i & kMask]; } sink(s); });

    std::fprintf(stderr, "[microbench] %u ops each, ns per op (the loop's own cost included):\n", kOps);
    for (int i = 0; i < count; ++i)
        std::fprintf(stderr, "[microbench] %-38s %7.2f ns\n", results[i].name, results[i].nsPerOp);
    std::fflush(stderr);
}

#endif
