#ifndef TIMER_H
#define TIMER_H
#include "types.h"

void init_timer(nat32);
extern nat32 timer_ticks;
void sleep_s(nat32 seconds);
void sleep_ms(nat32 ms);

#endif