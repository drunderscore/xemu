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

#include "Interface.h"
#include "Core.h"
#include "Game.h"
#include "imgui.h"
#include <memory>

#include <cstdint>
// Must include cstdint before this.
#include "exec/vaddr.h"

extern "C"
{
    struct CPUState;

    CPUState* qemu_get_cpu(int index);

    int cpu_memory_rw_debug(CPUState* cpu, vaddr addr, void* ptr, size_t len, bool is_write);
}

template<typename T>
T read(TSSM::GuestPtr<T> address)
{
    T value;

    cpu_memory_rw_debug(qemu_get_cpu(0), address.value, &value, sizeof(T), false);

    return value;
}

template<unsigned Size>
void read_string(TSSM::GuestPtr<char> address, char (&buffer)[Size])
{
    // FIXME: Holy stupidity! At least try to avoid the likely
    //        lock/unlock churn when reading using cpu_memory_rw_debug,
    //        one byte at a time...

    for (unsigned i = 0; i < sizeof(buffer); i++)
    {
        if (!(buffer[i] = read(address)))
            break;

        address.value += 1;
    }

    // Make absolutely sure that for the worst-case scenario, we are null-terminated.
    buffer[Size - 1] = '\0';
}

template<typename T>
void write(TSSM::GuestPtr<T> address, T& value)
{
    cpu_memory_rw_debug(qemu_get_cpu(0), address.value, &value, sizeof(T), true);
}

// In most cases, you aren't going to be writing back a whole object.
template<typename T, typename TField>
void write_field(TSSM::GuestPtr<T> address, T& object, TField T::* field)
{
    // A hack as old as time honestly.
    auto offset = reinterpret_cast<uintptr_t>(std::addressof((reinterpret_cast<T*>(0))->*field));

    cpu_memory_rw_debug(qemu_get_cpu(0), address.value + offset, &((&object)->*field), sizeof(TField), true);
}

namespace TSSM
{
Interface s_interface;

void Interface::Overlay::draw()
{
    auto window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    const auto PAD = 10.0f;
    const auto viewport = ImGui::GetMainViewport();
    const auto& work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
    const auto& work_size = viewport->WorkSize;

    if (m_position <= 3)
    {
        window_flags |= ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

        ImVec2 window_pos{
            (m_position & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD),
            (m_position & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD),
        };

        ImVec2 window_pos_pivot{
            (m_position & 1) ? 1.0f : 0.0f,
            (m_position & 2) ? 1.0f : 0.0f,
        };

        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }

    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

    if (ImGui::Begin(name(), nullptr, window_flags))
    {
        if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
            ImGui::SetTooltip("right-click to change position");

        draw_contents();

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Top-left", nullptr, m_position == 0))
                m_position = 0;
            if (ImGui::MenuItem("Top-right", nullptr, m_position == 1))
                m_position = 1;
            if (ImGui::MenuItem("Bottom-left", nullptr, m_position == 2))
                m_position = 2;
            if (ImGui::MenuItem("Bottom-right", nullptr, m_position == 3))
                m_position = 3;
            if (ImGui::MenuItem("Float", nullptr, m_position == 4))
                m_position = 4;

            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

void Interface::PlayerOverlay::draw_contents()
{
    auto player_ptr = read(Core::xGlobals::___player_ent_dont_use_directly);
    if (!player_ptr)
        return;

    auto player = read(player_ptr);

    if (player.frame)
    {
        auto frame = read(player.frame);

        ImGui::Text("Position %f, %f, %f", frame.mat.pos.x, frame.mat.pos.y, frame.mat.pos.z);
        ImGui::Text("Velocity %f, %f, %f", frame.vel.x, frame.vel.y, frame.vel.z);
    }
}

void Interface::BowlStorageOverlay::draw_contents()
{
    if (!read(Game::s_bubble_bowl_explosion_effect_active))
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No bowl active");
        return;
    }

    auto incrediball_ptr = read(Game::s_incrediball_model_instance);
    if (!incrediball_ptr)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Bowl active without model?");
        return;
    }

    auto incrediball = read(incrediball_ptr);

    if ((incrediball.Flags & 0x401) == 1)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Problematic flags 0x%x", incrediball.Flags);

        if (incrediball.BoneCount != 0)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                               "Problematic bone count %d -- relying "
                               "on frustum cull avoidance",
                               incrediball.BoneCount);

            ImGui::Separator();

            ImGui::BeginDisabled();
            ImGui::InputFloat3("Upper Animation Bounds", &incrediball.animBound.upper.x);
            ImGui::InputFloat3("Lower Animation Bounds", &incrediball.animBound.lower.x);
            ImGui::EndDisabled();

            ImGui::Separator();

            auto pressed_l3 = ImGui::IsKeyPressed(ImGuiKey_GamepadL3, false);
            if (ImGui::Button("Resolve", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)) || pressed_l3)
            {
                incrediball.Flags &= ~(0b0000'0001);
                write_field(incrediball_ptr, incrediball, &Core::xModelInstance::Flags);
            }
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "No issues likely storing this bowl");
    }
}

void Interface::draw_menu_item()
{
    if (ImGui::BeginMenu("TSSM"))
    {
        ImGui::MenuItem("Player", nullptr, &m_player_overlay_visible);
        ImGui::MenuItem("Bowl Storage", nullptr, &m_bowl_storage_overlay_visible);

        ImGui::EndMenu();
    }
}

void Interface::draw()
{
    ImGui::ShowDemoWindow();

    if (m_player_overlay_visible)
        m_player_overlay.draw();

    if (m_bowl_storage_overlay_visible)
        m_bowl_storage_overlay.draw();
}
}
