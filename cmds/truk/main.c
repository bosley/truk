#include "screen/screen.h"
#include "sxs/sxs.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef TRUK_GIT_HASH
#define TRUK_GIT_HASH "unknown"
#endif

#ifndef TRUK_GIT_VERSION
#define TRUK_GIT_VERSION "unknown"
#endif

#ifndef TRUK_GIT_BRANCH
#define TRUK_GIT_BRANCH "unknown"
#endif

int main(void) {
  printf("truk build info:\n");
  printf("  version: %s\n", TRUK_GIT_VERSION);
  printf("  commit:  %s\n", TRUK_GIT_HASH);
  printf("  branch:  %s\n", TRUK_GIT_BRANCH);
  printf("\n");

  /*

// Uncommend out this block with asan to see some magic

  int *leak = malloc(sizeof(int) * 100);
  leak[0] = 42;
  printf("Leaked value: %d\n", leak[0]);

  int *x = malloc(sizeof(int) * 5);
  x[10] = 42;
  */

  const char *message = sxs_hello();
  printf("%s\n", message);

  printf("\nCreating 800x600 blue screen...\n");
  truk_screen *screen = truk_screen_create(800, 600);
  if (!screen) {
    fprintf(stderr, "Failed to create screen\n");
    return 1;
  }

  truk_screen_render(screen);

  printf("Screen created. Close the window or press Ctrl+C to exit.\n");

  SDL_Event event;
  int running = 1;
  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = 0;
      }
    }
    SDL_Delay(16);
  }

  truk_screen_destroy(screen);
  printf("Screen destroyed. Goodbye!\n");

  return 0;
}
