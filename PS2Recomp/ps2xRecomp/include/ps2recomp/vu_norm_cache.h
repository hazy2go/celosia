#pragma once

#include <array>
#include <string>

namespace ps2recomp
{
    // On by default (CELOSIA_VU_NORM_CSE=0 at recompile time: off). VU0 macro
    // operations normalize
    // every input (ps2VuNormalize, ~6 NEON instructions on the Vita), also
    // inputs that a VU operation just produced, and a register read four
    // times is normalized four times. This keeps each register's normalized
    // value in a local (vnN, vacc) while it is known to match the context and
    // gives the operations the _PRE macros that take normalized inputs.
    // Normalizing is idempotent and lane-wise, so the results are the same.
    // Valid within straight-line code only: labels, branches and calls (which
    // may change any VU0 register) forget everything.
    bool vuNormCacheEnabled();

    class VuNormCache
    {
    public:
        // The instruction's translation with the cache applied; what it needs
        // computed first is put before it.
        std::string apply(const std::string &translated);
        void forget();
        // Declarations for the function's entry (empty when nothing is used).
        std::string declarations() const;

    private:
        std::array<bool, 32> m_vf{};   // vnN equals ps2VuNormalize(ctx->vu0_vf[N])
        bool m_acc = false;            // vacc equals ps2VuNormalize(ctx->vu0_acc)
        std::array<bool, 32> m_used{}; // declared
        bool m_accUsed = false;
    };
}
