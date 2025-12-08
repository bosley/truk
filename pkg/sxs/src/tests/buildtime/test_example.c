#include "sxs/sxs.h"
#include <stdio.h>
#include <string.h>

int main(void) {
  const char *result = sxs_hello();

  if (result == NULL) {
    fprintf(stderr, "sxs_hello() returned NULL\n");
    return 1;
  }

  if (strlen(result) == 0) {
    fprintf(stderr, "sxs_hello() returned empty string\n");
    return 1;
  }

  printf("sxs_hello() returned: %s\n", result);
  return 0;
}
