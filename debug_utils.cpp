//
// Created by gregkwaste on 5/6/19.
//

#include "debug_utils.h"

void debug_printf(const char *fmt, ...){
#ifdef DEBUG
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
#else
    return;
#endif
}
