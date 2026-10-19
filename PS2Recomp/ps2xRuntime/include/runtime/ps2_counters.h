#pragma once

#include <atomic>
#include <cstdint>

// Counters of the Vita performance work, apart from ps2_stats.h (which much
// of the runtime includes) so that adding one rebuilds only the files that
// count. Plain counters are the EE thread's alone.
namespace ps2_stats
{
    // Guest calls dispatched and scheduler checks (calls + loop edges), counted
    // on the one EE thread: plain counters.
    inline uint64_t guestCalls = 0, guestCheckpoints = 0;
    // Back to the dispatcher: its loop turns, and the checkpoints that unwound
    // the guest stack, by reason (pending event or stop, a deadline passed,
    // the time slice over with another thread ready).
    inline uint64_t dispatcherIterations = 0, unwindsPending = 0, unwindsDeadline = 0, unwindsSlice = 0;
    // Guest loads/stores that left the inline RAM path (runtime->LoadN/StoreN),
    // and how many of them were the scratchpad (0x70000000). EE thread only.
    inline uint64_t slowAccesses = 0, slowScratchpad = 0;
    inline void noteSlowAccess(uint32_t vaddr)
    {
        ++slowAccesses;
        slowScratchpad += (vaddr >> 28) == 7u;
    }
    inline std::atomic<uint64_t> vifChainUs{0}; // walking a VIF1 DMA chain (native: compacting it)
    inline uint64_t chainWalks = 0, chainTags = 0, chainBytes = 0; // EE thread only
    inline uint64_t compactorCodes = 0, compactorCopied = 0, gifScannedQwords = 0, refReplays = 0;
    inline std::atomic<uint64_t> vu1SyncWaitUs{0}, vu1SyncWaits{0}, gsPrivReadUs{0}, gsPrivReads{0}; // syncVu1 waits; GS privileged register reads
    inline std::atomic<uint64_t> discReadUs{0}, discReads{0}, discReadBytes{0}; // IOP-side disc file reads
    inline uint64_t skinPosed = 0, skinOutput = 0, skinDraws = 0; // skinned vertices posed / triangle-list vertices out, EE thread
    inline std::atomic<uint64_t> nmPaletteUs{0}, nmSkinUs{0}, nmParseUs{0}, nmUploadUs{0}; // native model work: bone palettes, CPU skinning, first-time packet parsing
    // Frame split (us): modelDisp's own (recompiled) code, the native model
    // work after it, the display list kicks (iosDmaSendSChainPath1).
    inline std::atomic<uint64_t> guestModelDispUs{0}, nativeModelUs{0}, dlKickUs{0}, modelDispCalls{0};
}
