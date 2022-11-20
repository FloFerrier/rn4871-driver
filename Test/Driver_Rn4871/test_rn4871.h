#ifndef TEST_RN4871_H
#define TEST_RN4871_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include "rn4871_api.h"
#include "rn4871_core.h"

#define BUFFER_SIZE_MAX 256

RN4871_CODE_RETURN rn4871UartTxCb(char *buf, uint16_t *len);
RN4871_CODE_RETURN rn4871UartRxCb(char *buf, uint16_t *len);
void rn4871DelayMsCb(uint32_t delay);

void tryInitModule_MissInterface(void **state);
void tryInitModule_Success(void **state);

void receiveData_ModuleOnCmdMode(void **state);
void receiveData_RebootMsg(void **state);
void receiveData_ConnectMsg(void **state);
void receiveData_StreamingMsg(void **state);
void receiveData_DisconnectMsg(void **state);
void receiveData_UserMsg(void **state);

void test_sendCommand(void **state);

void tryEnterOnCmdMode_NoHardwareModule(void **state);
void tryEnterOnCmdMode_IncorrectResponse(void **state);
void tryEnterOnCmdMode_Fail(void **state);
void tryEnterOnCmdMode_ModuleAlreadyOnCmdMode(void **state);
void tryEnterOnCmdMode_Success(void **state);

void tryQuitCmdMode_ModuleAlreadyOnDataMode(void **state);
void tryQuitCmdMode_Success(void **state);

void tryRebootModule_ModuleNotOnCmdMode(void **state);
void tryRebootModule_Success(void **state);

void trySetServices_ModuleNotOnCmdMode(void **state);
void trySetServices_UartTransparentSuccess(void **state);

void trySetDeviceName_ModuleNotOnCmdMode(void **state);
void trySetDeviceName_StringTooLong(void **state);
void trySetDeviceName_Success(void **state);

void trySetConfig_Success(void **state);

void tryGetFirmwareVersion_ModuleNotOnCmdMode(void **state);
void tryGetFirmwareVersion_Success(void **state);

void tryDumpInfos_ModuleNotOnCmdMode(void **state);
void tryDumpInfos_Success(void **state);

void tryGetServices_ModuleNotOnCmdMode(void **state);
void tryGetServices_Success(void **state);

void tryGetDeviceName_ModuleNotOnCmdMode(void **state);
void tryGetDeviceName_Success(void **state);

void tryGetMacAddress_ModuleNotOnCmdMode(void **state);
void tryGetMacAddress_Success(void **state);

void tryGetConfig_Success(void **state);

void checkTransparentUartMode_isTrue(void **state);
void checkTransparentUartMode_isFalse(void **state);

void trySendDataOnTransparentUart_ModuleNotOnDataMode(void **state);
void trySendDataOnTransparentUart_NoStreaming(void **state);
void trySendDataOnTransparentUart_Success(void **state);

void setForceFsmState_OutOfBounds(void **state);
void setForceFsmState_CorrectState(void **state);

void getFsmState_None(void **state);
void getFsmState_OnCmdMode(void **state);
void getFsmState_AfterReboot(void **state);
void getFsmState_AfterExternalBleConnection(void **state);
void getFsmState_AfterExternalBleStreaming(void **state);
void getFsmState_AfterExternalBleDisconnection(void **state);

void setForceDataMode_Success(void **state);

void getErrorCodeStr_OutOfBounds(void **state);
void getErrorCodeStr_CorrectCode(void **state);

#endif /* TEST_RN4871_H */