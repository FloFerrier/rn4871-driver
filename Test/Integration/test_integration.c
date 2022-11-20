#include "test_integration.h"

#include <string.h>
#include <stdio.h>

VIRTUAL_MODULE g_virtual;

RN4871_CODE_RETURN rn4871UartTxCb(char *buf, uint16_t *len)
{
    assert((NULL != buf) || (NULL != len));

    if(0 < len)
    {
        //printf("[UART_TX] %s\r\n", buf);
        virtualModuleReceiveData(&g_virtual, buf);
    }

    return CODE_RETURN_SUCCESS;
}

RN4871_CODE_RETURN rn4871UartRxCb(char *buf, uint16_t *len)
{
    assert((NULL != buf) || (NULL != len));

	virtualModuleSendData(&g_virtual, buf, len);
    //printf("[UART_RX] %s\r\n", buf);

    return CODE_RETURN_SUCCESS;
}

void rn4871LogSender(char *log, int len)
{
	assert(NULL != log);

    if(0 < len)
    {
        printf("<RN4871-LOGGER> %s", log);
    }
}

void rn4871DelayMsCb(uint32_t delay)
{
    // nothing
}

void transparentUart(void **state)
{
    RN4871_MODULE module;
    module.delayMs = rn4871DelayMsCb;
    module.uartTx = rn4871UartTxCb;
    module.uartRx = rn4871UartRxCb;
    assert_int_equal(rn4871Init(&module), CODE_RETURN_SUCCESS);

    VIRTUAL_MODULE *virtual = &g_virtual;
    virtualModuleInit(virtual);
    assert_false(virtual->_command_mode);
    assert_int_equal(rn4871GetFsmState(&module), FSM_STATE_NONE);

    assert_int_equal(rn4871EnterCommandMode(&module), CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetFsmState(&module), FSM_STATE_INIT);
    assert_true(virtual->_command_mode);

    RN4871_CONFIG setConfig =
	{
		.deviceName = "RN4871-integration",
		.services = DEVICE_INFORMATION | UART_TRANSPARENT,
	};
    assert_int_equal(rn4871SetConfig(&module, &setConfig), CODE_RETURN_SUCCESS);

    RN4871_CONFIG getConfig;
    assert_int_equal(rn4871GetConfig(&module, &getConfig), CODE_RETURN_SUCCESS);
    assert_string_equal(virtual->moduleName, "RN4871-integration");
    assert_int_equal(virtual->services, DEVICE_INFORMATION | UART_TRANSPARENT);

    bool transparentUartMode = false;
    assert_int_equal(rn4871IsOnTransparentUart(&module, &transparentUartMode), CODE_RETURN_SUCCESS);
    assert_true(transparentUartMode);

    assert_int_equal(rn4871RebootModule(&module), CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetFsmState(&module), FSM_STATE_IDLE);
    assert_false(virtual->_command_mode);

    char receivedData[256] = "";
    uint16_t receivedDataLen = 0;
    assert_int_equal(virtualModuleConnect(virtual, &module), CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871WaitReceivedData(&module, receivedData, &receivedDataLen), CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetFsmState(&module), FSM_STATE_CONNECTED);

    assert_int_equal(virtualModuleStream(virtual, &module), CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871WaitReceivedData(&module, receivedData, &receivedDataLen), CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetFsmState(&module), FSM_STATE_STREAMING);

    char dataToSend[] = "Send an amount of data through transparent UART";
    uint16_t dataToSendLen = strlen(dataToSend);
    assert_int_equal(rn4871TransparentUartSendData(&module, dataToSend, dataToSendLen), CODE_RETURN_SUCCESS);
    assert_string_equal(virtual->_global_buffer, dataToSend);

    assert_int_equal(virtualModuleDisconnect(virtual, &module), CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871WaitReceivedData(&module, receivedData, &receivedDataLen), CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetFsmState(&module), FSM_STATE_IDLE);
}