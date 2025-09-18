#pragma once

namespace TSSM
{
// This originally tried to simply be:
// template<typename T>
// using GuestPtr = unsigned int
//
// But this made writing the read function difficult.
// See the conversation in SerenityOS Discord:
// https://discord.com/channels/830522505605283862/830525235803586570/1417525875351621652
// Using a discrete type seemingly avoids issues.
template<typename T>
struct GuestPtr
{
    constexpr GuestPtr() {}
    constexpr GuestPtr(unsigned int value) : value(value) {}

    explicit operator unsigned int() const { return value; }
    explicit operator bool() const { return value; }

    unsigned int value;
};
}
