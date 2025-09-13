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

using uchar = unsigned char;
struct xGrid;

struct xGridBound {
    guest_ptr<void> data;
    ushort gx;
    ushort gz;
    uchar oversize;
    uchar deleted;
    uchar gpad;
    uchar pad;
    guest_ptr<xGrid> grid;
    guest_ptr<guest_ptr<xGridBound>> head;
    guest_ptr<xGridBound> next;
};

struct xVec3 {
    float x;
    float y;
    float z;
};

struct xQCData {
    char xmin;
    char ymin;
    char zmin;
    char zmin_dup;
    char xmax;
    char ymax;
    char zmax;
    char zmax_dup;
    xVec3 min;
    xVec3 max;
};

struct xSphere {
    xVec3 center;
    float r;
};

struct xBox {
    // total size: 0x18
    xVec3 upper; // offset 0x0, size 0xC
    xVec3 lower; // offset 0xC, size 0xC
};

struct xCylinder {
    xVec3 center;
    float r;
    float h;
};

struct xBBox {
    xVec3 center;
    xBox box;
};

union _union_151 {
    xSphere sph;
    xBBox box;
    xCylinder cyl;
};

struct xMat3x3 {
    xVec3 right; // offset 0x0, size 0xC
    signed int flags; // offset 0xC, size 0x4
    xVec3 up; // offset 0x10, size 0xC
    unsigned int pad1; // offset 0x1C, size 0x4
    xVec3 at; // offset 0x20, size 0xC
    unsigned int pad2; // offset 0x2C, size 0x4
};

struct xMat4x3 : xMat3x3 {
    xVec3 pos; // offset 0x30, size 0xC
    unsigned int pad3; // offset 0x3C, size 0x4
};

struct xBound {
    xQCData qcd;
    uchar type;
    uchar pad[3];
    _union_151 field3_0x24;
    guest_ptr<xMat4x3> mat;
};

struct xEntAsset;

enum BaseFlags : U16 {
    Enabled = 1 << 0,
    Persistent = 1 << 1,
    Valid = 1 << 2,
    VisibleDuringCutscenes = 1 << 3,
    ReceiveShadows = 1 << 4,
};

struct xColor_tag {
    unsigned char r; // offset 0x0, size 0x1
    unsigned char g; // offset 0x1, size 0x1
    unsigned char b; // offset 0x2, size 0x1
    unsigned char a; // offset 0x3, size 0x1
};

struct _xFadeData {
    // total size: 0x18
    unsigned char active; // offset 0x0, size 0x1
    unsigned char hold_at_dest; // offset 0x1, size 0x1
    xColor_tag src; // offset 0x2, size 0x4
    xColor_tag dest; // offset 0x6, size 0x4
    float time_passed; // offset 0xC, size 0x4
    float time_total; // offset 0x10, size 0x4
    xColor_tag current_color; // offset 0x14, size 0x4
};

struct xModelPipe {
    // total size: 0x8
    unsigned int Flags; // offset 0x0, size 0x4
    unsigned char Layer; // offset 0x4, size 0x1
    unsigned char AlphaDiscard; // offset 0x5, size 0x1
    unsigned short PipePad; // offset 0x6, size 0x2
};

typedef void xModelPool;
typedef void xAnimPlay;
typedef void RpAtomic;
typedef void xSurface;
typedef void xModelBucket;
typedef void xLightKit;
typedef void RwMatrixTag;

struct xModelInstance {
    // total size: 0xA4
    guest_ptr<xModelInstance> Next; // offset 0x0, size 0x4
    guest_ptr<xModelInstance> Parent; // offset 0x4, size 0x4
    guest_ptr<xModelPool> Pool; // offset 0x8, size 0x4
    guest_ptr<xAnimPlay> Anim; // offset 0xC, size 0x4
    guest_ptr<RpAtomic> Data; // offset 0x10, size 0x4
    struct xModelPipe Pipe; // offset 0x14, size 0x8
    unsigned char InFrustum; // offset 0x1C, size 0x1
    unsigned char TrueClip; // offset 0x1D, size 0x1
    signed char sortBias; // offset 0x1E, size 0x1
    unsigned char modelpad; // offset 0x1F, size 0x1
    float RedMultiplier; // offset 0x20, size 0x4
    float GreenMultiplier; // offset 0x24, size 0x4
    float BlueMultiplier; // offset 0x28, size 0x4
    float Alpha; // offset 0x2C, size 0x4
    float FadeStart; // offset 0x30, size 0x4
    float FadeEnd; // offset 0x34, size 0x4
    guest_ptr<xSurface> Surf; // offset 0x38, size 0x4
    guest_ptr<guest_ptr<xModelBucket>> Bucket; // offset 0x3C, size 0x4
    guest_ptr<xModelInstance> BucketNext; // offset 0x40, size 0x4
    guest_ptr<xLightKit> LightKit; // offset 0x44, size 0x4
    guest_ptr<void> Object; // offset 0x48, size 0x4
    unsigned short Flags; // offset 0x4C, size 0x2
    unsigned char BoneCount; // offset 0x4E, size 0x1
    unsigned char BoneIndex; // offset 0x4F, size 0x1
    guest_ptr<unsigned char> BoneRemap; // offset 0x50, size 0x4
    guest_ptr<RwMatrixTag> Mat; // offset 0x54, size 0x4
    struct xVec3 Scale; // offset 0x58, size 0xC
    struct xBox animBound; // offset 0x64, size 0x18
    struct xBox combinedAnimBound; // offset 0x7C, size 0x18
    unsigned int modelID; // offset 0x94, size 0x4
    unsigned int shadowID; // offset 0x98, size 0x4
    guest_ptr<RpAtomic> shadowmapAtomic; // offset 0x9C, size 0x4
    struct /* @struct */ {
        // total size: 0x4
        guest_ptr<xVec3> verts; // offset 0x0, size 0x4
    } anim_coll; // offset 0xA0, size 0x4
};

struct xBaseAsset {
    unsigned int id; // offset 0x0, size 0x4
    unsigned char baseType; // offset 0x4, size 0x1
    unsigned char linkCount; // offset 0x5, size 0x1
    unsigned short baseFlags; // offset 0x6, size 0x2
};

struct xDynAsset {
    xBaseAsset base;
    unsigned int type; // offset 0x8, size 0x4
    unsigned short version; // offset 0xC, size 0x2
    unsigned short handle; // offset 0xE, size 0x2
};

struct asset_type {
    xDynAsset base;
    unsigned char persistent : 8; // offset 0x10, size 0x1
    unsigned char loop : 8; // offset 0x11, size 0x1
    unsigned char enable : 8; // offset 0x12, size 0x1
    unsigned char retry : 8; // offset 0x13, size 0x1
    unsigned int talk_box; // offset 0x14, size 0x4
    unsigned int next_task; // offset 0x18, size 0x4
    unsigned int stages[6]; // offset 0x1C, size 0x18
};

enum state_enum : int {
    STATE_INVALID = -1,
    STATE_BEGIN = 0,
    STATE_DESCRIPTION = 1,
    STATE_REMINDER = 2,
    STATE_SUCCESS = 3,
    STATE_FAILURE = 4,
    STATE_END = 5,
    MAX_STATE = 6,
};

struct xEntFrame;
struct xEntCollis;
struct xFFX;
struct xShadowSimpleCache;
struct xEntShadow;
struct anim_coll_data;

struct xRot {
    xVec3 axis; // offset 0x0, size 0xC
    float angle; // offset 0xC, size 0x4
};

struct xEntFrame {
    xMat4x3 mat; // offset 0x0, size 0x40
    xMat4x3 oldmat; // offset 0x40, size 0x40
    xVec3 oldvel; // offset 0x80, size 0xC
    xRot oldrot; // offset 0x8C, size 0x10
    xRot drot; // offset 0x9C, size 0x10
    xRot rot; // offset 0xAC, size 0x10
    xVec3 dvel; // offset 0xBC, size 0xC
    xVec3 vel; // offset 0xC8, size 0xC
    unsigned int mode; // offset 0xD4, size 0x4
    xVec3 dpos; // offset 0xD8, size 0xC
};

struct xEnt {
    xBase base;
    guest_ptr<xEntAsset> asset;
    ushort idx;
    uchar flags;
    uchar miscflags;
    uchar subType;
    uchar pflags;
    ushort moreFlags;
    uchar _isCulled : 2;
    uchar collisionEventReceived : 2;
    uchar driving_count;
    uchar num_ffx;
    uchar collType;
    uchar collLev;
    uchar chkby;
    uchar penby;
    // void (*visUpdate)(struct xEnt *);
    guest_ptr<void> visUpdate;
    guest_ptr<xModelInstance> model;
    guest_ptr<xModelInstance> collModel;
    guest_ptr<xModelInstance> camcollModel;
    // void (*update)(struct xEnt *, struct xScene *, float);
    // void (*endUpdate)(struct xEnt *, struct xScene *, float);
    // void (*bupdate)(struct xEnt *, struct xVec3 *);
    // void (*move)(struct xEnt *, struct xScene *, float, struct xEntFrame *);
    // void (*render)(struct xEnt *);
    guest_ptr<void> update;
    guest_ptr<void> endUpdate;
    guest_ptr<void> bupdate;
    guest_ptr<void> move;
    guest_ptr<void> render;
    guest_ptr<xEntFrame> frame;
    guest_ptr<xEntCollis> collis;
    xGridBound gridb;
    xBound bound;
    // void (*transl)(struct xEnt *, struct xVec3 *, struct xMat4x3 *);
    guest_ptr<void> transl;
    guest_ptr<xFFX> ffx;
    guest_ptr<xEnt> driver;
    guest_ptr<xEnt> driven;
    int driveMode;
    guest_ptr<xShadowSimpleCache> simpShadow;
    guest_ptr<xEntShadow> entShadow;
    guest_ptr<anim_coll_data> anim_coll;
    guest_ptr<void> user_data;
};

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

const char *task_box_state_names[] = { "Begin",   "Description", "Reminder",
                                       "Success", "Failure",     "End" };

struct ztaskbox {
    xBase base;
    // there is some kind of anonymous struct here called "flag", but thats so
    // stupid i'm removing it. fucks everyone up.
    unsigned char enabled;
    unsigned char running;
    unsigned short dummy;
    int pad;
    guest_ptr<asset_type> asset; // offset 0x14, size 0x4
    state_enum state; // offset 0x18, size 0x4
    guest_ptr<void> cb; // offset 0x1C, size 0x4
    guest_ptr<ztaskbox> current; // offset 0x20, size 0x4
};

struct xTimerAsset {
    xBaseAsset base;
    float seconds; // offset 0x8, size 0x4
    float randomRange; // offset 0xC, size 0x4
};

struct xTimer {
    xBase base;
    guest_ptr<xTimerAsset> tasset; // offset 0x10, size 0x4
    unsigned char state; // offset 0x14, size 0x1
    unsigned char runsInPause; // offset 0x15, size 0x1
    unsigned short flags; // offset 0x16, size 0x2
    float secondsLeft; // offset 0x18, size 0x4
};

struct xCounterAsset {
    xBaseAsset base;
    signed short count; // offset 0x8, size 0x2
};

struct _xCounter {
    xBase base;
    guest_ptr<xCounterAsset> asset; // offset 0x10, size 0x4
    signed short count; // offset 0x14, size 0x2
    unsigned char state; // offset 0x16, size 0x1
    unsigned char pad; // offset 0x17, size 0x1
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
                    if (ImGui::BeginTable("Depth", 2,
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
    sprintf(text_buffer, "Scene %s###Scene", scene_id);

    if (ImGui::Begin(text_buffer)) {
        auto content_region_available_at_start = ImGui::GetContentRegionAvail();

        if (ImGui::BeginChild(
                "Bases",
                ImVec2(0, content_region_available_at_start.y * 0.8f))) {
            if (ImGui::BeginTable("Bases", 4,
                                  ImGuiTableFlags_RowBg |
                                      ImGuiTableFlags_Borders)) {
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
                    sprintf(text_buffer, "0x%x##ID", base.id);

                    if (ImGui::Selectable(text_buffer, false)) {
                        m_selected_base_index = i;
                        ImGui::OpenPopup("SelectedBaseIDPopup");
                    }

                    if (ImGui::BeginPopup("SelectedBaseIDPopup")) {
                        if (m_selected_base_index < scene.num_base) {
                            auto base_ptr =
                                read_guest_infallible<guest_ptr<xBase>>(
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
                    sprintf(text_buffer, "0x%x##BaseFlags", base.baseFlags);

                    if (ImGui::Selectable(text_buffer, false)) {
                        m_selected_base_index = i;
                        ImGui::OpenPopup("SelectedBaseFlagsPopup");
                    }


                    if (ImGui::BeginPopup("SelectedBaseFlagsPopup")) {
                        if (m_selected_base_index < scene.num_base) {
                            auto base_ptr =
                                read_guest_infallible<guest_ptr<xBase>>(
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

        ImGui::EndChild();

        if (ImGui::BeginChild(
                "Properties",
                ImVec2(0, content_region_available_at_start.y * 0.2f))) {
            ImGui::Text("hello");
        }

        ImGui::EndChild();
    }

    ImGui::End();

    if (ImGui::Begin("Special")) {
        constexpr const char *game_mode_names[] = {
            "Boot",  "Intro",    "Title",          "Start",
            "Load",  "Options",  "Save",           "Pause",
            "Stall", "WorldMap", "MonsterGallery", "ConceptArtGallery",
            "Game"
        };

        auto game_mode = read_guest_infallible<U32>(0x00309484);
        ImGui::Text("gGameMode: %d", game_mode);

        if (game_mode < sizeof(game_mode_names)) {
            ImGui::SameLine();
            ImGui::Text("%s", game_mode_names[game_mode]);
        }

        auto game_state = read_guest_infallible<U32>(0x002b5380);
        ImGui::Text("gGameState: %d", game_state);

        ImGui::Separator();

        auto time_current = read_guest_infallible<U32>(0x00309340);
        ImGui::Text("sTimeCurrent: %d", time_current);

        auto time_elapsed = read_guest_infallible<float>(0x00309330);
        ImGui::Text("sTimeElapsed: %f", time_elapsed);

        auto real_time_elapsed = read_guest_infallible<float>(0x00309334);
        ImGui::Text("sRealTimeElapsed: %f", real_time_elapsed);

        auto gloop_ct = read_guest_infallible<U32>(0x003093d0);
        ImGui::Text("gloop_ct: %d", gloop_ct);

        m_frame_times.push_back(time_elapsed);
        if (m_frame_times.size() > 60 * 10)
            m_frame_times.erase(m_frame_times.begin());

        ImGui::PlotLines("Frame Time", m_frame_times.data(),
                         m_frame_times.size());

        ImGui::Separator();

        auto fade = read_guest_infallible<_xFadeData>(0x00455418);
        ImGui::Text("Fade: %s", fade.active ? "true" : "false");
        ImGui::Text("Hold: %d", fade.hold_at_dest);
        ImGui::Text("Remaining: %f", fade.time_total - fade.time_passed);

        // ___player_ent_dont_use_directly
        if (auto player_entity_ptr =
                read_guest_infallible<guest_ptr<xEnt>>(0x002afe28)) {
            ImGui::Text("Player: 0x%x", player_entity_ptr);

            auto player_entity = read_guest_infallible<xEnt>(player_entity_ptr);
            ImGui::Text("Frame: 0x%x", player_entity.frame);

            auto frame = read_guest_infallible<xEntFrame>(player_entity.frame);

            ImGui::Text("%f, %f, %f", frame.mat.pos.x, frame.mat.pos.y,
                        frame.mat.pos.z);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No player");
        }
    }

    ImGui::End();

    if (ImGui::Begin("Bowl Storage", {}, ImGuiWindowFlags_AlwaysAutoResize)) {
        auto incrediball_ptr =
            read_guest_infallible<guest_ptr<xModelInstance>>(0x00317234);
        ImGui::Text("Model pointer: 0x%x", incrediball_ptr);

        ImGui::Separator();

        auto active = read_guest_infallible<bool>(0x00317238);
        if (!active) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                               "No bowl active");
        } else {
            auto incrediball =
                read_guest_infallible<xModelInstance>(incrediball_ptr);

            if ((incrediball.Flags & 0x401) == 1) {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                                   "Problematic flags 0x%x", incrediball.Flags);

                if (incrediball.BoneCount != 0) {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                                       "Problematic bone count %d -- relying "
                                       "on frustum cull avoidance",
                                       incrediball.BoneCount);

                    ImGui::Separator();

                    if (ImGui::InputFloat3("Upper Animation Bounds",
                                           &incrediball.animBound.upper.x)) {
                        write_guest_infallible(incrediball_ptr + 0x64 + 0x0 +
                                                   0x0,
                                               incrediball.animBound.upper.x);
                        write_guest_infallible(incrediball_ptr + 0x64 + 0x0 +
                                                   0x4,
                                               incrediball.animBound.upper.y);
                        write_guest_infallible(incrediball_ptr + 0x64 + 0x0 +
                                                   0x8,
                                               incrediball.animBound.upper.z);
                    }

                    if (ImGui::InputFloat3("Lower Animation Bounds",
                                           &incrediball.animBound.lower.x)) {
                        write_guest_infallible(incrediball_ptr + 0x64 + 0xC +
                                                   0x0,
                                               incrediball.animBound.lower.x);
                        write_guest_infallible(incrediball_ptr + 0x64 + 0xC +
                                                   0x4,
                                               incrediball.animBound.lower.y);
                        write_guest_infallible(incrediball_ptr + 0x64 + 0xC +
                                                   0x8,
                                               incrediball.animBound.lower.z);
                    }

                    ImGui::Separator();
                    auto pressed_l3 =
                        ImGui::IsKeyPressed(ImGuiKey_GamepadL3, false);
                    if (ImGui::Button(
                            "Resolve",
                            ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)) ||
                        pressed_l3) {
                        write_guest_infallible(incrediball_ptr + 0x4C,
                                               incrediball.Flags &
                                                   ~(0b0000'0001));
                    }
                }
            } else {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                                   "No issues likely storing this bowl");
            }
        }
    }

    ImGui::End();

    if (ImGui::Begin("Funny Numbers")) {
        for (auto i = 0u; i < scene.num_base; i++) {
            auto base_ptr = read_guest_infallible<guest_ptr<xBase>>(
                scene.base + (sizeof(guest_ptr<xBase>) * i));

            auto base = read_guest_infallible<xBase>(base_ptr);

            if (base.baseType == eBaseTypeTaskBox) {
                ImGui::PushID(i);
                // We'll have to re-read this object from the guest, we didn't
                // read enough originally which means we've sliced it.
                auto task_box = read_guest_infallible<ztaskbox>(base_ptr);
                ImGui::Text("TaskBox @ 0x%x (asset 0x%x)", base_ptr, base.id);
                ImGui::Text("State 0x%x", task_box.state);
                if (task_box.state >= 0 && task_box.state < MAX_STATE) {
                    ImGui::SameLine();
                    ImGui::Text("%s", task_box_state_names[task_box.state]);
                }
                if (ImGui::InputInt("State",
                                    reinterpret_cast<int *>(&task_box.state))) {
                    write_guest_infallible(base_ptr + 32,
                                           static_cast<int>(task_box.state));
                }
                ImGui::Text("Enabled: %s", task_box.enabled ? "true" : "false");
                ImGui::Text("Running: %s", task_box.running ? "true" : "false");

                auto is_this_current = base_ptr == task_box.current;

                if (is_this_current)
                    ImGui::PushStyleColor(ImGuiCol_Text,
                                          ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                ImGui::Text("Current: 0x%x", task_box.current);
                if (is_this_current)
                    ImGui::PopStyleColor();

                auto task_box_asset =
                    read_guest_infallible<asset_type>(task_box.asset);

                ImGui::Text("Asset: Persistent: %d", task_box_asset.persistent);
                ImGui::Text("Asset: Loop: %d", task_box_asset.loop);
                ImGui::Text("Asset: Enable: %d", task_box_asset.enable);
                ImGui::Text("Asset: Retry: %d", task_box_asset.retry);
                ImGui::Text("Asset: Talk Box: 0x%x", task_box_asset.talk_box);
                ImGui::Text("Asset: Next Task: 0x%x", task_box_asset.next_task);

                for (auto j = 0; j < 6; j++)
                    ImGui::Text("Asset: Stage %s: 0x%x",
                                task_box_state_names[j],
                                task_box_asset.stages[j]);

                ImGui::Separator();
                ImGui::PopID();
            } else if (base.baseType == eBaseTypeTimer) {
                ImGui::Text("Timer @ 0x%x (asset 0x%x)", base_ptr, base.id);
                // We'll have to re-read this object from the guest, we didn't
                // read enough originally which means we've sliced it.
                auto timer = read_guest_infallible<xTimer>(base_ptr);
                ImGui::Text("Flags: 0x%x", timer.flags);
                ImGui::Text("Runs in Pause: %s",
                            timer.runsInPause ? "true" : "false");
                ImGui::Text("Seconds Left: %f", timer.secondsLeft);
                ImGui::Text("State: 0x%x", timer.state);

                auto timer_asset =
                    read_guest_infallible<xTimerAsset>(timer.tasset);
                ImGui::Text("Asset: Random Range: %f", timer_asset.randomRange);
                ImGui::Text("Asset: Seconds: %f", timer_asset.seconds);

                ImGui::Separator();
            } else if (base.baseType == eBaseTypeCounter) {
                ImGui::Text("Counter @ 0x%x (asset 0x%x)", base_ptr, base.id);
                // We'll have to re-read this object from the guest, we didn't
                // read enough originally which means we've sliced it.
                auto counter = read_guest_infallible<_xCounter>(base_ptr);
                ImGui::Text("Count: %d", counter.count);
                ImGui::Text("Pad?: %d", counter.pad);
                ImGui::Text("State: %d", counter.state);

                auto counter_asset =
                    read_guest_infallible<xCounterAsset>(counter.asset);
                ImGui::Text("Asset: Count: %d", counter_asset.count);

                ImGui::Separator();
            }
        }
    }

    ImGui::End();
}
