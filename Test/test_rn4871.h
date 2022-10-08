#ifndef TEST_RN4871_H
#define TEST_RN4871_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include "rn4871_api.h"

#define BUFFER_SIZE_MAX 256

uint8_t rn4871UartTxCb(char *buf, uint16_t *len);
uint8_t rn4871UartRxCb(char *buf, uint16_t *len);
void rn4871DelayMsCb(uint32_t delay);

void test_rn4871Init(void **state);
void test_rn4871WaitReceivedData(void **state);
void test_rn4871EnterCommandMode(void **state);
void test_rn4871QuitCommandMode(void **state);
void test_rn4871RebootModule(void **state);
void test_rn4871SetServices(void **state);
void test_rn4871SetDeviceName(void **state);
void test_rn4871EraseAllGattServices(void **state);
void test_rn4871GetServices(void **state);
void test_rn4871GetDeviceName(void **state);
void test_rn4871GetFirmwareVersion(void **state);
void test_rn4871DumpInfos(void **state);
void test_rn4871GetMacAddress(void **state);
void test_rn4871IsOnTransparentUart(void **state);
void test_rn4871TransparentUartSendData(void **state);
void test_rn4871GetFsmState(void **state);
void test_rn4871SetForceFsmState(void **state);
void test_rn4871SetForceDataMode(void **state);
void test_rn4871GetErrorCodeStr(void **state);

#endif /* TEST_RN4871_H */