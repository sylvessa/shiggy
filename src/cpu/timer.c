#include "globals.h"
#include "cpu/timer.h"

static void timer_callback() {
    static nat32 tick = 0;
    tick++;
}

void init_timer(nat32 freq) {
    register_interrupt_handler(32, timer_callback); // irq0

    nat32 divisor = 1193180 / freq;
    nat8 low = (nat8)low_8(divisor);
    nat8 high = (nat8)high_8(divisor);

    out_byte(0x43, 0x36);
    out_byte(0x40, low);
    out_byte(0x40, high);
}