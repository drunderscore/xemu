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

#include "tssm.hh"
#include "misc.hh"
#include <cstring>

extern "C" {
CPUState *qemu_get_cpu(int index);
#include "exec/vaddr.h"
int cpu_memory_rw_debug(CPUState *cpu, vaddr addr, void *ptr, size_t len,
                        bool is_write);
}

DebugTSSMWindow tssm_window;

using U8 = uint8_t;
using U32 = uint32_t;
using S32 = int32_t;
using U16 = uint16_t;
using S16 = int16_t;
using F32 = float;
// FIXME: Somehow a template could just take this value and spit out it's
//        object. No idea how to make it work though.
template <typename T> using guest_ptr = U32;

template <typename T>
bool ImGui::CheckboxFlagsT(const char *label, T *flags, T flags_value)
{
    bool all_on = (*flags & flags_value) == flags_value;
    bool any_on = (*flags & flags_value) != 0;
    bool pressed;
    if (!all_on && any_on) {
        ImGuiContext &g = *GImGui;
        g.NextItemData.ItemFlags |= ImGuiItemFlags_MixedValue;
        pressed = Checkbox(label, &all_on);
    } else {
        pressed = Checkbox(label, &all_on);
    }
    if (pressed) {
        if (all_on)
            *flags |= flags_value;
        else
            *flags &= ~flags_value;
    }
    return pressed;
}

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

// FIXME: Stolen from BFBB, not entirely accurate.
enum en_ZBASETYPE : U8 {
    eBaseTypeUnknown,
    eBaseTypeTrigger,
    eBaseTypeVillain,
    eBaseTypePlayer,
    eBaseTypePickup,
    eBaseTypeEnv,
    eBaseTypePlatform,
    eBaseTypeCamera,
    eBaseTypeDoor,
    eBaseTypeSavePoint,
    eBaseTypeItem,
    eBaseTypeStatic,
    eBaseTypeDynamic,
    eBaseTypeMovePoint,
    eBaseTypeTimer,
    eBaseTypeBubble,
    eBaseTypePortal,
    eBaseTypeGroup,
    eBaseTypePendulum,
    eBaseTypeUnk1,
    eBaseTypeFFX,
    eBaseTypeVFX,
    eBaseTypeCounter,
    eBaseTypeHangable,
    eBaseTypeButton,
    eBaseTypeProjectile,
    eBaseTypeSurface,
    eBaseTypeDestructObj,
    eBaseTypeGust,
    eBaseTypeVolume,
    eBaseTypeDispatcher,
    eBaseTypeCond,
    eBaseTypeUI,
    eBaseTypeUIFont,
    eBaseTypeProjectileType,
    eBaseTypeLobMaster,
    eBaseTypeFog,
    eBaseTypeLight,
    eBaseTypeParticleEmitter,
    eBaseTypeParticleSystem,
    eBaseTypeCutsceneMgr,
    eBaseTypeEGenerator,
    eBaseTypeScript,
    eBaseTypeNPC,
    eBaseTypeHud,
    eBaseTypeNPCProps,
    eBaseTypeParticleEmitterProps,
    eBaseTypeBoulder,
    eBaseTypeCruiseBubble,
    eBaseTypeTeleportBox,
    eBaseTypeBusStop,
    eBaseTypeTextBox,
    eBaseTypeTalkBox,
    eBaseTypeTaskBox,
    eBaseTypeBoulderGenerator,
    eBaseTypeNPCSettings,
    eBaseTypeDiscoFloor,
    eBaseTypeTaxi,
    eBaseTypeHUD_model,
    eBaseTypeHUD_font_meter,
    eBaseTypeHUD_unit_meter,
    eBaseTypeBungeeHook,
    eBaseTypeCameraFly,
    eBaseTypeTrackPhysics,
    eBaseTypeZipLine,
    eBaseTypeArena,
    eBaseTypeDuplicator,
    eBaseTypeLaserBeam,
    eBaseTypeTurret,
    eBaseTypeCameraTweak,
    eBaseTypeSlideProps,
    eBaseTypeHUD_text,
    eBaseTypeUnk20,
    eBaseTypeUnk21,
    eBaseTypeUnk22,
    eBaseTypeSFX,
    eBaseTypeUnk23,
    eBaseTypeUnk24,
    eBaseTypeUnk25,
    eBaseTypeUnk2,
    eBaseTypeUI_text,
    eBaseTypeUI_image,
    eBaseTypeUI_model,
    eBaseTypeUnk6,
    eBaseTypeUnk7,
    eBaseTypeUnk8,
    eBaseTypeUnk9,
    eBaseTypeUI_box,
    eBaseTypeUnk11,
    eBaseTypeUnk12,
    eBaseTypeUnk13,
    eBaseTypeUnk14,
    eBaseTypeUnk15,
    eBaseTypeRumble,
    eBaseTypeUnk16,
    eBaseTypeUnk18,
    eBaseTypeUnk19,
    eBaseTypeCount
};

const char *base_type_names[] = {
    "Unknown",
    "Trigger",
    "Villain",
    "Player",
    "Pickup",
    "Env",
    "Platform",
    "Camera",
    "Door",
    "SavePoint",
    "Item",
    "Static",
    "Dynamic",
    "MovePoint",
    "Timer",
    "Bubble",
    "Portal",
    "Group",
    "Pendulum",
    "Unk1",
    "FFX",
    "VFX",
    "Counter",
    "Hangable",
    "Button",
    "Projectile",
    "Surface",
    "DestructObj",
    "Gust",
    "Volume",
    "Dispatcher",
    "Cond",
    "UI",
    "UIFont",
    "ProjectileType",
    "LobMaster",
    "Fog",
    "Light",
    "ParticleEmitter",
    "ParticleSystem",
    "CutsceneMgr",
    "EGenerator",
    "Script",
    "NPC",
    "Hud",
    "NPCProps",
    "ParticleEmitterProps",
    "Boulder",
    "CruiseBubble",
    "TeleportBox",
    "BusStop",
    "TextBox",
    "TalkBox",
    "TaskBox",
    "BoulderGenerator",
    "NPCSettings",
    "DiscoFloor",
    "Taxi",
    "HUD_model",
    "HUD_font_meter",
    "HUD_unit_meter",
    "BungeeHook",
    "CameraFly",
    "TrackPhysics",
    "ZipLine",
    "Arena",
    "Duplicator",
    "LaserBeam",
    "Turret",
    "CameraTweak",
    "SlideProps",
    "HUD_text",
    "Unknown",
    "Unknown",
    "Unknown",
    "SFX",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "UI_text",
    "UI_image",
    "UI_model",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "UI_box",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Rumble",
    "Unknown",
    "Unknown",
    "Unknown",
};

using xLinkAsset = void;

struct xBase;
typedef S32 (*xBaseEventCB)(xBase *, xBase *, U32, const F32 *, xBase *);

struct xBase {
    U32 unk;
    U32 id;
    U8 baseType;
    U8 linkCount;
    U16 baseFlags;
    guest_ptr<xLinkAsset> link;
    guest_ptr<xBaseEventCB> eventFunc;
};

struct xEnt : xBase {};

struct xScene {
    U32 sceneID;
    U16 flags;
    U16 num_trigs;
    U16 num_stats;
    U16 num_dyns;
    U16 num_npcs;
    U16 num_act_ents;
    char _padding[24];
    guest_ptr<guest_ptr<xEnt>> trigs;
    guest_ptr<guest_ptr<xEnt>> stats;
    guest_ptr<guest_ptr<xEnt>> dyns;
    guest_ptr<guest_ptr<xEnt>> npcs;
    guest_ptr<guest_ptr<xEnt>> act_ents;
    char _padding2[48]; // FIXME: How long is this versus zScene?
};

struct zScene : xScene {
    U32 num_base;
    guest_ptr<guest_ptr<xBase>> base;
};

enum BaseFlags : U16 {
    Enabled = 1 << 0,
    Persistent = 1 << 1,
    Valid = 1 << 2,
    VisibleDuringCutscenes = 1 << 3,
    ReceiveShadows = 1 << 4,
};

const char *base_flag_names[] = { "Enabled", "Persistent", "Valid",
                                  "VisibleDuringCutscenes", "ReceiveShadows" };

struct {
    guest_ptr<U32> active_heap = 0x00413a3c;
    guest_ptr<xMemHeap_tag> gx_heap = 0x00413718;
    guest_ptr<zScene> scene = 0x002afe2c;
    guest_ptr<S32> mem_depth_just_hip_start = 0x002b6a9c;
    guest_ptr<S32> mem_depth_scene_start = 0x002b6a98;
    guest_ptr<S32> mem_depth_just_hip_start_player = 0x002b62ec;
} offsets;

template <typename T> static T read_guest_infallible(vaddr address)
{
    T value{};

    cpu_memory_rw_debug(qemu_get_cpu(0), address, &value, sizeof(value), false);

    return std::move(value);
}

template <typename T> static void write_guest_infallible(vaddr address, T value)
{
    cpu_memory_rw_debug(qemu_get_cpu(0), address, &value, sizeof(value), true);
}

DebugTSSMWindow::DebugTSSMWindow()
{
    m_memory_editor.ReadFn = [](const ImU8 *mem, size_t off,
                                void *user_data) -> ImU8 {
        return read_guest_infallible<ImU8>(reinterpret_cast<vaddr>(mem) + off);
    };

    m_memory_editor.WriteFn = [](ImU8 *mem, size_t off, ImU8 d,
                                 void *user_data) {
        write_guest_infallible(reinterpret_cast<vaddr>(mem) + off, d);
    };
}

void DebugTSSMWindow::Draw()
{
    if (!m_is_open)
        return;

    m_memory_editor.DrawWindow("Memory", 0x0, 1024 * 1024 * 64);

    if (ImGui::Begin("Heap", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        auto heap_0 = read_guest_infallible<xMemHeap_tag>(offsets.gx_heap);
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

    auto scene_ptr = read_guest_infallible<guest_ptr<zScene>>(offsets.scene);
    auto scene = read_guest_infallible<zScene>(scene_ptr);

    char scene_id[5] = {
        static_cast<char>((scene.sceneID >> 24) & 0xFF),
        static_cast<char>((scene.sceneID >> 16) & 0xFF),
        static_cast<char>((scene.sceneID >> 8) & 0xFF),
        static_cast<char>(scene.sceneID & 0xFF),
        '\0',
    };

    if (scene.sceneID == 0)
        strcpy(scene_id, "None");

    char text_buffer[64];
    sprintf(text_buffer, "Scene %s##Scene", scene_id);

    if (ImGui::Begin(text_buffer)) {
        if (ImGui::BeginTable(
                "Bases", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Links");
            ImGui::TableSetupColumn("Flags");
            ImGui::TableHeadersRow();

            for (auto i = 0u; i < scene.num_base; i++) {
                ImGui::PushID(i);
                auto base_ptr = read_guest_infallible<guest_ptr<xBase>>(
                    scene.base + (sizeof(guest_ptr<xBase>) * i));

                auto base = read_guest_infallible<xBase>(base_ptr);

                ImGui::TableNextColumn();
                sprintf(text_buffer, "0x%x", base.id);

                if (ImGui::Selectable(text_buffer, false)) {
                    m_selected_base_index = i;
                    ImGui::OpenPopup("SelectedBaseIDPopup");
                }

                if (ImGui::BeginPopup("SelectedBaseIDPopup")) {
                    if (m_selected_base_index < scene.num_base) {
                        auto base_ptr = read_guest_infallible<guest_ptr<xBase>>(
                            scene.base + (sizeof(guest_ptr<xBase>) *
                                          *m_selected_base_index));

                        if (ImGui::Selectable("Goto in memory"))
                            m_memory_editor.GotoAddr = base_ptr;
                    } else {
                        m_selected_base_index.reset();
                    }

                    ImGui::EndPopup();
                }

                ImGui::TableNextColumn();

                const char *base_type_name;
                if (base.baseType >= eBaseTypeCount)
                    base_type_name = base_type_names[eBaseTypeUnknown];
                else
                    base_type_name = base_type_names[base.baseType];
                ImGui::Text("%s", base_type_name);

                if (ImGui::BeginItemTooltip()) {
                    ImGui::Text("0x%x", base.baseType);
                    ImGui::EndTooltip();
                }

                ImGui::TableNextColumn();
                ImGui::Text("%d", base.linkCount);
                ImGui::TableNextColumn();
                sprintf(text_buffer, "0x%x", base.baseFlags);

                if (ImGui::Selectable(text_buffer, false)) {
                    m_selected_base_index = i;
                    ImGui::OpenPopup("SelectedBaseFlagsPopup");
                }


                if (ImGui::BeginPopup("SelectedBaseFlagsPopup")) {
                    if (m_selected_base_index < scene.num_base) {
                        auto base_ptr = read_guest_infallible<guest_ptr<xBase>>(
                            scene.base + (sizeof(guest_ptr<xBase>) *
                                          *m_selected_base_index));


                        auto base = read_guest_infallible<xBase>(base_ptr);

                        auto modified = false;

                        modified |= ImGui::CheckboxFlagsT<U16>(
                            "Enabled", &base.baseFlags, BaseFlags::Enabled);
                        modified |= ImGui::CheckboxFlagsT<U16>(
                            "Persistent", &base.baseFlags,
                            BaseFlags::Persistent);
                        modified |= ImGui::CheckboxFlagsT<U16>(
                            "Valid", &base.baseFlags, BaseFlags::Valid);
                        modified |= ImGui::CheckboxFlagsT<U16>(
                            "Visible During Cutscenes", &base.baseFlags,
                            BaseFlags::VisibleDuringCutscenes);
                        modified |= ImGui::CheckboxFlagsT<U16>(
                            "Receive Shadows", &base.baseFlags,
                            BaseFlags::ReceiveShadows);

                        if (modified)
                            write_guest_infallible(
                                reinterpret_cast<vaddr>(
                                    &reinterpret_cast<xBase *>(base_ptr)
                                         ->baseFlags),
                                base.baseFlags);
                    } else {
                        m_selected_base_index.reset();
                    }

                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }

    ImGui::End();
}
