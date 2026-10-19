#pragma once

#include "ps2_gif_events.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

// VIF1 stream compaction for when no VU1 program runs (GS::skipRaster):
// the DMA chain is copied without the data nothing reads. UNPACK and MPG
// become NOPs (keeping the interrupt bit) and their data is left out; every
// other VIFcode and its data (DIRECT packets for the GS, STROW, STMASK,
// STCOL) is kept, so processVIF1Data sees the same registers. DIRECT packets
// (the GS's path 2) go to a GifEventScanner instead and become NOPs too.
// Fed in pieces as the chain is walked; VIFcodes never straddle pieces
// (they are whole quadwords), data may.
class Vif1Compactor
{
public:
    Vif1Compactor(uint32_t cycle, std::vector<uint8_t> &out, GifEventScanner &direct)
        : m_cycle(cycle), m_out(out), m_direct(direct) {}
    void feed(const uint8_t *data, uint32_t bytes);
    // A REF tag's payload (static model packets: ~3.3 MB of the opening's
    // ~4.2 MB a frame, 99% the same (address, size) frame after frame): its
    // output, end cycle and skipped bytes recorded the first time, replayed
    // after while its first and last 16 bytes still match. Not for packets
    // with GIF data (their events must be scanned) or that do not start and
    // end on a VIFcode boundary.
    void feedRef(const uint8_t *data, uint32_t bytes, uint32_t address);
    [[nodiscard]] uint64_t skippedBytes() const { return m_skipped; }

private:
    void append(const void *data, uint32_t bytes)
    {
        const size_t at = m_out.size();
        if (m_out.capacity() < at + bytes)
            m_out.reserve(std::max<size_t>(at * 2u, 4096u));
        m_out.resize(at + bytes);
        std::memcpy(m_out.data() + at, data, bytes);
    }

    struct RefSummary
    {
        uint8_t head[16], tail[16];
        uint32_t entryCycle, exitCycle;
        uint64_t skipped;
        std::vector<uint8_t> out;
    };
    static std::unordered_map<uint64_t, RefSummary> &refSummaries();
    bool atCodeBoundary() const { return m_copy == 0u && m_skip == 0u && m_gif == 0u; }

    bool m_sawGif = false;
    uint32_t m_cycle;
    std::vector<uint8_t> &m_out;
    GifEventScanner &m_direct;
    uint32_t m_copy = 0, m_skip = 0, m_gif = 0;
    uint64_t m_skipped = 0;
};
