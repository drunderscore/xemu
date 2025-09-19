//
// xemu User Interface
//
// Copyright (C) 2025 resin
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

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
