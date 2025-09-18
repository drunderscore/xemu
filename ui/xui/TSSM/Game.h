#pragma once

#include "GuestPtr.h"
#include "ui/xui/TSSM/Core.h"

namespace TSSM::Game
{
static constexpr GuestPtr<GuestPtr<Core::xModelInstance>> s_incrediball_model_instance = 0x317234;
static constexpr GuestPtr<bool> s_bubble_bowl_explosion_effect_active = 0x317238;
}
