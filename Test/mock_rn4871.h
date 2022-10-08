#ifndef MOCK_RN4871_H
#define MOCK_RN4871_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include "rn4871_api.h"

void mock_rn4871UartTxCb(char *buffer, uint8_t codeReturn);
void mock_rn4871UartRxCb(char *buffer, uint8_t codeReturn);

void mock_rn4871EnterCommandMode(struct rn4871_dev_s *dev);
void mock_rn4871WaitReceivedData(struct rn4871_dev_s *dev, char *mockReceivedData);
void mock_rn4871QuitCommandMode(struct rn4871_dev_s *dev);
void mock_rn4871RebootModule(struct rn4871_dev_s *dev);
void mock_rn4871IsOnTransparentUart(struct rn4871_dev_s *dev);

#endif /* MOCK_RN4871_H */