#ifndef MOCK_RN4871_H
#define MOCK_RN4871_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include "rn4871_api.h"

void mock_rn4871UartTxCb(char *buffer, RN4871_CODE_RETURN codeReturn);
void mock_rn4871UartRxCb(char *buffer, RN4871_CODE_RETURN codeReturn);

void mock_rn4871EnterCommandMode(RN4871_DEV *dev);
void mock_rn4871WaitReceivedData(RN4871_DEV *dev, char *mockReceivedData);
void mock_rn4871QuitCommandMode(RN4871_DEV *dev);
void mock_rn4871RebootModule(RN4871_DEV *dev);
void mock_rn4871IsOnTransparentUart(RN4871_DEV *dev);

#endif /* MOCK_RN4871_H */