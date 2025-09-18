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

namespace TSSM
{
class Interface
{
private:
    class Overlay
    {
    public:
        void draw();

    protected:
        virtual const char* name() const = 0;
        virtual void draw_contents() = 0;

    private:
        unsigned int m_position{};
    };

    class PlayerOverlay : public Overlay
    {
    protected:
        const char* name() const override { return "Player"; }
        void draw_contents() override;
    };

    class BowlStorageOverlay : public Overlay
    {
    protected:
        const char* name() const override { return "Bowl Storage"; }
        void draw_contents() override;
    };

public:
    void draw_menu_item();

    void draw();

private:
    PlayerOverlay m_player_overlay;
    bool m_player_overlay_visible{};
    BowlStorageOverlay m_bowl_storage_overlay;
    bool m_bowl_storage_overlay_visible{};
};

extern Interface s_interface;
}
