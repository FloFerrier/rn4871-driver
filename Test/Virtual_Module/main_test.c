#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#include "test_virtual_module.h"

RN4871_CODE_RETURN rn4871UartTxCb(char *buf, uint16_t *len)
{
    uint16_t size = *len;
    check_expected(buf);
    check_expected(size);
    return mock_type(RN4871_CODE_RETURN);
}

RN4871_CODE_RETURN rn4871UartRxCb(char *buf, uint16_t *len)
{
	char *tmp;
    tmp = mock_type(char*);
    *len = (uint16_t) strlen(tmp);
	memcpy(buf, tmp, *len);
    return mock_type(RN4871_CODE_RETURN);
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
    RN4871_MODULE *dev = malloc(sizeof(RN4871_MODULE));
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
        cmocka_unit_test(test_virtualModuleInit),
        cmocka_unit_test(test_virtualModuleReceiveData),
        cmocka_unit_test(test_virtualModuleSendData),
        cmocka_unit_test_setup_teardown(test_virtualModuleConnect, setup, teardown),
        cmocka_unit_test_setup_teardown(test_virtualModuleStream, setup, teardown),
        cmocka_unit_test_setup_teardown(test_virtualModuleDisconnect, setup, teardown),
        cmocka_unit_test(test_virtualModuleSetForceDataMode),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}