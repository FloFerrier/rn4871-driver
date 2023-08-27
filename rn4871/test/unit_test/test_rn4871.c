#include "test_rn4871.h"

#include <stdio.h>
#include <string.h>

#include "mock_rn4871.h"
#include "rn4871_defs.h"

#define NB_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

extern rn4871_error_e rn4871SendCmd(rn4871_module_s *dev, rn4871_cmd_e cmd,
                                    const char *format, ...);
extern rn4871_error_e rn4871ResponseProcess(rn4871_module_s *dev,
                                            const char *response);
extern rn4871_error_e rn4871ParseDumpInfos(const char *infos,
                                           RN4871_DUMP_INFOS_FIELD field,
                                           char *result, uint16_t resultMaxLen);
extern rn4871_error_e rn4871ParseFirmwareVersion(const char *firmwareVersion,
                                                 char *result,
                                                 uint16_t resultMaxLen);

void tryInitModule_MissInterface(void **state) {
  rn4871_module_s *module = malloc(sizeof(rn4871_module_s));
  assert_int_equal(rn4871Init(module), RN4871_ERROR_BAD_PARAMETER);
  free(module);
}

void tryInitModule_Success(void **state) {
  rn4871_module_s *module = malloc(sizeof(rn4871_module_s));
  module->delayMs = rn4871DelayMsCb;
  module->uartRx = rn4871UartRxCb;
  module->uartTx = rn4871UartTxCb;
  assert_int_equal(rn4871Init(module), RN4871_ERROR_NONE);
  assert_int_equal(module->_currentCmd, RN4871_CMD_NONE);
  assert_int_equal(module->_currentMode, RN4871_MODE_DATA);
  assert_int_equal(module->_fsmState, RN4871_FSM_STATE_NONE);
  free(module);
}

void receiveData_ModuleOnCmdMode(void **state) {
  rn4871_module_s *module = *state;
  char receivedData[BUFFER_SIZE_MAX] = "Fake data\r\n";
  uint16_t receivedDataLen = strlen(receivedData);
  mock_rn4871EnterCommandMode(module);
  assert_int_equal(module->_currentMode, RN4871_MODE_COMMAND);
  assert_int_equal(
      rn4871WaitReceivedData(module, receivedData, &receivedDataLen),
      RN4871_ERROR_NO_MODE_DATA);
}

void receiveData_RebootMsg(void **state) {
  rn4871_module_s *module = *state;
  char receivedData[BUFFER_SIZE_MAX] = "";
  uint16_t receivedDataLen = 0;
  mock_rn4871UartRxCb("Rebooting", RN4871_ERROR_NONE);
  assert_int_equal(
      rn4871WaitReceivedData(module, receivedData, &receivedDataLen),
      RN4871_ERROR_NONE);
  assert_string_equal(receivedData, "Rebooting");
  assert_int_equal(receivedDataLen, 9);
  assert_int_equal(module->_fsmState, RN4871_FSM_STATE_IDLE);
}

void receiveData_ConnectMsg(void **state) {
  rn4871_module_s *module = *state;
  char receivedData[BUFFER_SIZE_MAX] = "";
  uint16_t receivedDataLen = 0;
  mock_rn4871UartRxCb("CONNECT", RN4871_ERROR_NONE);
  assert_int_equal(
      rn4871WaitReceivedData(module, receivedData, &receivedDataLen),
      RN4871_ERROR_NONE);
  assert_string_equal(receivedData, "CONNECT");
  assert_int_equal(receivedDataLen, 7);
  assert_int_equal(module->_fsmState, RN4871_FSM_STATE_CONNECTED);
}

void receiveData_StreamingMsg(void **state) {
  rn4871_module_s *module = *state;
  char receivedData[BUFFER_SIZE_MAX] = "";
  uint16_t receivedDataLen = 0;
  mock_rn4871UartRxCb("STREAM_OPEN", RN4871_ERROR_NONE);
  assert_int_equal(
      rn4871WaitReceivedData(module, receivedData, &receivedDataLen),
      RN4871_ERROR_NONE);
  assert_string_equal(receivedData, "STREAM_OPEN");
  assert_int_equal(receivedDataLen, 11);
  assert_int_equal(module->_fsmState, RN4871_FSM_STATE_STREAMING);
}

void receiveData_DisconnectMsg(void **state) {
  rn4871_module_s *module = *state;
  char receivedData[BUFFER_SIZE_MAX] = "";
  uint16_t receivedDataLen = 0;
  mock_rn4871UartRxCb("DISCONNECT", RN4871_ERROR_NONE);
  assert_int_equal(
      rn4871WaitReceivedData(module, receivedData, &receivedDataLen),
      RN4871_ERROR_NONE);
  assert_string_equal(receivedData, "DISCONNECT");
  assert_int_equal(receivedDataLen, 10);
  assert_int_equal(module->_fsmState, RN4871_FSM_STATE_IDLE);
}

void receiveData_UserMsg(void **state) {
  rn4871_module_s *module = *state;
  char receivedData[BUFFER_SIZE_MAX] = "";
  uint16_t receivedDataLen = 0;
  mock_rn4871UartRxCb("User message", RN4871_ERROR_NONE);
  assert_int_equal(
      rn4871WaitReceivedData(module, receivedData, &receivedDataLen),
      RN4871_ERROR_NONE);
  assert_string_equal(receivedData, "User message");
  assert_int_equal(receivedDataLen, 12);
}

void test_sendCommand(void **state) {
  rn4871_module_s *module = *state;
  rn4871SendCmd(module, RN4871_CMD_GET_VERSION, NULL);
}

void tryEnterOnCmdMode_NoHardwareModule(void **state) {
  rn4871_module_s *module = *state;
  mock_rn4871UartTxCb("$", RN4871_ERROR_UART_FAIL);
  mock_rn4871UartTxCb("$", RN4871_ERROR_UART_FAIL);
  mock_rn4871UartTxCb("$", RN4871_ERROR_UART_FAIL);
  assert_int_equal(rn4871EnterCommandMode(module), RN4871_ERROR_UART_FAIL);
  assert_int_equal(module->_currentCmd, RN4871_CMD_MODE_ENTER);
  assert_int_equal(module->_currentMode, RN4871_MODE_DATA);
  assert_int_equal(module->_fsmState, RN4871_FSM_STATE_NONE);
}

void tryEnterOnCmdMode_IncorrectResponse(void **state) {
  rn4871_module_s *module = *state;
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("Fake response to simulate another moduleice\r\n",
                      RN4871_ERROR_NONE);
  assert_int_equal(rn4871EnterCommandMode(module), RN4871_ERROR_BAD_PARAMETER);
  assert_int_equal(module->_currentCmd, RN4871_CMD_MODE_ENTER);
  assert_int_equal(module->_currentMode, RN4871_MODE_DATA);
  assert_int_equal(module->_fsmState, RN4871_FSM_STATE_NONE);
}

void tryEnterOnCmdMode_Fail(void **state) {
  rn4871_module_s *module = *state;
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("", RN4871_ERROR_UART_FAIL);
  mock_rn4871UartTxCb("\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("Fake response", RN4871_ERROR_NONE);
  assert_int_equal(rn4871EnterCommandMode(module), RN4871_ERROR_BAD_PARAMETER);
  assert_int_equal(module->_currentCmd, RN4871_CMD_MODE_ENTER);
  assert_int_equal(module->_currentMode, RN4871_MODE_COMMAND);
  assert_int_equal(module->_fsmState, RN4871_FSM_STATE_INIT);
}

void tryEnterOnCmdMode_ModuleAlreadyOnCmdMode(void **state) {
  rn4871_module_s *module = *state;
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("", RN4871_ERROR_UART_FAIL);
  mock_rn4871UartTxCb("\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("Err\r\nCMD>", RN4871_ERROR_NONE);
  assert_int_equal(rn4871EnterCommandMode(module), RN4871_ERROR_NONE);
  assert_int_equal(module->_currentCmd, RN4871_CMD_MODE_ENTER);
  assert_int_equal(module->_currentMode, RN4871_MODE_COMMAND);
  assert_int_equal(module->_fsmState, RN4871_FSM_STATE_INIT);
}

void tryEnterOnCmdMode_Success(void **state) {
  rn4871_module_s *module = *state;
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("AOK\r\nCMD>", RN4871_ERROR_NONE);
  assert_int_equal(rn4871EnterCommandMode(module), RN4871_ERROR_NONE);
  assert_int_equal(module->_currentCmd, RN4871_CMD_MODE_ENTER);
  assert_int_equal(module->_currentMode, RN4871_MODE_COMMAND);
  assert_int_equal(module->_fsmState, RN4871_FSM_STATE_INIT);
}

void tryQuitCmdMode_ModuleAlreadyOnDataMode(void **state) {
  rn4871_module_s *module = *state;
  assert_int_equal(rn4871QuitCommandMode(module), RN4871_ERROR_NO_MODE_COMMAND);
  assert_int_equal(module->_currentMode, RN4871_MODE_DATA);
}

void tryQuitCmdMode_Success(void **state) {
  rn4871_module_s *module = *state;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("---\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("AOK\r\nCMD>", RN4871_ERROR_NONE);
  assert_int_equal(rn4871QuitCommandMode(module), RN4871_ERROR_NONE);
  assert_int_equal(module->_currentCmd, RN4871_CMD_MODE_QUIT);
  assert_int_equal(module->_currentMode, RN4871_MODE_DATA);
  assert_int_equal(module->_fsmState, RN4871_FSM_STATE_INIT);
}

void tryRebootModule_ModuleNotOnCmdMode(void **state) {
  rn4871_module_s *module = *state;
  assert_int_equal(rn4871RebootModule(module), RN4871_ERROR_NO_MODE_COMMAND);
}

void tryRebootModule_Success(void **state) {
  rn4871_module_s *module = *state;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("R,1\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("AOK\r\nCMD>", RN4871_ERROR_NONE);
  assert_int_equal(rn4871RebootModule(module), RN4871_ERROR_NONE);
  assert_int_equal(module->_currentMode, RN4871_MODE_DATA);
  assert_int_equal(module->_fsmState, RN4871_FSM_STATE_IDLE);
}

void trySetServices_ModuleNotOnCmdMode(void **state) {
  rn4871_module_s *module = *state;
  assert_int_equal(rn4871SetServices(module, RN4871_UART_TRANSPARENT |
                                                 RN4871_DEVICE_INFORMATION),
                   RN4871_ERROR_NO_MODE_COMMAND);
}

void trySetServices_UartTransparentSuccess(void **state) {
  rn4871_module_s *module = *state;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("SS,C0\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("AOK\r\nCMD>", RN4871_ERROR_NONE);
  assert_int_equal(rn4871SetServices(module, RN4871_UART_TRANSPARENT |
                                                 RN4871_DEVICE_INFORMATION),
                   RN4871_ERROR_NONE);
}

void trySetDeviceName_ModuleNotOnCmdMode(void **state) {
  rn4871_module_s *module = *state;
  char deviceName[] = "test_rn4871";
  assert_int_equal(rn4871SetDeviceName(module, deviceName),
                   RN4871_ERROR_NO_MODE_COMMAND);
}

void trySetDeviceName_StringTooLong(void **state) {
  rn4871_module_s *module = *state;
  char deviceName[] = "test_rn4871_with_too_long_name";
  mock_rn4871EnterCommandMode(module);
  assert_int_equal(rn4871SetDeviceName(module, deviceName),
                   RN4871_ERROR_BAD_PARAMETER);
}

void trySetDeviceName_Success(void **state) {
  rn4871_module_s *module = *state;
  char deviceName[] = "test_rn4871";
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("SN,test_rn4871\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("AOK\r\nCMD>", RN4871_ERROR_NONE);
  assert_int_equal(rn4871SetDeviceName(module, deviceName), RN4871_ERROR_NONE);
}

void trySetConfig_Success(void **state) {
  rn4871_module_s *module = *state;
  rn4871_config_s config = {.deviceName = "RN4871-Test", .services = 0xC0};
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("SN,RN4871-Test\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("AOK\r\nCMD>", RN4871_ERROR_NONE);
  mock_rn4871UartTxCb("SS,C0\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("AOK\r\nCMD>", RN4871_ERROR_NONE);
  assert_int_equal(rn4871SetConfig(module, &config), RN4871_ERROR_NONE);
}

void tryGetFirmwareVersion_ModuleNotOnCmdMode(void **state) {
  rn4871_module_s *module = *state;
  char firmwareVersion[BUFFER_SIZE_MAX] = "";
  assert_int_equal(
      rn4871GetFirmwareVersion(module, firmwareVersion, BUFFER_SIZE_MAX - 1),
      RN4871_ERROR_NO_MODE_COMMAND);
}

void tryGetFirmwareVersion_Success(void **state) {
  rn4871_module_s *module = *state;
  char firmwareVersion[BUFFER_SIZE_MAX] = "";
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("V\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb(
      "RN4871 V1.40 7/9/2019 (c)Microship Technology Inc\r\nCMD>",
      RN4871_ERROR_NONE);
  assert_int_equal(
      rn4871GetFirmwareVersion(module, firmwareVersion, BUFFER_SIZE_MAX - 1),
      RN4871_ERROR_NONE);
  assert_string_equal(firmwareVersion, "V1.40");
}

void tryDumpInfos_ModuleNotOnCmdMode(void **state) {
  rn4871_module_s *module = *state;
  char infos[BUFFER_SIZE_MAX] = "";
  assert_int_equal(rn4871DumpInfos(module, infos),
                   RN4871_ERROR_NO_MODE_COMMAND);
}

void tryDumpInfos_Success(void **state) {
  rn4871_module_s *module = *state;
  char infos[BUFFER_SIZE_MAX] = "";
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("D\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb(
      "BTA=none\r\nName=test_rn4871\r\nConnected=no\r\nAuthen=0\r\nFeatures="
      "0000\r\nServices=00\r\nCMD>",
      RN4871_ERROR_NONE);
  assert_int_equal(rn4871DumpInfos(module, infos), RN4871_ERROR_NONE);
  assert_string_equal(infos,
                      "BTA=none\r\nName=test_rn4871\r\nConnected=no\r\nAuthen="
                      "0\r\nFeatures=0000\r\nServices=00\r\nCMD>");
}

void tryGetServices_ModuleNotOnCmdMode(void **state) {
  rn4871_module_s *module = *state;
  uint16_t services = 0xFF;
  assert_int_equal(rn4871GetServices(module, &services),
                   RN4871_ERROR_NO_MODE_COMMAND);
}

void tryGetServices_Success(void **state) {
  rn4871_module_s *module = *state;
  uint16_t services = 0xFF;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("D\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb(
      "BTA=none\r\nName=none\r\nConnected=no\r\nAuthen=0\r\nFeatures="
      "0000\r\nServices=C0\r\nCMD>",
      RN4871_ERROR_NONE);
  assert_int_equal(rn4871GetServices(module, &services), RN4871_ERROR_NONE);
  assert_int_equal(services, 0xC0);
}

void tryGetDeviceName_ModuleNotOnCmdMode(void **state) {
  rn4871_module_s *module = *state;
  char deviceName[BUFFER_SIZE_MAX] = "";
  assert_int_equal(rn4871GetDeviceName(module, deviceName, BUFFER_SIZE_MAX - 1),
                   RN4871_ERROR_NO_MODE_COMMAND);
}

void tryGetDeviceName_Success(void **state) {
  rn4871_module_s *module = *state;
  char deviceName[BUFFER_SIZE_MAX] = "";
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("D\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb(
      "BTA=none\r\nName=test_rn4871\r\nConnected=no\r\nAuthen=0\r\nFeatures="
      "0000\r\nServices=00\r\nCMD>",
      RN4871_ERROR_NONE);
  assert_int_equal(rn4871GetDeviceName(module, deviceName, BUFFER_SIZE_MAX - 1),
                   RN4871_ERROR_NONE);
  assert_string_equal(deviceName, "test_rn4871");
}

void tryGetMacAddress_ModuleNotOnCmdMode(void **state) {
  rn4871_module_s *module = *state;
  char macAddress[BUFFER_SIZE_MAX] = "";
  assert_int_equal(rn4871GetMacAddress(module, macAddress, BUFFER_SIZE_MAX - 1),
                   RN4871_ERROR_NO_MODE_COMMAND);
}

void tryGetMacAddress_Success(void **state) {
  rn4871_module_s *module = *state;
  char macAddress[BUFFER_SIZE_MAX] = "";
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("D\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb(
      "BTA=001122334455\r\nName=test_rn4871\r\nConnected=no\r\nAuthen="
      "0\r\nFeatures=0000\r\nServices=00\r\nCMD>",
      RN4871_ERROR_NONE);
  assert_int_equal(rn4871GetMacAddress(module, macAddress, BUFFER_SIZE_MAX - 1),
                   RN4871_ERROR_NONE);
  assert_string_equal(macAddress, "001122334455");
}

void tryGetConfig_Success(void **state) {
  rn4871_module_s *module = *state;
  rn4871_config_s config;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("D\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb(
      "BTA=001122334455\r\nName=test_rn4871\r\nConnected=no\r\nAuthen="
      "0\r\nFeatures=0000\r\nServices=C0\r\nCMD>",
      RN4871_ERROR_NONE);
  mock_rn4871UartTxCb("V\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb(
      "RN4871 V1.40 7/9/2019 (c)Microship Technology Inc\r\nCMD>",
      RN4871_ERROR_NONE);
  assert_int_equal(rn4871GetConfig(module, &config), RN4871_ERROR_NONE);
  assert_string_equal(config.deviceName, "test_rn4871");
  assert_int_equal(config.services, 0xC0);
  assert_string_equal(config.firmwareVersion, "V1.40");
  assert_string_equal(config.macAddress, "001122334455");
}

void checkTransparentUartMode_isTrue(void **state) {
  rn4871_module_s *module = *state;
  bool result = false;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("D\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb(
      "BTA=001122334455\r\nName=test_rn4871\r\nConnected=no\r\nAuthen="
      "0\r\nFeatures=0000\r\nServices=C0\r\nCMD>",
      RN4871_ERROR_NONE);
  assert_int_equal(rn4871IsOnTransparentUart(module, &result),
                   RN4871_ERROR_NONE);
  assert_true(result);
}

void checkTransparentUartMode_isFalse(void **state) {
  rn4871_module_s *module = *state;
  bool result = false;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871UartTxCb("D\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb(
      "BTA=001122334455\r\nName=test_rn4871\r\nConnected=no\r\nAuthen="
      "0\r\nFeatures=0000\r\nServices=00\r\nCMD>",
      RN4871_ERROR_NONE);
  assert_int_equal(rn4871IsOnTransparentUart(module, &result),
                   RN4871_ERROR_NONE);
  assert_false(result);
}

void trySendDataOnTransparentUart_ModuleNotOnDataMode(void **state) {
  rn4871_module_s *module = *state;
  char dataToSend[] = "Test data send by transparent UART";
  uint16_t dataToSendLen = (uint16_t)strlen(dataToSend);
  mock_rn4871EnterCommandMode(module);
  assert_int_equal(
      rn4871TransparentUartSendData(module, dataToSend, dataToSendLen),
      RN4871_ERROR_NO_MODE_DATA);
}

void trySendDataOnTransparentUart_NoStreaming(void **state) {
  rn4871_module_s *module = *state;
  char dataToSend[] = "Test data send by transparent UART";
  uint16_t dataToSendLen = (uint16_t)strlen(dataToSend);
  assert_int_equal(
      rn4871TransparentUartSendData(module, dataToSend, dataToSendLen),
      RN4871_ERROR_NO_STREAMING);
}

void trySendDataOnTransparentUart_Success(void **state) {
  rn4871_module_s *module = *state;
  char dataToSend[] = "Test data send by transparent UART";
  uint16_t dataToSendLen = (uint16_t)strlen(dataToSend);
  module->_fsmState = RN4871_FSM_STATE_STREAMING;
  mock_rn4871UartTxCb("Test data send by transparent UART", RN4871_ERROR_NONE);
  assert_int_equal(
      rn4871TransparentUartSendData(module, dataToSend, dataToSendLen),
      RN4871_ERROR_NONE);
}

void setForceFsmState_OutOfBounds(void **state) {
  rn4871_module_s *module = *state;
  rn4871SetForceFsmState(module, 0xFF);
  assert_int_equal(module->_fsmState, RN4871_FSM_STATE_NONE);
}

void setForceFsmState_CorrectState(void **state) {
  rn4871_module_s *module = *state;
  rn4871_fsm_e tableFsmSate[] = {
      RN4871_FSM_STATE_NONE,      RN4871_FSM_STATE_INIT,
      RN4871_FSM_STATE_IDLE,      RN4871_FSM_STATE_CONNECTED,
      RN4871_FSM_STATE_STREAMING, RN4871_FSM_STATE_HALT,
  };

  for (int i = 0; i < NB_ELEMENTS(tableFsmSate); i++) {
    rn4871SetForceFsmState(module, tableFsmSate[i]);
    assert_int_equal(module->_fsmState, tableFsmSate[i]);
  }
}

void getFsmState_None(void **state) {
  rn4871_module_s *module = *state;
  assert_int_equal(rn4871GetFsmState(module), RN4871_FSM_STATE_NONE);
}

void getFsmState_OnCmdMode(void **state) {
  rn4871_module_s *module = *state;
  mock_rn4871EnterCommandMode(module);
  assert_int_equal(rn4871GetFsmState(module), RN4871_FSM_STATE_INIT);
}

void getFsmState_AfterReboot(void **state) {
  rn4871_module_s *module = *state;
  mock_rn4871EnterCommandMode(module);
  mock_rn4871RebootModule(module);
  mock_rn4871WaitReceivedData(module, "REBOOT");
  assert_int_equal(rn4871GetFsmState(module), RN4871_FSM_STATE_IDLE);
}

void getFsmState_AfterExternalBleConnection(void **state) {
  rn4871_module_s *module = *state;
  mock_rn4871WaitReceivedData(module, "CONNECT");
  assert_int_equal(rn4871GetFsmState(module), RN4871_FSM_STATE_CONNECTED);
}

void getFsmState_AfterExternalBleStreaming(void **state) {
  rn4871_module_s *module = *state;
  mock_rn4871WaitReceivedData(module, "CONNECT");
  mock_rn4871WaitReceivedData(module, "STREAM_OPEN");
  assert_int_equal(rn4871GetFsmState(module), RN4871_FSM_STATE_STREAMING);
}

void getFsmState_AfterExternalBleDisconnection(void **state) {
  rn4871_module_s *module = *state;
  mock_rn4871WaitReceivedData(module, "CONNECT");
  mock_rn4871WaitReceivedData(module, "STREAM_OPEN");
  mock_rn4871WaitReceivedData(module, "DISCONNECT");
  assert_int_equal(rn4871GetFsmState(module), RN4871_FSM_STATE_IDLE);
}

void setForceDataMode_Success(void **state) {
  rn4871_module_s *module = *state;
  rn4871SetForceDataMode(module);
  assert_int_equal(module->_currentMode, RN4871_MODE_DATA);
}

void getErrorCodeStr_OutOfBounds(void **state) {
  assert_string_equal(rn4871GetErrorCodeStr(0xFF), "UNKNOWN");
}

void getErrorCodeStr_CorrectCode(void **state) {
  char errorCodeStr[RN4871_ERROR_MAX][ERROR_CODE_LEN_MAX] = {
      "NONE",
      "BAD_PARAMETER",
      "UART_FAIL",
      "COMMAND_UNKNOWN",
      "NO_COMMAND_MODE",
      "NO_DATA_MODE",
      "NO_TRANSPARENT_UART",
      "NO_CONNECTED",
      "NO_STREAMING",
      "UNKNOWN",
  };

  for (int i = 0; i < RN4871_ERROR_MAX; i++) {
    assert_string_equal(rn4871GetErrorCodeStr((rn4871_error_e)i),
                        errorCodeStr[i]);
  }
}