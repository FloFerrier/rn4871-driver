#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "test_integration.h"

int main() {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(transparentUart),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}