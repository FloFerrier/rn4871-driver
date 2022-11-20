#ifndef MOCK_RN4871_H
#define MOCK_RN4871_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include "rn4871_api.h"

#define BUFFER_SIZE_MAX 256

RN4871_CODE_RETURN rn4871UartTxCb(char *buf, uint16_t *len);
RN4871_CODE_RETURN rn4871UartRxCb(char *buf, uint16_t *len);
void rn4871LogSender(char *log, int len);
void rn4871DelayMsCb(uint32_t delay);

void mock_rn4871UartTxCb(char *buffer, RN4871_CODE_RETURN codeReturn);
void mock_rn4871UartRxCb(char *buffer, RN4871_CODE_RETURN codeReturn);

void mock_rn4871EnterCommandMode(RN4871_MODULE *dev);
void mock_rn4871WaitReceivedData(RN4871_MODULE *dev, char *mockReceivedData);
void mock_rn4871QuitCommandMode(RN4871_MODULE *dev);
void mock_rn4871RebootModule(RN4871_MODULE *dev);
void mock_rn4871IsOnTransparentUart(RN4871_MODULE *dev);

#endif /* MOCK_RN4871_H */