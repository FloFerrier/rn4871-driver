#ifndef TEST_INTEGRATION_H
#define TEST_INTEGRATION_H

#include <cmocka.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "rn4871_api.h"
#include "rn4871_core.h"
#include "virtual_module.h"

void transparentUart(void **state);

#endif /* TEST_INTEGRATION_H */