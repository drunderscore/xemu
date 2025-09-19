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

namespace TSSM::Core
{
struct RpAtomic;
struct RpLight;

struct RwRGBAReal
{
    float red;   // offset 0x0, size 0x4
    float green; // offset 0x4, size 0x4
    float blue;  // offset 0x8, size 0x4
    float alpha; // offset 0xC, size 0x4
};

struct RwV3d
{
    float x; // offset 0x0, size 0x4
    float y; // offset 0x4, size 0x4
    float z; // offset 0x8, size 0x4
};

struct RwMatrixTag
{
    RwV3d right;        // offset 0x0, size 0xC
    unsigned int flags; // offset 0xC, size 0x4
    RwV3d up;           // offset 0x10, size 0xC
    unsigned int pad1;  // offset 0x1C, size 0x4
    RwV3d at;           // offset 0x20, size 0xC
    unsigned int pad2;  // offset 0x2C, size 0x4
    RwV3d pos;          // offset 0x30, size 0xC
    unsigned int pad3;  // offset 0x3C, size 0x4
};
}
