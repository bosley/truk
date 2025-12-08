#include "example/example.h"
#include <stdio.h>
#include <string.h>

int main(void) {
  const char *result = example_hello();

  if (result == NULL) {
    fprintf(stderr, "example_hello() returned NULL\n");
    return 1;
  }

  if (strlen(result) == 0) {
    fprintf(stderr, "example_hello() returned empty string\n");
    return 1;
  }

  printf("Test passed: example_hello() returned: %s\n", result);
  return 0;
}
