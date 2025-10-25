#include "globals.h"
#include "cpu/timer.h"
#include "apps/gfx.h"

nat32 timer_ticks = 0;

static void timer_callback() {
    timer_ticks++;
    timer_vga_callback();
    gfx_app_timer_callback();
}

void sleep_ms(nat32 ms) {
	nat32 start = timer_ticks;
	nat32 ticks_to_wait = ms / 20;
	if(ticks_to_wait == 0) ticks_to_wait = 1;
	while(timer_ticks - start < ticks_to_wait) {}
}

void sleep_s(nat32 seconds) {
	sleep_ms(seconds * 1000);
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