#include "player_manager.h"

bool g_randomize_spawn_points = false;
bool g_has_tick_period = false;

void SetTickPeriodMode(bool enabled) {
    g_has_tick_period = enabled;
}
