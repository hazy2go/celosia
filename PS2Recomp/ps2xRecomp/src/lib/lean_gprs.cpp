#include "ps2recomp/lean_gprs.h"

#include <array>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

namespace ps2recomp
{
    namespace
    {
        constexpr int kRegs = 32;

        struct Access
        {
            const char *prefix; // up to and including "ctx, "
            bool write;
        };

        // Writes first: "GPR_U32(ctx, " is also the tail of "SET_GPR_U32(ctx, ".
        constexpr std::array<Access, 12> kAccesses{{
            {"SET_GPR_U32(ctx, ", true},
            {"SET_GPR_S32(ctx, ", true},
            {"SET_GPR_ZE32(ctx, ", true},
            {"SET_GPR_U64(ctx, ", true},
            {"SET_GPR_S64(ctx, ", true},
            {"SET_GPR_VEC(ctx, ", true},
            {"GPR_U32(ctx, ", false},
            {"GPR_S32(ctx, ", false},
            {"GPR_U64(ctx, ", false},
            {"GPR_S64(ctx, ", false},
            {"GPR_VEC(ctx, ", false},
            {"getRegU32(ctx, ", false},
        }};

        bool startsAt(const std::string &s, size_t at, const char *prefix)
        {
            return s.compare(at, std::char_traits<char>::length(prefix), prefix) == 0;
        }

        // The register number after a prefix, or -1.
        int registerAt(const std::string &s, size_t at, size_t &end)
        {
            size_t i = at;
            int n = 0;
            while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i])))
                n = n * 10 + (s[i++] - '0');
            if (i == at || n >= kRegs)
                return -1;
            end = i;
            return n;
        }

        // Index of the ')' closing the '(' before `from`.
        size_t closingParen(const std::string &s, size_t from)
        {
            int depth = 1;
            for (size_t i = from; i < s.size(); ++i)
            {
                if (s[i] == '(')
                    ++depth;
                else if (s[i] == ')' && --depth == 0)
                    return i;
            }
            return std::string::npos;
        }

        bool isIdentifierChar(char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }

        std::string rewriteAccesses(const std::string &s, const std::array<bool, kRegs> &cached)
        {
            std::string out;
            out.reserve(s.size());
            for (size_t i = 0; i < s.size();)
            {
                bool done = false;
                if (i == 0 || !isIdentifierChar(s[i - 1]))
                    for (const Access &a : kAccesses)
                    {
                        if (!startsAt(s, i, a.prefix))
                            continue;
                        const size_t regAt = i + std::char_traits<char>::length(a.prefix);
                        size_t regEnd = 0;
                        const int reg = registerAt(s, regAt, regEnd);
                        if (reg <= 0 || !cached[reg])
                            break;
                        const std::string name = "gr" + std::to_string(reg);
                        if (a.write)
                        {
                            // SET_GPR_x(ctx, N, value) -> grN = convert(value)
                            if (s.compare(regEnd, 2, ", ") != 0)
                                break;
                            const size_t valueAt = regEnd + 2;
                            const size_t close = closingParen(s, valueAt);
                            if (close == std::string::npos)
                                break;
                            const std::string value = rewriteAccesses(s.substr(valueAt, close - valueAt), cached);
                            const std::string prefix(a.prefix);
                            const char *convert = prefix.find("ZE32") != std::string::npos ? "PS2_ZX32" : prefix.find("32") != std::string::npos ? "PS2_SX32" : "static_cast<uint64_t>";
                            out += name + " = " + convert + "(" + value + ")";
                            i = close + 1;
                        }
                        else
                        {
                            if (regEnd >= s.size() || s[regEnd] != ')')
                                break;
                            const std::string prefix(a.prefix);
                            if (prefix.rfind("GPR_U64", 0) == 0)
                                out += name;
                            else if (prefix.rfind("GPR_S64", 0) == 0)
                                out += "static_cast<int64_t>(" + name + ")";
                            else if (prefix.rfind("GPR_S32", 0) == 0)
                                out += "static_cast<int32_t>(" + name + ")";
                            else
                                out += "static_cast<uint32_t>(" + name + ")";
                            i = regEnd + 1;
                        }
                        done = true;
                        break;
                    }
                if (!done)
                    out += s[i++];
            }
            return out;
        }

        // A line that hands the context to code that may read or change the
        // guest registers: calls, transfers, syscalls, runtime services.
        bool isCallLine(const std::string &line)
        {
            if (line.find("(rdram, ctx") != std::string::npos)
                return true;
            for (size_t at = line.find("runtime->"); at != std::string::npos; at = line.find("runtime->", at + 1))
                if (!startsAt(line, at, "runtime->eeCheckpointDue("))
                    return true;
            return false;
        }

        std::string indentOf(const std::string &line)
        {
            return line.substr(0, line.find_first_not_of(' '));
        }

        int braceBalance(const std::string &line)
        {
            int balance = 0;
            for (char c : line)
                balance += c == '{' ? 1 : c == '}' ? -1 : 0;
            return balance;
        }
    }

    bool leanGprsEnabled()
    {
        static const bool enabled = std::getenv("CELOSIA_LEAN_GPRS") && std::string(std::getenv("CELOSIA_LEAN_GPRS")) != "0";
        return enabled;
    }

    std::string leanGprs(const std::string &function)
    {
        // Which registers: used through the scalar macros, never as 128 bits.
        std::array<bool, kRegs> used{}, written{}, wide{}, cached{};
        for (size_t i = 0; i < function.size(); ++i)
        {
            if (i > 0 && isIdentifierChar(function[i - 1]))
                continue;
            for (const Access &a : kAccesses)
            {
                if (!startsAt(function, i, a.prefix))
                    continue;
                size_t end = 0;
                const int reg = registerAt(function, i + std::char_traits<char>::length(a.prefix), end);
                if (reg > 0)
                {
                    const bool vec = std::string(a.prefix).find("VEC") != std::string::npos;
                    (vec ? wide : used)[reg] = true;
                    if (a.write && !vec)
                        written[reg] = true;
                }
                break;
            }
        }
        bool any = false;
        for (int r = 1; r < kRegs; ++r)
        {
            cached[r] = used[r] && !wide[r];
            any |= cached[r];
        }
        if (!any)
            return function;

        std::string writeback, reload, declare;
        for (int r = 1; r < kRegs; ++r)
        {
            if (!cached[r])
                continue;
            const std::string n = std::to_string(r);
            if (written[r])
                writeback += "Ps2StoreGpr64(ctx, " + n + ", gr" + n + "); ";
            reload += "gr" + n + " = Ps2LoadGpr64(ctx, " + n + "); ";
            declare += std::string(declare.empty() ? "    uint64_t " : ", ") + "gr" + n + " = Ps2LoadGpr64(ctx, " + n + ")";
        }
        declare += ";\n";
        const std::string writebackBlock = writeback.empty() ? std::string() : "{ " + writeback + "}";
        const std::string reloadBlock = "{ " + reload + "}";
        auto returnWithWriteback = [&](std::string line)
        {
            if (writebackBlock.empty())
                return line;
            for (size_t at = line.find("return;"); at != std::string::npos; at = line.find("return;", at + writebackBlock.size() + 12))
                line.replace(at, 7, "{ " + writebackBlock + " return; }");
            return line;
        };

        const std::string body = rewriteAccesses(function, cached);
        std::istringstream lines(body);
        std::ostringstream out;
        std::string line;
        bool declared = false;
        std::vector<int> reloadAtDepth; // a call's block: reload once the depth drops back
        int depth = 0;
        while (std::getline(lines, line))
        {
            if (!declared)
            {
                out << line << "\n";
                if (line.rfind("void ", 0) == 0 && line.find("(uint8_t* rdram, R5900Context* ctx, PS2Runtime *runtime) {") != std::string::npos)
                {
                    out << declare;
                    declared = true;
                    depth = 1;
                }
                continue;
            }
            const std::string indent = indentOf(line);
            if (isCallLine(line))
            {
                if (!writebackBlock.empty())
                    out << indent << writebackBlock << "\n";
                const size_t ret = line.find("return;");
                const size_t trimmedEnd = line.find_last_not_of(' ');
                const bool opensBlock = trimmedEnd != std::string::npos && line[trimmedEnd] == '{';
                if (ret != std::string::npos && !opensBlock)
                {
                    // "call; return;" (or "if (call) { return; }"): reload
                    // in between, the context is the truth now; and after
                    // the line, for when it does not return.
                    out << line.substr(0, ret) << "\n" << indent << reloadBlock << "\n" << indent << line.substr(ret) << "\n";
                    out << indent << reloadBlock << "\n";
                }
                else
                {
                    out << line << "\n";
                    if (opensBlock)
                    {
                        out << indent << "    " << reloadBlock << "\n";
                        reloadAtDepth.push_back(depth);
                    }
                    else
                        out << indent << reloadBlock << "\n";
                }
                depth += braceBalance(line);
                continue;
            }
            depth += braceBalance(line);
            if (depth == 0 && line == "}" && !writebackBlock.empty())
                out << "    " << writebackBlock << "\n"; // falling off the end is a return too
            out << returnWithWriteback(line) << "\n";
            if (!reloadAtDepth.empty() && depth <= reloadAtDepth.back())
            {
                reloadAtDepth.pop_back();
                out << indent << reloadBlock << "\n";
            }
        }
        return out.str();
    }
}
