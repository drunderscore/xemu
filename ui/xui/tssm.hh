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

    MemoryEditor m_memory_editor;
};

extern DebugTSSMWindow tssm_window;
