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
#include "ui/xui/TSSM/Core.h"

namespace TSSM::Game
{
static constexpr GuestPtr<GuestPtr<Core::xModelInstance>> s_incrediball_model_instance = 0x317234;
static constexpr GuestPtr<bool> s_bubble_bowl_explosion_effect_active = 0x317238;
}
