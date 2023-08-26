#ifndef TEST_INTEGRATION_H
#define TEST_INTEGRATION_H

// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on

#include "rn4871_api.h"
#include "rn4871_core.h"

void transparentUart(void **state);

#endif /* TEST_INTEGRATION_H */