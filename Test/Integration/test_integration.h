#ifndef TEST_INTEGRATION_H
#define TEST_INTEGRATION_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include "rn4871_api.h"
#include "rn4871_core.h"
#include "virtual_module.h"

void transparentUart(void **state);

#endif /* TEST_INTEGRATION_H */