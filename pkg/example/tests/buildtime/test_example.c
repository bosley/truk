#include "example/example.h"
#include "testlib/test.h"
#include <stdio.h>
#include <string.h>

int main(void) {
  const char *result = example_hello();

  ASSERT_NOT_NULL(result);
  ASSERT_NEQ(strlen(result), 0);

  printf("example_hello() returned: %s\n", result);
  return 0;
}
