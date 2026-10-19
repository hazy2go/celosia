#pragma once

#include <string>

namespace ps2recomp
{
    // Rewrites one generated function so the guest GPRs it uses through the
    // scalar macros live in locals (grN), loaded from the context at entry,
    // written back (the ones it writes) at every return and before every call
    // that can read them, and reloaded after every call. Registers it also
    // uses as 128-bit values (GPR_VEC / SET_GPR_VEC) stay in the context.
    // CELOSIA_LEAN_GPRS=1 when the recompiler runs.
    std::string leanGprs(const std::string &function);
    bool leanGprsEnabled();
}
