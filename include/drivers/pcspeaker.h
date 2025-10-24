#ifndef PC_SPEAKER_H
#define PC_SPEAKER_H

#include "types.h"

void pc_speaker_play(nat32 frequency);
void pc_speaker_stop();
void pc_speaker_beep(nat32 frequency, nat32 duration_ms);

#endif
