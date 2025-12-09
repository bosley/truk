#include "screen.h"
#include <SDL.h>
#include <stdlib.h>

struct truk_screen {
  uint32_t width;
  uint32_t height;
  SDL_Window *window;
  SDL_Renderer *renderer;
};

truk_screen *truk_screen_create(uint32_t width, uint32_t height) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    return NULL;
  }

  truk_screen *screen = malloc(sizeof(truk_screen));
  if (!screen) {
    SDL_Quit();
    return NULL;
  }

  screen->width = width;
  screen->height = height;

  screen->window =
      SDL_CreateWindow("truk", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       width, height, SDL_WINDOW_SHOWN);

  if (!screen->window) {
    free(screen);
    SDL_Quit();
    return NULL;
  }

  screen->renderer = SDL_CreateRenderer(
      screen->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!screen->renderer) {
    SDL_DestroyWindow(screen->window);
    free(screen);
    SDL_Quit();
    return NULL;
  }

  return screen;
}

int truk_screen_render(truk_screen *screen) {
  if (!screen || !screen->renderer) {
    return -1;
  }

  SDL_SetRenderDrawColor(screen->renderer, 0, 100, 200, 255);
  SDL_RenderClear(screen->renderer);
  SDL_RenderPresent(screen->renderer);

  return 0;
}

void truk_screen_destroy(truk_screen *screen) {
  if (!screen) {
    return;
  }

  if (screen->renderer) {
    SDL_DestroyRenderer(screen->renderer);
  }

  if (screen->window) {
    SDL_DestroyWindow(screen->window);
  }

  free(screen);
  SDL_Quit();
}
