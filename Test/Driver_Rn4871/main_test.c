#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#include "test_rn4871.h"

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
    RN4871_MODULE *module = malloc(sizeof(RN4871_MODULE));
    module->delayMs = rn4871DelayMsCb;
	module->uartRx = rn4871UartRxCb;
    module->uartTx = rn4871UartTxCb;
    rn4871Init(module);
    *state = module;
    return 0;
}

int teardown(void **state)
{
    RN4871_MODULE *module = *state;
    free(module);
    return 0;
}

int main()
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(tryInitModule_MissInterface),
        cmocka_unit_test(tryInitModule_Success),

        cmocka_unit_test_setup_teardown(receiveData_ModuleOnCmdMode, setup, teardown),
        cmocka_unit_test_setup_teardown(receiveData_RebootMsg, setup, teardown),
        cmocka_unit_test_setup_teardown(receiveData_ConnectMsg, setup, teardown),
        cmocka_unit_test_setup_teardown(receiveData_StreamingMsg, setup, teardown),
        cmocka_unit_test_setup_teardown(receiveData_DisconnectMsg, setup, teardown),
        cmocka_unit_test_setup_teardown(receiveData_UserMsg, setup, teardown),

        cmocka_unit_test_setup_teardown(test_sendCommand, setup, teardown),

        cmocka_unit_test_setup_teardown(tryEnterOnCmdMode_NoHardwareModule, setup, teardown),
        cmocka_unit_test_setup_teardown(tryEnterOnCmdMode_IncorrectResponse, setup, teardown),
        //cmocka_unit_test_setup_teardown(tryEnterOnCmdMode_Fail, setup, teardown),
        cmocka_unit_test_setup_teardown(tryEnterOnCmdMode_ModuleAlreadyOnCmdMode, setup, teardown),
        cmocka_unit_test_setup_teardown(tryEnterOnCmdMode_Success, setup, teardown),

        cmocka_unit_test_setup_teardown(tryQuitCmdMode_ModuleAlreadyOnDataMode, setup, teardown),
        cmocka_unit_test_setup_teardown(tryQuitCmdMode_Success, setup, teardown),

        cmocka_unit_test_setup_teardown(tryRebootModule_ModuleNotOnCmdMode, setup, teardown),
        cmocka_unit_test_setup_teardown(tryRebootModule_Success, setup, teardown),

        cmocka_unit_test_setup_teardown(trySetServices_ModuleNotOnCmdMode, setup, teardown),
        cmocka_unit_test_setup_teardown(trySetServices_UartTransparentSuccess, setup, teardown),

        cmocka_unit_test_setup_teardown(trySetDeviceName_ModuleNotOnCmdMode, setup, teardown),
        cmocka_unit_test_setup_teardown(trySetDeviceName_StringTooLong, setup, teardown),
        cmocka_unit_test_setup_teardown(trySetDeviceName_Success, setup, teardown),

        cmocka_unit_test_setup_teardown(trySetConfig_Success, setup, teardown),

        cmocka_unit_test_setup_teardown(tryGetFirmwareVersion_ModuleNotOnCmdMode, setup, teardown),
        cmocka_unit_test_setup_teardown(tryGetFirmwareVersion_Success, setup, teardown),

        cmocka_unit_test_setup_teardown(tryDumpInfos_ModuleNotOnCmdMode, setup, teardown),
        cmocka_unit_test_setup_teardown(tryDumpInfos_Success, setup, teardown),

        cmocka_unit_test_setup_teardown(tryGetServices_ModuleNotOnCmdMode, setup, teardown),
        cmocka_unit_test_setup_teardown(tryGetServices_Success, setup, teardown),

        cmocka_unit_test_setup_teardown(tryGetDeviceName_ModuleNotOnCmdMode, setup, teardown),
        cmocka_unit_test_setup_teardown(tryGetDeviceName_Success, setup, teardown),

        cmocka_unit_test_setup_teardown(tryGetMacAddress_ModuleNotOnCmdMode, setup, teardown),
        cmocka_unit_test_setup_teardown(tryGetMacAddress_Success, setup, teardown),

        cmocka_unit_test_setup_teardown(tryGetConfig_Success, setup, teardown),

        cmocka_unit_test_setup_teardown(checkTransparentUartMode_isTrue, setup, teardown),
        cmocka_unit_test_setup_teardown(checkTransparentUartMode_isFalse, setup, teardown),

        cmocka_unit_test_setup_teardown(trySendDataOnTransparentUart_ModuleNotOnDataMode, setup, teardown),
        cmocka_unit_test_setup_teardown(trySendDataOnTransparentUart_NoStreaming, setup, teardown),
        cmocka_unit_test_setup_teardown(trySendDataOnTransparentUart_Success, setup, teardown),

        cmocka_unit_test_setup_teardown(setForceFsmState_OutOfBounds, setup, teardown),
        cmocka_unit_test_setup_teardown(setForceFsmState_CorrectState, setup, teardown),

        cmocka_unit_test_setup_teardown(getFsmState_None, setup, teardown),
        cmocka_unit_test_setup_teardown(getFsmState_OnCmdMode, setup, teardown),
        cmocka_unit_test_setup_teardown(getFsmState_AfterReboot, setup, teardown),
        cmocka_unit_test_setup_teardown(getFsmState_AfterExternalBleConnection, setup, teardown),
        cmocka_unit_test_setup_teardown(getFsmState_AfterExternalBleStreaming, setup, teardown),
        cmocka_unit_test_setup_teardown(getFsmState_AfterExternalBleDisconnection, setup, teardown),

        cmocka_unit_test_setup_teardown(setForceDataMode_Success, setup, teardown),

        cmocka_unit_test_setup_teardown(getErrorCodeStr_OutOfBounds, setup, teardown),
        cmocka_unit_test_setup_teardown(getErrorCodeStr_CorrectCode, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}