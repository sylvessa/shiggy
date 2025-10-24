#include "globals.h"
#include "drivers/pcspeaker.h"

#define PIT_CTRL 0x43
#define PIT_CHANNEL2 0x42
#define SPEAKER_CTRL 0x61
#define PIT_BASE_FREQ 1193180

void pc_speaker_play(nat32 frequency) {
	if (frequency == 0) return;

	nat16 divisor = (nat16)(PIT_BASE_FREQ / frequency);

	out_byte(PIT_CTRL, 0xB6); // channel 2, lobyte/hibyte, mode 3 (square wave)
	out_byte(PIT_CHANNEL2, divisor & 0xFF);
	out_byte(PIT_CHANNEL2, (divisor >> 8) & 0xFF);

	nat8 tmp = in_byte(SPEAKER_CTRL);
	if ((tmp & 3) != 3) {
		out_byte(SPEAKER_CTRL, tmp | 3); // enable speaker
	}
}

void pc_speaker_stop() {
	nat8 tmp = in_byte(SPEAKER_CTRL) & 0xFC; // clear bits 0 and 1
	out_byte(SPEAKER_CTRL, tmp);
}

void pc_speaker_beep(nat32 frequency, nat32 duration_ms) {
	pc_speaker_play(frequency);
	for (nat32 i = 0; i < duration_ms * 50000; i++) __asm__ volatile("nop");
	pc_speaker_stop();
}

