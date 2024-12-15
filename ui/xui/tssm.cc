#include "tssm.hh"
#include "misc.hh"

extern "C" {
CPUState *qemu_get_cpu(int index);
#include "exec/cpu-common.h"
}

DebugTSSMWindow tssm_window;

template <typename T> static T read_guest_infallible(vaddr address)
{
    T value{};

    cpu_memory_rw_debug(qemu_get_cpu(0), address, &value, sizeof(value), false);

    return value;
}

DebugTSSMWindow::DebugTSSMWindow()
{
    m_memory_editor.ReadFn = [](const ImU8 *mem, size_t off,
                                void *user_data) -> ImU8 {
        return read_guest_infallible<ImU8>(reinterpret_cast<vaddr>(mem) + off);
    };

    m_memory_editor.ReadOnly = true;
}

void DebugTSSMWindow::Draw()
{
    if (!m_is_open)
        return;

    m_memory_editor.DrawWindow("Memory", 0x0, 1024 * 1024 * 64);

    if (ImGui::Begin("Heap", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        auto heap_0 = read_guest_infallible<DebugTSSMWindow::xMemHeap_tag>(
            offsets.gx_heap);
        if (ImGui::BeginTabBar("Depths")) {
            char tab_title[8];

            auto just_hip_start_depth =
                read_guest_infallible<int>(offsets.mem_depth_just_hip_start);
            auto just_hip_start_player_depth = read_guest_infallible<int>(
                offsets.mem_depth_just_hip_start_player);
            auto scene_start_depth =
                read_guest_infallible<int>(offsets.mem_depth_scene_start);

            for (auto i = 0; i < 12; i++) {
                auto depth_not_yet_reached = i > heap_0.state_idx;

                sprintf(tab_title, "%d", i);
                auto state = heap_0.state[i];

                if (depth_not_yet_reached) {
                    ImGui::PushStyleColor(ImGuiCol_Tab, 0x600000FF);
                    ImGui::PushStyleColor(ImGuiCol_TabHovered, 0x600000FF);
                    ImGui::PushStyleColor(ImGuiCol_TabActive, 0x600000FF);
                    ImGui::PushStyleColor(ImGuiCol_TabUnfocused, 0x600000FF);
                    ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive,
                                          0x600000FF);
                }

                if (ImGui::BeginTabItem(tab_title)) {
                    if (ImGui::BeginTable("State", 2,
                                          ImGuiTableFlags_RowBg |
                                              ImGuiTableFlags_Borders)) {
                        ImGui::TableNextColumn();
                        ImGui::Text("Current");
                        ImGui::TableNextColumn();
                        ImGui::Text("0x%x", state.curr);

                        ImGui::TableNextColumn();
                        ImGui::Text("Block Count");
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", state.blk_ct);

                        ImGui::TableNextColumn();
                        ImGui::Text("Used");
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", state.used);

                        ImGui::TableNextColumn();
                        ImGui::Text("Wasted");
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", state.wasted);

                        ImGui::EndTable();
                    }

                    ImGui::EndTabItem();
                }

                if (i == just_hip_start_depth + 1) {
                    if (ImGui::BeginItemTooltip()) {
                        ImGui::Text("HIP Start");
                        ImGui::EndTooltip();
                    }
                } else if (i == just_hip_start_player_depth + 1) {
                    if (ImGui::BeginItemTooltip()) {
                        ImGui::Text("HIP Start Player");
                        ImGui::EndTooltip();
                    }
                } else if (i == scene_start_depth + 1) {
                    if (ImGui::BeginItemTooltip()) {
                        ImGui::Text("Scene Start");
                        ImGui::EndTooltip();
                    }
                }

                if (depth_not_yet_reached)
                    ImGui::PopStyleColor(5);
            }

            ImGui::EndTabBar();
        }
    }

    ImGui::End();
}
