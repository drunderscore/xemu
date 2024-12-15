//
// xemu User Interface
//
// Copyright (C) 2020-2022 Matt Borgerson
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

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_memory_editor.h"

class DebugTSSMWindow {
public:
    DebugTSSMWindow();
    void Draw();

    bool m_is_open{};

private:
    struct {
        uint32_t active_heap = 0x00413a3c;
        uint32_t gx_heap = 0x00413718;
        uint32_t scene = 0x002afe2c;
        uint32_t mem_depth_just_hip_start = 0x002b6a9c;
        uint32_t mem_depth_scene_start = 0x002b6a98;
        uint32_t mem_depth_just_hip_start_player = 0x002b62ec;
    } offsets;

    using U32 = uint32_t;
    using S32 = int32_t;
    using U16 = uint16_t;
    using S16 = int16_t;
    template <typename T> using guest_ptr = U32;

    struct xMemBlock_tag {
        U32 addr;
        U32 size;
        S32 align;
    };

    struct xHeapState_tag {
        U32 curr;
        U16 blk_ct;
        U16 pad;
        U32 used;
        U32 wasted;
        U32 unk;
    };

    struct xMemHeap_tag {
        U32 flags;
        U32 hard_base;
        U32 size;
        S16 opp_heap[2];

        xHeapState_tag state[12];

        U16 state_idx;
        U16 max_blks;
        guest_ptr<xMemBlock_tag> blk;
        guest_ptr<xMemBlock_tag> lastblk;
    };

    MemoryEditor m_memory_editor;
};

extern DebugTSSMWindow tssm_window;
