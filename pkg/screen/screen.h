#ifndef TRUK_SCREEN_H
#define TRUK_SCREEN_H

#include <stdint.h>

typedef struct truk_screen truk_screen;

truk_screen *truk_screen_create(uint32_t width, uint32_t height);

int truk_screen_render(truk_screen *screen);

void truk_screen_destroy(truk_screen *screen);

#endif
