// CELOSIA_MICROBENCH=1 (env.txt): times the building blocks of the
// recompiled code on the Vita itself, built as the game is (-Os, the same
// macros), and logs nanoseconds per operation before the game starts. What
// costs what on the Cortex-A9 decides which code generation to change.
#if defined(PLATFORM_VITA)

#include "ps2_runtime_macros.h"
#include "ps2_fpu_neon.h"

#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <vector>

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

    // The scheduler check at a loop's back edge, the same in both versions.
    uint64_t g_protoChecks = 0;
    [[gnu::noinline]] bool protoCheckpoint()
    {
        ++g_protoChecks;
        return false;
    }

#include "ps2_vita_proto_old.inc"
#include "ps2_vita_proto_genlean.inc"

    // The same function as a leaner code generator would emit it: guest
    // registers in locals (the compiler keeps them in core registers), no pc
    // or delay-slot bookkeeping per instruction, written back once on the way
    // out. Same memory macros, same check at the back edge.
    [[gnu::noinline]] void protoLean_110dd58(uint8_t *rdram, R5900Context *ctx, PS2Runtime *runtime)
    {
        const uint64_t a0 = GPR_U64(ctx, 4), a3 = GPR_U64(ctx, 7), t0 = GPR_U64(ctx, 8);
        uint64_t v0, v1, a1 = GPR_U64(ctx, 5), a2 = GPR_U64(ctx, 6);
        auto sext = [](uint32_t v) { return static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(v))); };
        for (;;)
        {
            v0 = sext(static_cast<uint32_t>(a2) << 2);                                  // sll v0, a2, 2
            v1 = static_cast<int64_t>(a2) < static_cast<int64_t>(t0) ? 1u : 0u;          // slt v1, a2, t0
            v0 = sext(static_cast<uint32_t>(v0) + static_cast<uint32_t>(a3));           // addu v0, v0, a3 (delay)
            if (v1 == 0u)                                                               // beqz v1 -> 0x110dd80
                goto out;
            WRITE32(static_cast<uint32_t>(a0) + 16u, static_cast<uint32_t>(a2));        // sw a2, 0x10(a0)
            v0 = sext(READ32(static_cast<uint32_t>(v0)));                               // lw v0, 0(v0)
            if (v0 == 0u)                                                               // beql v0, zero -> 0x110dd58
            {
                a2 = sext(static_cast<uint32_t>(a2) + 1u);                              // addiu a2, a2, 1 (likely delay)
                if (protoCheckpoint())
                {
                    Ps2SetGprLow64Scalar(ctx, 2, v0), Ps2SetGprLow64Scalar(ctx, 3, v1), Ps2SetGprLow64Scalar(ctx, 6, a2);
                    ctx->pc = 0x110dd58u;
                    return;
                }
                continue;
            }
            a1 = v0;                                                                    // daddu a1, v0, zero
            break;
        }
    out:
        v0 = a1;                                                                        // jr ra; daddu v0, a1 (delay)
        Ps2SetGprLow64Scalar(ctx, 2, v0), Ps2SetGprLow64Scalar(ctx, 3, v1), Ps2SetGprLow64Scalar(ctx, 5, a1),
            Ps2SetGprLow64Scalar(ctx, 6, a2);
        ctx->pc = GPR_U32(ctx, 31);
    }

    // A list scan of `length` words, the hit at `hit` (0x110dd58's loop).
    void protoSetup(uint8_t *rdram, R5900Context *ctx, uint32_t length, uint32_t hit)
    {
        std::memset(ctx, 0, sizeof(*ctx));
        Ps2SetGprLow64Scalar(ctx, 4, 0x200000u);  // a0: the struct (+0x10 written)
        Ps2SetGprLow64Scalar(ctx, 5, 0x55u);      // a1
        Ps2SetGprLow64Scalar(ctx, 6, 0u);         // a2: index
        Ps2SetGprLow64Scalar(ctx, 7, 0x100000u);  // a3: the table
        Ps2SetGprLow64Scalar(ctx, 8, length);     // t0: its length
        Ps2SetGprLow64Scalar(ctx, 31, 0x123450u); // ra
        std::memset(rdram + 0x100000u, 0, length * 4u);
        const uint32_t value = 0xCAFEu;
        std::memcpy(rdram + 0x100000u + hit * 4u, &value, 4u);
        ctx->pc = 0x110dd58u;
    }
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
    run("steady_clock::now()", [&]
        { uint64_t s = 0; for (uint32_t i = 0; i < kOps / 64u; ++i) s += static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count()); sink(s); });
    results[count - 1].nsPerOp *= 64.0; // a 64th of the loop
    run("dmb (memory barrier)", [&]
        { for (uint32_t i = 0; i < kOps; ++i) __sync_synchronize(); sink(0); });
    run("float->int->float (vmov round trip)", [&]
        { float s = 1.0f; for (uint32_t i = 0; i < kOps; ++i) { uint32_t b; std::memcpy(&b, &s, 4); b ^= (i & 1u); std::memcpy(&s, &b, 4); s += data[i & kMask]; } sink(s); });

    {
        // The recompiled function as generated now against the lean version:
        // a 1000-word scan with the hit at 900, many times; the same end state.
        static R5900Context oldCtx{}, leanCtx{};
        constexpr int kScans = 2000;
        const uint64_t t0 = nowUs();
        for (int i = 0; i < kScans; ++i)
        {
            protoSetup(rdram, &oldCtx, 1000u, 900u);
            protoOld_110dd58(rdram, &oldCtx, nullptr);
        }
        const uint64_t t1 = nowUs();
        uint32_t oldStruct = 0;
        std::memcpy(&oldStruct, rdram + 0x200010u, 4u);
        for (int i = 0; i < kScans; ++i)
        {
            protoSetup(rdram, &leanCtx, 1000u, 900u);
            protoLean_110dd58(rdram, &leanCtx, nullptr);
        }
        const uint64_t t2 = nowUs();
        static R5900Context genCtx{};
        for (int i = 0; i < kScans; ++i)
        {
            protoSetup(rdram, &genCtx, 1000u, 900u);
            protoGenLean_110dd58(rdram, &genCtx, nullptr);
        }
        const uint64_t t3 = nowUs();
        std::fprintf(stderr, "[microbench] proto 0x110dd58: lean-GPR pass output %.1f ns/iteration (v0=%llx)\n", (t3 - t2) * 1000.0 / (kScans * 901.0),
                     static_cast<unsigned long long>(GPR_U64(&genCtx, 2)));
        uint32_t leanStruct = 0;
        std::memcpy(&leanStruct, rdram + 0x200010u, 4u);
        bool same = oldCtx.pc == leanCtx.pc && oldStruct == leanStruct;
        for (int r : {2, 3, 4, 5, 6, 7, 8, 31})
            same = same && GPR_U64(&oldCtx, r) == GPR_U64(&leanCtx, r);
        const double iterations = kScans * 901.0;
        std::fprintf(stderr, "[microbench] proto 0x110dd58: generated %.1f ns/iteration, lean %.1f ns/iteration (%.2fx), results %s (v0=%llx a2=%llu pc=%x)\n",
                     (t1 - t0) * 1000.0 / iterations, (t2 - t1) * 1000.0 / iterations, static_cast<double>(t1 - t0) / std::max<uint64_t>(1, t2 - t1),
                     same ? "identical" : "DIFFERENT", static_cast<unsigned long long>(GPR_U64(&leanCtx, 2)), static_cast<unsigned long long>(GPR_U64(&leanCtx, 6)),
                     leanCtx.pc);
    }
    // Random dependent reads (one per 64-byte line) over a buffer: the
    // latency of a guest RAM access that misses the caches, by where the
    // memory comes from (the heap's pages, a memory block's, a physically
    // contiguous block's: TLB reach).
    {
        auto chase = [](uint32_t *buf, uint32_t bytes) -> double
        {
            const uint32_t nodes = bytes / 64u;
            std::vector<uint32_t> order(nodes);
            for (uint32_t i = 0; i < nodes; ++i)
                order[i] = i;
            uint32_t seed = 12345u;
            for (uint32_t i = nodes - 1u; i > 0; --i)
            {
                seed = seed * 1664525u + 1013904223u;
                std::swap(order[i], order[seed % (i + 1u)]);
            }
            for (uint32_t i = 0; i < nodes; ++i)
                buf[order[i] * 16u] = order[(i + 1u) % nodes] * 16u;
            constexpr uint32_t kSteps = 2u * 1024u * 1024u;
            uint32_t at = order[0] * 16u;
            const uint64_t start = nowUs();
            for (uint32_t i = 0; i < kSteps; ++i)
                at = buf[at];
            const uint64_t us = nowUs() - start;
            sink(at);
            return us * 1000.0 / kSteps;
        };
        constexpr uint32_t kBig = 32u << 20;
        if (auto *heap = static_cast<uint32_t *>(std::malloc(kBig)))
        {
            for (uint32_t mb : {1u, 2u, 4u, 8u, 16u, 32u})
                std::fprintf(stderr, "[microbench] random read, %2u MB of heap:              %6.1f ns\n", mb, chase(heap, mb << 20));
            std::free(heap);
        }
        const struct
        {
            const char *name;
            SceKernelMemBlockType type;
        } blocks[] = {{"USER_RW", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW},
                      {"USER_MAIN_GAME_RW", SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_GAME_RW},
                      {"USER_MAIN_PHYCONT_RW", SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_RW},
                      {"USER_CDRAM_L1WBWA_RW", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_L1WBWA_RW}};
        for (const auto &b : blocks)
            for (uint32_t mb : {4u, 8u, 16u, 32u})
            {
                const SceUID block = sceKernelAllocMemBlock("chase", b.type, mb << 20, nullptr);
                void *base = nullptr;
                if (block < 0 || sceKernelGetMemBlockBase(block, &base) < 0)
                {
                    std::fprintf(stderr, "[microbench] random read, %2u MB %-22s no block (0x%x)\n", mb, b.name, static_cast<unsigned>(block));
                    continue;
                }
                std::fprintf(stderr, "[microbench] random read, %2u MB %-22s %6.1f ns\n", mb, b.name, chase(static_cast<uint32_t *>(base), mb << 20));
                sceKernelFreeMemBlock(block);
            }
    }
    std::fprintf(stderr, "[microbench] %u ops each, ns per op (the loop's own cost included):\n", kOps);
    for (int i = 0; i < count; ++i)
        std::fprintf(stderr, "[microbench] %-38s %7.2f ns\n", results[i].name, results[i].nsPerOp);
    std::fflush(stderr);
}

#endif
