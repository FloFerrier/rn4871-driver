#include "test_rn4871.h"

#include <stdio.h>
#include <string.h>

#include "mock_rn4871.h"

#define NB_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

void tryInitModule_MissInterface(void **state) {
  RN4871_MODULE *module = malloc(sizeof(RN4871_MODULE));
  assert_int_equal(rn4871Init(module), CODE_RETURN_ERROR);
  free(module);
}

void tryInitModule_Success(void **state) {
  RN4871_MODULE *module = malloc(sizeof(RN4871_MODULE));
  module->delayMs = rn4871DelayMsCb;
  module->uartRx = rn4871UartRxCb;
  module->uartTx = rn4871UartTxCb;
  assert_int_equal(rn4871Init(module), CODE_RETURN_SUCCESS);
  assert_int_equal(module->_currentCmd, CMD_NONE);
  assert_int_equal(module->_currentMode, DATA_MODE);
  assert_int_equal(module->_fsmState, FSM_STATE_NONE);
  free(module);
}

void receiveData_ModuleOnCmdMode(void **state) {
  RN4871_MODULE *module = *state;
  char receivedData[BUFFER_SIZE_MAX] = "Fake data\r\n";
  uint16_t receivedDataLen = strlen(receivedData);
  mock_rn4871EnterCommandMode(module);
  assert_int_equal(module->_currentMode, COMMAND_MODE);
  assert_int_equal(
      rn4871WaitReceivedData(module, receivedData, &receivedDataLen),
      CODE_RETURN_NO_DATA_MODE);
}

void receiveData_RebootMsg(void **state) {
  RN4871_MODULE *module = *state;
  char receivedData[BUFFER_SIZE_MAX] = "";
  uint16_t receivedDataLen = 0;
  mock_rn4871UartRxCb("Rebooting", CODE_RETURN_SUCCESS);
  assert_int_equal(
      rn4871WaitReceivedData(module, receivedData, &receivedDataLen),
      CODE_RETURN_SUCCESS);
  assert_string_equal(receivedData, "Rebooting");
  assert_int_equal(receivedDataLen, 9);
  assert_int_equal(module->_fsmState, FSM_STATE_IDLE);
}

void receiveData_ConnectMsg(void **state) {
  RN4871_MODULE *module = *state;
  char receivedData[BUFFER_SIZE_MAX] = "";
  uint16_t receivedDataLen = 0;
  mock_rn4871UartRxCb("CONNECT", CODE_RETURN_SUCCESS);
  assert_int_equal(
      rn4871WaitReceivedData(module, receivedData, &receivedDataLen),
      CODE_RETURN_SUCCESS);
  assert_string_equal(receivedData, "CONNECT");
  assert_int_equal(receivedDataLen, 7);
  assert_int_equal(module->_fsmState, FSM_STATE_CONNECTED);
}

void receiveData_StreamingMsg(void **state) {
  RN4871_MODULE *module = *state;
  char receivedData[BUFFER_SIZE_MAX] = "";
  uint16_t receivedDataLen = 0;
  mock_rn4871UartRxCb("STREAM_OPEN", CODE_RETURN_SUCCESS);
  assert_int_equal(
      rn4871WaitReceivedData(module, receivedData, &receivedDataLen),
      CODE_RETURN_SUCCESS);
  assert_string_equal(receivedData, "STREAM_OPEN");
  assert_int_equal(receivedDataLen, 11);
  assert_int_equal(module->_fsmState, FSM_STATE_STREAMING);
}

void receiveData_DisconnectMsg(void **state) {
  RN4871_MODULE *module = *state;
  char receivedData[BUFFER_SIZE_MAX] = "";
  uint16_t receivedDataLen = 0;
  mock_rn4871UartRxCb("DISCONNECT", CODE_RETURN_SUCCESS);
  assert_int_equal(
      rn4871WaitReceivedData(module, receivedData, &receivedDataLen),
      CODE_RETURN_SUCCESS);
  assert_string_equal(receivedData, "DISCONNECT");
  assert_int_equal(receivedDataLen, 10);
  assert_int_equal(module->_fsmState, FSM_STATE_IDLE);
}

void receiveData_UserMsg(void **state) {
  RN4871_MODULE *module = *state;
  char receivedData[BUFFER_SIZE_MAX] = "";
  uint16_t receivedDataLen = 0;
  mock_rn4871UartRxCb("User message", CODE_RETURN_SUCCESS);
  assert_int_equal(
      rn4871WaitReceivedData(module, receivedData, &receivedDataLen),
      CODE_RETURN_SUCCESS);
  assert_string_equal(receivedData, "User message");
  assert_int_equal(receivedDataLen, 12);
}

void test_sendCommand(void **state) {
  RN4871_MODULE *module = *state;
  rn4871SendCmd(module, CMD_GET_VERSION, NULL);
}

void tryEnterOnCmdMode_NoHardwareModule(void **state) {
  RN4871_MODULE *module = *state;
  mock_rn4871UartTxCb("$", CODE_RETURN_UART_FAIL);
  mock_rn4871UartTxCb("$", CODE_RETURN_UART_FAIL);
  mock_rn4871UartTxCb("$", CODE_RETURN_UART_FAIL);
  assert_int_equal(rn4871EnterCommandMode(module), CODE_RETURN_UART_FAIL);
  assert_int_equal(module->_currentCmd, CMD_MODE_ENTER);
  assert_int_equal(module->_currentMode, DATA_MODE);
  assert_int_equal(module->_fsmState, FSM_STATE_NONE);
}

void tryEnterOnCmdMode_IncorrectResponse(void **state) {
  RN4871_MODULE *module = *state;
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("Fake response to simulate another moduleice\r\n",
                      CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871EnterCommandMode(module), CODE_RETURN_ERROR);
  assert_int_equal(module->_currentCmd, CMD_MODE_ENTER);
  assert_int_equal(module->_currentMode, DATA_MODE);
  assert_int_equal(module->_fsmState, FSM_STATE_NONE);
}

void tryEnterOnCmdMode_Fail(void **state) {
  RN4871_MODULE *module = *state;
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("", CODE_RETURN_UART_FAIL);
  mock_rn4871UartTxCb("\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("Fake response", CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871EnterCommandMode(module), CODE_RETURN_ERROR);
  assert_int_equal(module->_currentCmd, CMD_MODE_ENTER);
  assert_int_equal(module->_currentMode, COMMAND_MODE);
  assert_int_equal(module->_fsmState, FSM_STATE_INIT);
}

void tryEnterOnCmdMode_ModuleAlreadyOnCmdMode(void **state) {
  RN4871_MODULE *module = *state;
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("", CODE_RETURN_UART_FAIL);
  mock_rn4871UartTxCb("\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("Err\r\nCMD>", CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871EnterCommandMode(module), CODE_RETURN_SUCCESS);
  assert_int_equal(module->_currentCmd, CMD_MODE_ENTER);
  assert_int_equal(module->_currentMode, COMMAND_MODE);
  assert_int_equal(module->_fsmState, FSM_STATE_INIT);
}

void tryEnterOnCmdMode_Success(void **state) {
  RN4871_MODULE *module = *state;
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871EnterCommandMode(module), CODE_RETURN_SUCCESS);
  assert_int_equal(module->_currentCmd, CMD_MODE_ENTER);
  assert_int_equal(module->_currentMode, COMMAND_MODE);
  assert_int_equal(module->_fsmState, FSM_STATE_INIT);
}

void tryQuitCmdMode_ModuleAlreadyOnDataMode(void **state) {
  RN4871_MODULE *module = *state;
  assert_int_equal(rn4871QuitCommandMode(module), CODE_RETURN_NO_COMMAND_MODE);
  assert_int_equal(module->_currentMode, DATA_MODE);
}

void tryQuitCmdMode_Success(void **state) {
  RN4871_MODULE *module = *state;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("---\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871QuitCommandMode(module), CODE_RETURN_SUCCESS);
  assert_int_equal(module->_currentCmd, CMD_MODE_QUIT);
  assert_int_equal(module->_currentMode, DATA_MODE);
  assert_int_equal(module->_fsmState, FSM_STATE_INIT);
}

void tryRebootModule_ModuleNotOnCmdMode(void **state) {
  RN4871_MODULE *module = *state;
  assert_int_equal(rn4871RebootModule(module), CODE_RETURN_NO_COMMAND_MODE);
}

void tryRebootModule_Success(void **state) {
  RN4871_MODULE *module = *state;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("R,1\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871RebootModule(module), CODE_RETURN_SUCCESS);
  assert_int_equal(module->_currentMode, DATA_MODE);
  assert_int_equal(module->_fsmState, FSM_STATE_IDLE);
}

void trySetServices_ModuleNotOnCmdMode(void **state) {
  RN4871_MODULE *module = *state;
  assert_int_equal(
      rn4871SetServices(module, UART_TRANSPARENT | DEVICE_INFORMATION),
      CODE_RETURN_NO_COMMAND_MODE);
}

void trySetServices_UartTransparentSuccess(void **state) {
  RN4871_MODULE *module = *state;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("SS,C0\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
  assert_int_equal(
      rn4871SetServices(module, UART_TRANSPARENT | DEVICE_INFORMATION),
      CODE_RETURN_SUCCESS);
}

void trySetDeviceName_ModuleNotOnCmdMode(void **state) {
  RN4871_MODULE *module = *state;
  char deviceName[] = "test_rn4871";
  assert_int_equal(rn4871SetDeviceName(module, deviceName),
                   CODE_RETURN_NO_COMMAND_MODE);
}

void trySetDeviceName_StringTooLong(void **state) {
  RN4871_MODULE *module = *state;
  char deviceName[] = "test_rn4871_with_too_long_name";
  mock_rn4871EnterCommandMode(module);
  assert_int_equal(rn4871SetDeviceName(module, deviceName), CODE_RETURN_ERROR);
}

void trySetDeviceName_Success(void **state) {
  RN4871_MODULE *module = *state;
  char deviceName[] = "test_rn4871";
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("SN,test_rn4871\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871SetDeviceName(module, deviceName),
                   CODE_RETURN_SUCCESS);
}

void trySetConfig_Success(void **state) {
  RN4871_MODULE *module = *state;
  RN4871_CONFIG config = {.deviceName = "RN4871-Test", .services = 0xC0};
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("SN,RN4871-Test\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
  mock_rn4871UartTxCb("SS,C0\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871SetConfig(module, &config), CODE_RETURN_SUCCESS);
}

void tryGetFirmwareVersion_ModuleNotOnCmdMode(void **state) {
  RN4871_MODULE *module = *state;
  char firmwareVersion[BUFFER_SIZE_MAX] = "";
  assert_int_equal(
      rn4871GetFirmwareVersion(module, firmwareVersion, BUFFER_SIZE_MAX - 1),
      CODE_RETURN_NO_COMMAND_MODE);
}

void tryGetFirmwareVersion_Success(void **state) {
  RN4871_MODULE *module = *state;
  char firmwareVersion[BUFFER_SIZE_MAX] = "";
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("V\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb(
      "RN4871 V1.40 7/9/2019 (c)Microship Technology Inc\r\nCMD>",
      CODE_RETURN_SUCCESS);
  assert_int_equal(
      rn4871GetFirmwareVersion(module, firmwareVersion, BUFFER_SIZE_MAX - 1),
      CODE_RETURN_SUCCESS);
  assert_string_equal(firmwareVersion, "V1.40");
}

void tryDumpInfos_ModuleNotOnCmdMode(void **state) {
  RN4871_MODULE *module = *state;
  char infos[BUFFER_SIZE_MAX] = "";
  assert_int_equal(rn4871DumpInfos(module, infos), CODE_RETURN_NO_COMMAND_MODE);
}

void tryDumpInfos_Success(void **state) {
  RN4871_MODULE *module = *state;
  char infos[BUFFER_SIZE_MAX] = "";
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb(
      "BTA=none\r\nName=test_rn4871\r\nConnected=no\r\nAuthen=0\r\nFeatures="
      "0000\r\nServices=00\r\nCMD>",
      CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871DumpInfos(module, infos), CODE_RETURN_SUCCESS);
  assert_string_equal(infos,
                      "BTA=none\r\nName=test_rn4871\r\nConnected=no\r\nAuthen="
                      "0\r\nFeatures=0000\r\nServices=00\r\nCMD>");
}

void tryGetServices_ModuleNotOnCmdMode(void **state) {
  RN4871_MODULE *module = *state;
  uint16_t services = 0xFF;
  assert_int_equal(rn4871GetServices(module, &services),
                   CODE_RETURN_NO_COMMAND_MODE);
}

void tryGetServices_Success(void **state) {
  RN4871_MODULE *module = *state;
  uint16_t services = 0xFF;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb(
      "BTA=none\r\nName=none\r\nConnected=no\r\nAuthen=0\r\nFeatures="
      "0000\r\nServices=C0\r\nCMD>",
      CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871GetServices(module, &services), CODE_RETURN_SUCCESS);
  assert_int_equal(services, 0xC0);
}

void tryGetDeviceName_ModuleNotOnCmdMode(void **state) {
  RN4871_MODULE *module = *state;
  char deviceName[BUFFER_SIZE_MAX] = "";
  assert_int_equal(rn4871GetDeviceName(module, deviceName, BUFFER_SIZE_MAX - 1),
                   CODE_RETURN_NO_COMMAND_MODE);
}

void tryGetDeviceName_Success(void **state) {
  RN4871_MODULE *module = *state;
  char deviceName[BUFFER_SIZE_MAX] = "";
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb(
      "BTA=none\r\nName=test_rn4871\r\nConnected=no\r\nAuthen=0\r\nFeatures="
      "0000\r\nServices=00\r\nCMD>",
      CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871GetDeviceName(module, deviceName, BUFFER_SIZE_MAX - 1),
                   CODE_RETURN_SUCCESS);
  assert_string_equal(deviceName, "test_rn4871");
}

void tryGetMacAddress_ModuleNotOnCmdMode(void **state) {
  RN4871_MODULE *module = *state;
  char macAddress[BUFFER_SIZE_MAX] = "";
  assert_int_equal(rn4871GetMacAddress(module, macAddress, BUFFER_SIZE_MAX - 1),
                   CODE_RETURN_NO_COMMAND_MODE);
}

void tryGetMacAddress_Success(void **state) {
  RN4871_MODULE *module = *state;
  char macAddress[BUFFER_SIZE_MAX] = "";
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb(
      "BTA=001122334455\r\nName=test_rn4871\r\nConnected=no\r\nAuthen="
      "0\r\nFeatures=0000\r\nServices=00\r\nCMD>",
      CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871GetMacAddress(module, macAddress, BUFFER_SIZE_MAX - 1),
                   CODE_RETURN_SUCCESS);
  assert_string_equal(macAddress, "001122334455");
}

void tryGetConfig_Success(void **state) {
  RN4871_MODULE *module = *state;
  RN4871_CONFIG config;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb(
      "BTA=001122334455\r\nName=test_rn4871\r\nConnected=no\r\nAuthen="
      "0\r\nFeatures=0000\r\nServices=C0\r\nCMD>",
      CODE_RETURN_SUCCESS);
  mock_rn4871UartTxCb("V\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb(
      "RN4871 V1.40 7/9/2019 (c)Microship Technology Inc\r\nCMD>",
      CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871GetConfig(module, &config), CODE_RETURN_SUCCESS);
  assert_string_equal(config.deviceName, "test_rn4871");
  assert_int_equal(config.services, 0xC0);
  assert_string_equal(config.firmwareVersion, "V1.40");
  assert_string_equal(config.macAddress, "001122334455");
}

void checkTransparentUartMode_isTrue(void **state) {
  RN4871_MODULE *module = *state;
  bool result = false;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb(
      "BTA=001122334455\r\nName=test_rn4871\r\nConnected=no\r\nAuthen="
      "0\r\nFeatures=0000\r\nServices=C0\r\nCMD>",
      CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871IsOnTransparentUart(module, &result),
                   CODE_RETURN_SUCCESS);
  assert_true(result);
}

void checkTransparentUartMode_isFalse(void **state) {
  RN4871_MODULE *module = *state;
  bool result = false;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb(
      "BTA=001122334455\r\nName=test_rn4871\r\nConnected=no\r\nAuthen="
      "0\r\nFeatures=0000\r\nServices=00\r\nCMD>",
      CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871IsOnTransparentUart(module, &result),
                   CODE_RETURN_SUCCESS);
  assert_false(result);
}

void trySendDataOnTransparentUart_ModuleNotOnDataMode(void **state) {
  RN4871_MODULE *module = *state;
  char dataToSend[] = "Test data send by transparent UART";
  uint16_t dataToSendLen = (uint16_t)strlen(dataToSend);
  mock_rn4871EnterCommandMode(module);
  assert_int_equal(
      rn4871TransparentUartSendData(module, dataToSend, dataToSendLen),
      CODE_RETURN_NO_DATA_MODE);
}

void trySendDataOnTransparentUart_NoStreaming(void **state) {
  RN4871_MODULE *module = *state;
  char dataToSend[] = "Test data send by transparent UART";
  uint16_t dataToSendLen = (uint16_t)strlen(dataToSend);
  assert_int_equal(
      rn4871TransparentUartSendData(module, dataToSend, dataToSendLen),
      CODE_RETURN_NO_STREAMING);
}

void trySendDataOnTransparentUart_Success(void **state) {
  RN4871_MODULE *module = *state;
  char dataToSend[] = "Test data send by transparent UART";
  uint16_t dataToSendLen = (uint16_t)strlen(dataToSend);
  module->_fsmState = FSM_STATE_STREAMING;
  mock_rn4871UartTxCb("Test data send by transparent UART",
                      CODE_RETURN_SUCCESS);
  assert_int_equal(
      rn4871TransparentUartSendData(module, dataToSend, dataToSendLen),
      CODE_RETURN_SUCCESS);
}

void setForceFsmState_OutOfBounds(void **state) {
  RN4871_MODULE *module = *state;
  rn4871SetForceFsmState(module, 0xFF);
  assert_int_equal(module->_fsmState, FSM_STATE_NONE);
}

void setForceFsmState_CorrectState(void **state) {
  RN4871_MODULE *module = *state;
  RN4871_FSM tableFsmSate[] = {
      FSM_STATE_NONE,      FSM_STATE_INIT,      FSM_STATE_IDLE,
      FSM_STATE_CONNECTED, FSM_STATE_STREAMING, FSM_STATE_HALT,
  };

  for (int i = 0; i < NB_ELEMENTS(tableFsmSate); i++) {
    rn4871SetForceFsmState(module, tableFsmSate[i]);
    assert_int_equal(module->_fsmState, tableFsmSate[i]);
  }
}

void getFsmState_None(void **state) {
  RN4871_MODULE *module = *state;
  assert_int_equal(rn4871GetFsmState(module), FSM_STATE_NONE);
}

void getFsmState_OnCmdMode(void **state) {
  RN4871_MODULE *module = *state;
  mock_rn4871EnterCommandMode(module);
  assert_int_equal(rn4871GetFsmState(module), FSM_STATE_INIT);
}

void getFsmState_AfterReboot(void **state) {
  RN4871_MODULE *module = *state;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871RebootModule(module);
  mock_rn4871WaitReceivedData(module, "REBOOT");
  assert_int_equal(rn4871GetFsmState(module), FSM_STATE_IDLE);
}

void getFsmState_AfterExternalBleConnection(void **state) {
  RN4871_MODULE *module = *state;
  mock_rn4871WaitReceivedData(module, "CONNECT");
  assert_int_equal(rn4871GetFsmState(module), FSM_STATE_CONNECTED);
}

void getFsmState_AfterExternalBleStreaming(void **state) {
  RN4871_MODULE *module = *state;
  mock_rn4871WaitReceivedData(module, "CONNECT");
  mock_rn4871WaitReceivedData(module, "STREAM_OPEN");
  assert_int_equal(rn4871GetFsmState(module), FSM_STATE_STREAMING);
}

void getFsmState_AfterExternalBleDisconnection(void **state) {
  RN4871_MODULE *module = *state;
  mock_rn4871WaitReceivedData(module, "CONNECT");
  mock_rn4871WaitReceivedData(module, "STREAM_OPEN");
  mock_rn4871WaitReceivedData(module, "DISCONNECT");
  assert_int_equal(rn4871GetFsmState(module), FSM_STATE_IDLE);
}

void setForceDataMode_Success(void **state) {
  RN4871_MODULE *module = *state;
  rn4871SetForceDataMode(module);
  assert_int_equal(module->_currentMode, DATA_MODE);
}

void getErrorCodeStr_OutOfBounds(void **state) {
  assert_string_equal(rn4871GetErrorCodeStr(0xFF), "");
}

void getErrorCodeStr_CorrectCode(void **state) {
  char errorCodeStr[ERROR_CODE_NB_MAX][ERROR_CODE_LEN_MAX] = {
      "SUCCESS",
      "ERROR",
      "UART_FAIL",
      "COMMAND_UNKNOWN",
      "NO_COMMAND_MODE",
      "NO_DATA_MODE",
      "NO_TRANSPARENT_UART",
      "NO_CONNECTED",
      "NO_STREAMING",
      "",
  };

  for (int i = 0; i < ERROR_CODE_NB_MAX; i++) {
    assert_string_equal(rn4871GetErrorCodeStr((RN4871_CODE_RETURN)i),
                        errorCodeStr[i]);
  }
}