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

#include "GuestPtr.h"
#include <map>
#include <memory>
#include <string>
#include <variant>

typedef int ImGuiWindowFlags;

struct ImNodesEditorContext;

namespace TSSM
{
namespace Core
{
struct xBase;
}

class Interface
{
private:
    class Window
    {
    public:
        virtual void draw(ImGuiWindowFlags = 0);

    protected:
        virtual const char* name() = 0;
        virtual void draw_contents() = 0;
    };

    class Overlay : public Window
    {
    public:
        virtual void draw(ImGuiWindowFlags = 0) override;

    protected:
        virtual void draw_contents() override;

    private:
        unsigned int m_position{};
    };

    class PlayerOverlay : public Overlay
    {
    protected:
        const char* name() override { return "Player"; }
        void draw_contents() override;
    };

    class BowlStorageOverlay : public Overlay
    {
    protected:
        const char* name() override { return "Bowl Storage"; }
        void draw_contents() override;
    };

    class SceneBrowser : public Window
    {
    public:
        explicit SceneBrowser(const std::map<unsigned int, std::string>& hash_names);
        virtual ~SceneBrowser();

    protected:
        const char* name() override;
        virtual void draw_contents() override;

    private:
        // Note: Our string_view must be null-terminated from here!
        std::variant<const char*, std::string> name_of_or_stringified_asset_id(int) const;

        void draw_properties(GuestPtr<Core::xBase>);
        void draw_links(GuestPtr<Core::xBase>);

        // 0 as empty should be okay!
        unsigned int m_selected_id{};

        const std::map<unsigned int, std::string>& m_hash_names;
        char m_window_name[64]{};
        ImNodesEditorContext* m_links_nodes_editor_context{};
    };

public:
    Interface();
    ~Interface();

    void draw_menu_item();

    void draw();

private:
    std::map<unsigned int, std::string> m_hash_names;
    PlayerOverlay m_player_overlay;
    bool m_player_overlay_visible{};
    BowlStorageOverlay m_bowl_storage_overlay;
    bool m_bowl_storage_overlay_visible{};
    std::unique_ptr<SceneBrowser> m_scene_browser;
    bool m_scene_browser_visible{};
};

extern Interface s_interface;
}
