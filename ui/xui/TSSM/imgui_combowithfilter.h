#pragma once

#include <cstddef>

namespace ImGui
{
bool ComboWithFilter(const char* label, int* current_item, const char** items, int number_of_items,
                     int popup_max_height_in_items = -1);
}
