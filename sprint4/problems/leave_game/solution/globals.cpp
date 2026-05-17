#include "player_manager.h"
#include "request_handler.h"

namespace http_handler {
    bool g_has_tick_period = false;
    
    void SetTickPeriodMode(bool enabled) {
        g_has_tick_period = enabled;
    }
}

bool g_randomize_spawn_points = false;
