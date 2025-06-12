//
// xemu User Interface
//
// Copyright (C) 2024-2025 James Puleo
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
#include <optional>
#include <vector>

class DebugTSSMWindow {
public:
    DebugTSSMWindow();
    void Draw();

    bool m_is_open{};

private:
    MemoryEditor m_memory_editor;
    std::optional<unsigned short> m_selected_base_index{};
    std::vector<float> m_frame_times;
};

extern DebugTSSMWindow tssm_window;
