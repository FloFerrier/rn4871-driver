#ifndef TEST_VIRTUAL_MODULE_H
#define TEST_VIRTUAL_MODULE_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include "virtual_module.h"

void test_virtualModuleInit(void **state);
void test_virtualModuleReceiveData(void **state);
void test_virtualModuleSendData(void **state);

#endif /* TEST_VIRTUAL_MODULE_H */