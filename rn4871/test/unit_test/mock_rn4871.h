#ifndef MOCK_RN4871_H
#define MOCK_RN4871_H

// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on

#include "rn4871_api.h"

void mock_rn4871UartTxCb(char *buffer, rn4871_error_e codeReturn);
void mock_rn4871UartRxCb(char *buffer, rn4871_error_e codeReturn);

void mock_rn4871EnterCommandMode(rn4871_module_s *dev);
void mock_rn4871WaitReceivedData(rn4871_module_s *dev, char *mockReceivedData);
void mock_rn4871QuitCommandMode(rn4871_module_s *dev);
void mock_rn4871RebootModule(rn4871_module_s *dev);
void mock_rn4871IsOnTransparentUart(rn4871_module_s *dev);

#endif /* MOCK_RN4871_H */