#include "example/example.h"
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

  const char *message = example_hello();
  printf("%s\n", message);
  return 0;
}
