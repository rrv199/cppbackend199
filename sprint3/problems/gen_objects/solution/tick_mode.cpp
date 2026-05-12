#include "request_handler.h"

namespace http_handler {

bool g_has_tick_period = false;

void SetTickPeriodMode(bool enabled) {
    g_has_tick_period = enabled;
}

}
