#include "ps2recomp/vu_norm_cache.h"

#include <cstdlib>
#include <regex>
#include <set>
#include <vector>

namespace ps2recomp
{
    namespace
    {
        const std::string kVf = "ctx->vu0_vf[";
        const std::string kAcc = "ctx->vu0_acc";

        // The argument list of the call whose '(' is at `open`: [begin, end) of
        // each argument and the index of the closing ')'.
        bool splitArguments(const std::string &s, size_t open, std::vector<std::pair<size_t, size_t>> &arguments, size_t &close)
        {
            int depth = 0;
            size_t start = open + 1;
            for (size_t i = open; i < s.size(); ++i)
            {
                const char c = s[i];
                if (c == '(')
                    ++depth;
                else if (c == ')')
                {
                    if (--depth == 0)
                    {
                        arguments.emplace_back(start, i);
                        close = i;
                        return true;
                    }
                }
                else if (c == ',' && depth == 1)
                {
                    arguments.emplace_back(start, i);
                    start = i + 1;
                }
            }
            return false;
        }

        std::string trim(const std::string &s)
        {
            const size_t a = s.find_first_not_of(' '), b = s.find_last_not_of(' ');
            return a == std::string::npos ? std::string() : s.substr(a, b - a + 1);
        }

        // N when `s` is exactly ctx->vu0_vf[N], else -1.
        int plainRegister(const std::string &s)
        {
            static const std::regex re(R"(^ctx->vu0_vf\[(\d+)\]$)");
            std::smatch m;
            if (!std::regex_match(s, m, re))
                return -1;
            const int n = std::stoi(m[1]);
            return n < 32 ? n : -1;
        }
    }

    bool vuNormCacheEnabled()
    {
        static const bool enabled = !std::getenv("CELOSIA_VU_NORM_CSE") || std::string(std::getenv("CELOSIA_VU_NORM_CSE")) != "0";
        return enabled;
    }

    void VuNormCache::forget()
    {
        m_vf.fill(false);
        m_acc = false;
    }

    std::string VuNormCache::declarations() const
    {
        std::string out;
        for (int n = 0; n < 32; ++n)
            if (m_used[n])
                out += std::string(out.empty() ? "    __m128 " : ", ") + "vn" + std::to_string(n);
        if (m_accUsed)
            out += std::string(out.empty() ? "    __m128 " : ", ") + "vacc";
        return out.empty() ? out : out + ";\n";
    }

    std::string VuNormCache::apply(const std::string &translated)
    {
        std::string s = translated;
        // Anything that may change VU0 registers out of sight, or leave this
        // straight-line code: forget after it.
        const bool leaves = s.find("goto ") != std::string::npos || s.find("return") != std::string::npos ||
                            s.find("(rdram, ctx") != std::string::npos || s.find("runtime->") != std::string::npos;
        if (s.find("vu0_vf[") == std::string::npos && s.find("vu0_acc") == std::string::npos)
        {
            if (leaves)
                forget();
            return s;
        }

        std::string prefix;
        std::set<int> loadsNeeded, reused;
        bool accNeeded = false, accReused = false;
        auto normalized = [&](int n)
        {
            if (!m_vf[n])
                loadsNeeded.insert(n);
            else
                reused.insert(n);
            m_used[n] = true;
            return "vn" + std::to_string(n);
        };

        // Temporaries that shuffle one register (vopmula's): from vnN, normalized too.
        std::set<std::string> normalizedNames{"mul_res"};
        {
            static const std::regex re(R"(__m128 (\w+) = PS2_SHUFFLE_SAME\(ctx->vu0_vf\[(\d+)\], )");
            std::string out;
            std::sregex_iterator it(s.begin(), s.end(), re), end;
            size_t last = 0;
            for (; it != end; ++it)
            {
                const int n = std::stoi((*it)[2]);
                if (n >= 32)
                    continue;
                out += s.substr(last, it->position() - last);
                out += "__m128 " + (*it)[1].str() + " = PS2_SHUFFLE_SAME(" + normalized(n) + ", ";
                normalizedNames.insert((*it)[1]);
                last = it->position() + it->length();
            }
            s = out + s.substr(last);
        }

        // The arithmetic: inputs as normalized locals where they can be.
        std::set<std::string> preResults; // __m128 names assigned from a _PRE operation
        for (const char *op : {"PS2_VMUL(", "PS2_VADD(", "PS2_VSUB("})
        {
            for (size_t at = s.find(op); at != std::string::npos; at = s.find(op, at + 1))
            {
                const size_t open = at + std::string(op).size() - 1;
                std::vector<std::pair<size_t, size_t>> arguments;
                size_t close = 0;
                if (!splitArguments(s, open, arguments, close) || arguments.size() != 2)
                    continue;
                std::string rewritten[2];
                for (int k = 0; k < 2; ++k)
                {
                    const std::string a = trim(s.substr(arguments[k].first, arguments[k].second - arguments[k].first));
                    static const std::regex shuffleSame(R"(^PS2_SHUFFLE_SAME\(ctx->vu0_vf\[(\d+)\], (_MM_SHUFFLE\(\d,\d,\d,\d\))\)$)");
                    static const std::regex shufflePair(R"(^_mm_shuffle_ps\(ctx->vu0_vf\[(\d+)\], ctx->vu0_vf\[(\d+)\], (.+)\)$)");
                    std::smatch m;
                    const int n = plainRegister(a);
                    if (n >= 0)
                        rewritten[k] = normalized(n);
                    else if (a == kAcc)
                    {
                        accNeeded = !m_acc;
                        accReused = m_acc;
                        m_accUsed = true;
                        rewritten[k] = "vacc";
                    }
                    else if (normalizedNames.count(a))
                        rewritten[k] = a;
                    else if (std::regex_match(a, m, shuffleSame) && std::stoi(m[1]) < 32)
                        rewritten[k] = "PS2_SHUFFLE_SAME(" + normalized(std::stoi(m[1])) + ", " + m[2].str() + ")";
                    else if (std::regex_match(a, m, shufflePair) && m[1] == m[2] && std::stoi(m[1]) < 32)
                    {
                        const std::string v = normalized(std::stoi(m[1]));
                        rewritten[k] = "_mm_shuffle_ps(" + v + ", " + v + ", " + m[3].str() + ")";
                    }
                    else
                        rewritten[k] = "ps2VuNormalize((__m128)(" + a + "))";
                }
                const std::string name = std::string(op).substr(0, std::string(op).size() - 1);
                s.replace(at, close + 1 - at, name + "_PRE(" + rewritten[0] + ", " + rewritten[1] + ")");
            }
        }
        {
            static const std::regex re(R"(__m128 (\w+) = PS2_V(MUL|ADD|SUB)_PRE\()");
            for (std::sregex_iterator it(s.begin(), s.end(), re), end; it != end; ++it)
                preResults.insert((*it)[1]);
        }

        // Writes of a whole register from a _PRE result: the local takes it.
        // (The masks the translator writes: all four lanes.)
        const bool fullMask = s.find("_mm_set_epi32(-1, -1, -1, -1)") != std::string::npos;
        int fullVf = -1;
        bool fullAcc = false;
        if (fullMask)
        {
            static const std::regex vfWrite(R"(ctx->vu0_vf\[(\d+)\] = (?:_mm_blendv_ps|PS2_VBLEND)\(ctx->vu0_vf\[(\d+)\], (\w+), )");
            static const std::regex accWrite(R"(ctx->vu0_acc = (?:_mm_blendv_ps|PS2_VBLEND)\(ctx->vu0_acc, (\w+), )");
            std::smatch m;
            if (std::regex_search(s, m, vfWrite) && m[1] == m[2] && preResults.count(m[3]) && std::stoi(m[1]) < 32)
            {
                fullVf = std::stoi(m[1]);
                const size_t end = s.find(';', m.position() + m.length());
                if (end != std::string::npos)
                    s.insert(end + 1, " vn" + m[1].str() + " = " + m[3].str() + ";");
                m_used[fullVf] = true;
            }
            if (std::regex_search(s, m, accWrite) && preResults.count(m[1]))
            {
                fullAcc = true;
                const size_t end = s.find(';', m.position() + m.length());
                if (end != std::string::npos)
                    s.insert(end + 1, " vacc = " + m[1].str() + ";");
                m_accUsed = true;
            }
        }

        // CELOSIA_VU_NORM_VERIFY: every value taken from the cache checked
        // against the register's own (ps2VuNormCheck logs a difference).
        static const bool verify = std::getenv("CELOSIA_VU_NORM_VERIFY") != nullptr;
        if (verify)
        {
            for (int n : reused)
                prefix += "ps2VuNormCheck(vn" + std::to_string(n) + ", ctx->vu0_vf[" + std::to_string(n) + "], " + std::to_string(n) + "); ";
            if (accReused)
                prefix += "ps2VuNormCheck(vacc, ctx->vu0_acc, 32); ";
        }
        // Loads the instruction needs, before it.
        for (int n : loadsNeeded)
            prefix += "vn" + std::to_string(n) + " = ps2VuNormalize(ctx->vu0_vf[" + std::to_string(n) + "]); ";
        if (accNeeded)
            prefix += "vacc = ps2VuNormalize(ctx->vu0_acc); ";
        for (int n : loadsNeeded)
            m_vf[n] = true;
        if (accNeeded)
            m_acc = true;

        // Whatever still names a register raw (writes, raw reads) forgets it,
        // except the whole-register writes just recorded.
        static const std::regex anyVf(R"(vu0_vf\[(\d+)\])");
        for (std::sregex_iterator it(s.begin(), s.end(), anyVf), end; it != end; ++it)
        {
            const int n = std::stoi((*it)[1]);
            if (n < 32 && n != fullVf)
                m_vf[n] = false;
        }
        if (fullVf >= 0)
            m_vf[fullVf] = true;
        if (s.find("vu0_acc") != std::string::npos && !fullAcc)
        {
            // Raw ACC reads left are fine; any write to it is not.
            static const std::regex accAssign(R"(ctx->vu0_acc\s*=)");
            if (std::regex_search(s, accAssign))
                m_acc = false;
        }
        if (fullAcc)
            m_acc = true;
        if (leaves)
            forget();
        return prefix.empty() ? s : prefix + s;
    }
}
