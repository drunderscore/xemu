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
#include <cstdio>
#include <fstream>
#include <ios>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>

#include <cstdint>
// Must include cstdint before this.
#include "exec/vaddr.h"

// FIXME: Most all reads/writes result in TOCTOUs.
//        Some lock is held, but only during the individual operation.
//        For our sake, we actually want to be holding it more coarsely:
//        during all iteration, anytime we encounter any level of pointer indirection, etc.

extern "C"
{
    struct CPUState;

    CPUState* qemu_get_cpu(int index);

    int cpu_memory_rw_debug(CPUState* cpu, vaddr addr, void* ptr, size_t len, bool is_write);
}

template<typename T>
T read(TSSM::GuestPtr<T> address, unsigned index = 0)
{
    T value;

    if (cpu_memory_rw_debug(qemu_get_cpu(0), address.value + (index * sizeof(T)), &value, sizeof(T), false) != 0)
        value = {};

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

Interface::Interface() : m_scene_browser(m_hash_names)
{
    std::ifstream file("rainbow.txt");

    if (!file.is_open())
    {
        fprintf(stderr, "Unable to open rainbow.txt. Scene Browser experience may be sad :(\n");
        return;
    }

    std::string line;
    size_t line_number = 0;
    while (std::getline(file >> std::ws, line))
    {
        line_number++;

        // Empty lines are cool with me. Pound symbol # can be comments.
        // Organize the rainbow, taste the rainbow.
        if (line.empty() || line[0] == '#')
            continue;

        auto delimiter_position = line.find('=');
        if (delimiter_position == std::string::npos)
        {
            fprintf(stderr, "Malformed rainbow.txt missing delimiter on line %zu\n", line_number);
            continue;
        }

        const auto hash = line.substr(0, delimiter_position);
        const auto value = line.substr(delimiter_position + 1);

        unsigned int hash_value;
        std::stringstream ss;
        ss << std::hex << hash;

        if (!(ss >> hash_value))
        {
            fprintf(stderr, "Malformed rainbow.txt invalid hexadecimal on line %zu\n", line_number);
            continue;
        }

        // FIXME: My rainbow table actually does have some genuine duplicates due to letter case oddities.
        //        Not really a problem that hurts anyone right now?
        if (!m_hash_names.emplace(hash_value, std::move(value)).second)
            fprintf(stderr, "Suspicious rainbow.txt duplicate assignment on line %zu\n", line_number);
    }

    printf("Loaded %zu hash name mappings\n", m_hash_names.size());
}

void Interface::Window::draw(ImGuiWindowFlags window_flags)
{
    if (ImGui::Begin(name(), {}, window_flags))
        draw_contents();

    ImGui::End();
}

void Interface::Overlay::draw(ImGuiWindowFlags window_flags)
{
    const auto PAD = 10.0f;

    window_flags |= ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

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

    Window::draw(window_flags);
}

void Interface::Overlay::draw_contents()
{
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::BeginTooltip())
    {
        ImGui::Text("right-click to change position");

        if (m_position >= 4)
            ImGui::Text("left-click-drag to change position");

        ImGui::EndTooltip();
    }

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

void Interface::PlayerOverlay::draw_contents()
{
    Overlay::draw_contents();

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
    Overlay::draw_contents();

    if (!read(Game::s_bubble_bowl_explosion_effect_active))
    {
        ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f}, "No bowl active");
        return;
    }

    auto incrediball_ptr = read(Game::s_incrediball_model_instance);
    if (!incrediball_ptr)
    {
        ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f}, "Bowl active without model?");
        return;
    }

    auto incrediball = read(incrediball_ptr);

    if ((incrediball.Flags & 0x401) == 1)
    {
        ImGui::TextColored({1.0f, 0.5f, 0.0f, 1.0f}, "Problematic flags 0x%x", incrediball.Flags);

        if (incrediball.BoneCount != 0)
        {
            ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f},
                               "Problematic bone count %d -- relying "
                               "on frustum cull avoidance",
                               incrediball.BoneCount);

            ImGui::Separator();

            ImGui::BeginDisabled();
            ImGui::InputFloat3("Upper Animation Bounds", &incrediball.animBound.upper.x);
            ImGui::InputFloat3("Lower Animation Bounds", &incrediball.animBound.lower.x);
            ImGui::EndDisabled();
        }

        ImGui::Separator();

        auto pressed_l3 = ImGui::IsKeyPressed(ImGuiKey_GamepadL3, false);
        if (ImGui::Button("Resolve", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)) || pressed_l3)
        {
            incrediball.Flags &= ~(0b0000'0001);
            write_field(incrediball_ptr, incrediball, &Core::xModelInstance::Flags);
        }
    }
    else
    {
        ImGui::TextColored({0.0f, 1.0f, 0.0f, 1.0f}, "No issues likely storing this bowl");
    }
}

void Interface::draw_menu_item()
{
    if (ImGui::BeginMenu("TSSM"))
    {
        ImGui::MenuItem("Player", nullptr, &m_player_overlay_visible);
        ImGui::MenuItem("Bowl Storage", nullptr, &m_bowl_storage_overlay_visible);
        ImGui::MenuItem("Scene Browser", nullptr, &m_scene_browser_visible);

        ImGui::EndMenu();
    }
}

const char* Interface::SceneBrowser::name()
{
    char scene_name[5] = "None";

    auto scene_ptr = read(Core::xGlobals::sceneCur);
    if (scene_ptr)
    {
        auto scene = read(scene_ptr);

        if (scene._base.sceneID != 0)
        {
            scene_name[0] = static_cast<char>((scene._base.sceneID >> 24) & 0xFF);
            scene_name[1] = static_cast<char>((scene._base.sceneID >> 16) & 0xFF);
            scene_name[2] = static_cast<char>((scene._base.sceneID >> 8) & 0xFF);
            scene_name[3] = static_cast<char>(scene._base.sceneID & 0xFF);
        }
    }

    snprintf(m_window_name, sizeof(m_window_name), "Scene Browser %s###Scene Browser", scene_name);

    return m_window_name;
}

void Interface::SceneBrowser::draw_contents()
{
    auto scene_ptr = read(Core::xGlobals::sceneCur);
    if (!scene_ptr)
    {
        ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f}, "No scene active.");
        return;
    }

    auto scene = read(scene_ptr);

    if (scene.num_base <= 0)
    {
        ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f}, "Scene empty.");
        return;
    }

    // We're going to list all of our bases for selection, but we also need to do that
    // to actually find the selection, so remember that here -- we can use it by the end.
    // This also gives us a chance to invalidate the selection.
    std::optional<int> selected_index;

    if (ImGui::BeginChild("Bases", {}, ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders))
    {
        for (auto i = 0; i < scene.num_base; i++)
        {
            auto base_ptr = read(scene.base, i);
            if (!base_ptr)
                continue;

            auto base = read(base_ptr);

            ImGui::PushID(i);

            const char* name;
            std::string formatted_hash_string;

            // Yeah, we probably have a name for this hash...
            if (auto it = m_hash_names.find(base.id); it != m_hash_names.end())
            {
                name = it->second.c_str();
            }
            else // ...but be very reasonable if we happen to not -- format it as big hexadecimal.
            {
                std::stringstream ss;
                ss << std::hex << std::uppercase << base.id;
                ss >> formatted_hash_string;

                name = formatted_hash_string.c_str();
            }

            if (ImGui::Selectable(name, m_selected_id == base.id))
                m_selected_id = base.id;

            // That's our selection! We need that to give you the properties.
            if (base.id == m_selected_id)
                selected_index = i;

            ImGui::PopID();
        }
    }

    // We didn't find the selection by the ID, so reset the ID.
    if (!selected_index.has_value())
        m_selected_id = 0;

    ImGui::EndChild();

    if (m_selected_id == 0)
        return;

    ImGui::SameLine();

    if (ImGui::BeginChild("Properties", {}, ImGuiChildFlags_Borders)) {}

    ImGui::EndChild();
}

void Interface::draw()
{
    ImGui::ShowDemoWindow();

    if (m_player_overlay_visible)
        m_player_overlay.draw();

    if (m_bowl_storage_overlay_visible)
        m_bowl_storage_overlay.draw();

    if (m_scene_browser_visible)
        m_scene_browser.draw();
}
}
