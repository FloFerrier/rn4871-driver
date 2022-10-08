#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#include "test_rn4871.h"
#include "test_virtual_module.h"

uint8_t rn4871UartTxCb(char *buf, uint16_t *len)
{
    uint16_t size = *len;
    check_expected(buf);
    check_expected(size);
    return mock_type(uint8_t);
}

uint8_t rn4871UartRxCb(char *buf, uint16_t *len)
{
	char *tmp;
    tmp = mock_type(char*);
    *len = (uint16_t) strlen(tmp);
	memcpy(buf, tmp, *len);
    return mock_type(uint8_t);
}

void rn4871LogSender(char *log, int len)
{
	printf("%s", log);
}

void rn4871DelayMsCb(uint32_t delay)
{
    // Nothing
}

int setup(void **state)
{
    struct rn4871_dev_s *dev = malloc(sizeof(struct rn4871_dev_s));
    dev->delayMs = rn4871DelayMsCb;
	dev->uartRx = rn4871UartRxCb;
    dev->uartTx = rn4871UartTxCb;
    dev->_currentCmd = CMD_NONE;
    dev->_currentMode = DATA_MODE;
    dev->_fsmState = FSM_STATE_NONE;
    *state = dev;
    return 0;
}

int teardown(void **state)
{
    free(*state);
    return 0;
}

int main()
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_rn4871Init),
        cmocka_unit_test_setup_teardown(test_rn4871EnterCommandMode, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871WaitReceivedData, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871QuitCommandMode, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871RebootModule, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871SetServices, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871SetDeviceName, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871EraseAllGattServices, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871GetServices, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871GetDeviceName, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871GetFirmwareVersion, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871DumpInfos, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871GetMacAddress, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871IsOnTransparentUart, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871TransparentUartSendData, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871GetFsmState, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871SetForceFsmState, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rn4871SetForceDataMode, setup, teardown),
        cmocka_unit_test(test_rn4871GetErrorCodeStr),

        cmocka_unit_test(test_virtualModuleInit),
        cmocka_unit_test(test_virtualModuleReceiveData),
        cmocka_unit_test(test_virtualModuleSendData),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}