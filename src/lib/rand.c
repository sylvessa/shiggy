#include "globals.h"

static unsigned int rng_seed = 78962135;

unsigned int rand() {
	rng_seed = rng_seed * 1103515245 + 12345;
	return (rng_seed / 65536) % 32768;
}

float rand_float() {
	return rand() / 32768.0f;
}