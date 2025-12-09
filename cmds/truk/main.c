#include "buffer/buffer.h"
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

void do_show_screen(void) {
  printf("\nCreating 800x600 blue screen...\n");
  truk_screen *screen = truk_screen_create(800, 600);
  if (!screen) {
    fprintf(stderr, "Failed to create screen\n");
    return;
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
}

#define HANDLE_ARG(long, short, action)                                        \
  if (strcmp(argv[idx], long) == 0 || strcmp(argv[idx], short) == 0) {         \
    action;                                                                    \
    return 0;                                                                  \
  }

#define HANDLE_COMMAND(name, action)                                           \
  if (strcmp(argv[idx], name) == 0) {                                          \
    action;                                                                    \
    return 0;                                                                  \
  }

void show_usage(char *program_name) {
  printf("Usage: %s [OPTIONS]\n", program_name);
  printf("Options:\n");
  printf("  --screen, -s      Show a blue screen\n");
  printf("  --help, -h        Show this help message\n");
  printf("Commands:\n");
  printf("  run <file>        Run a file\n");
  exit(0);
}

void show_version(char *program_name) {
  printf("Version: %s\n", program_name);
  printf("  commit:  %s\n", TRUK_GIT_HASH);
  printf("  branch:  %s\n", TRUK_GIT_BRANCH);
  printf("  version: %s\n", TRUK_GIT_VERSION);
  exit(0);
}

int print_byte(uint8_t *byte, size_t idx, void *callback_data) {
  printf("%02x ", *byte);
  return 1;
}

// ./truk run <file>
//
void command_run(char *filename) {
  if (!filename) {
    fprintf(stderr, "Error: run command requires a file\n");
    return;
  }

  slp_buffer_t *buffer = slp_buffer_from_file(filename);
  if (!buffer) {
    fprintf(stderr, "Error: failed to load file: %s\n", filename);
    return;
  }

  slp_buffer_for_each(buffer, print_byte, NULL);
}

int main(int argc, char *argv[]) {
  int idx = 1;
  while (idx < argc) {
    HANDLE_ARG("--help", "-h", { show_usage(argv[0]); });
    HANDLE_ARG("--screen", "-s", { do_show_screen(); });
    HANDLE_ARG("--version", "--version", { // dont trample -v for version
      show_version(argv[0]);
    });
    HANDLE_COMMAND("run", {
      if (++idx >= argc) {
        fprintf(stderr, "Error: run command requires a file\n");
        return 1;
      }
      command_run(argv[idx]);
    });
    idx++;
  }

  fprintf(stderr, "Error: unknown command: %s\n", argv[idx]);
  show_usage(argv[0]);

  return 0;
}
