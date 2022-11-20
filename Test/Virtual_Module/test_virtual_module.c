#include "test_virtual_module.h"

#include "mock_rn4871.h"

void tryInitModule_Success(void **state)
{
    VIRTUAL_MODULE module;
    virtualModuleInit(&module);
    assert_string_equal(module.firmwareVersion, "V1.40");
    assert_string_equal(module.moduleName, "RN4871-virtualModule");
    assert_string_equal(module.macAddress, "001122334455");
    assert_int_equal(module.services, DEVICE_INFORMATION);
}

void receiveData_CmdEnter(void **state)
{
    VIRTUAL_MODULE *module = *state;
    assert_false(module->_command_mode);
    virtualModuleReceiveData(module, "$");
    virtualModuleReceiveData(module, "$");
    virtualModuleReceiveData(module, "$");
    assert_string_equal(module->_global_buffer, "CMD> ");
    assert_true(module->_command_mode);
}

void receiveData_CmdQuit(void **state)
{
    VIRTUAL_MODULE *module = *state;
    module->_command_mode = true;
    virtualModuleReceiveData(module, "---\r\n");
    assert_string_equal(module->_global_buffer, "END\r\n");
    assert_false(module->_command_mode);
}

void receiveData_CmdIncorrect(void **state)
{
    VIRTUAL_MODULE *module = *state;
    module->_command_mode = true;
    virtualModuleReceiveData(module, "Fake\r\n");
    assert_string_equal(module->_global_buffer, "Err\r\nCMD> ");
}

void receiveData_CmdDumpInfos(void **state)
{
    VIRTUAL_MODULE *module = *state;
    module->_command_mode = true;
    virtualModuleReceiveData(module, "D\r\n");
    assert_string_equal(module->_global_buffer, "BTA=001122334455\r\nName=RN4871-virtualModule\r\nConnected=no\r\nAuthen=0\r\nFeatures=0000\r\nServices=80\r\nCMD> ");
}

void receiveData_CmdFirmwareVersion(void **state)
{
    VIRTUAL_MODULE *module = *state;
    module->_command_mode = true;
    virtualModuleReceiveData(module, "V\r\n");
    assert_string_equal(module->_global_buffer, "RN4871 V1.40 7/9/2019 (c)Microship Technology Inc\r\nCMD> ");
}

void receiveData_CmdReboot(void **state)
{
    VIRTUAL_MODULE *module = *state;
    module->_command_mode = true;
    virtualModuleReceiveData(module, "R,1\r\n");
    assert_string_equal(module->_global_buffer, "Rebooting\r\nCMD> ");
    assert_false(module->_command_mode);
}

void receiveData_CmdResetFactory(void **state)
{
    VIRTUAL_MODULE *module = *state;
    module->_command_mode = true;
    virtualModuleReceiveData(module, "SF,1\r\n");
    assert_string_equal(module->_global_buffer, "Rebooting\r\nCMD> ");
    assert_false(module->_command_mode);
}

void receiveData_CmdGetDeviceName(void **state)
{
    VIRTUAL_MODULE *module = *state;
    module->_command_mode = true;
    virtualModuleReceiveData(module, "GN\r\n");
    assert_string_equal(module->_global_buffer, "RN4871-virtualModule\r\nCMD> ");
}

void receiveData_CmdSetDeviceName(void **state)
{
    VIRTUAL_MODULE *module = *state;
    module->_command_mode = true;
    virtualModuleReceiveData(module, "SN,test\r\n");
    assert_string_equal(module->_global_buffer, "AOK\r\nCMD> ");
    assert_string_equal(module->moduleName, "test");
}

void receiveData_CmdSetServices(void **state)
{
    VIRTUAL_MODULE *module = *state;
    module->_command_mode = true;
    virtualModuleReceiveData(module, "SS,C0\r\n");
    assert_string_equal(module->_global_buffer, "AOK\r\nCMD> ");
    assert_int_equal(module->services, 0xC0);
}

void receiveData_AmountOfData(void **state)
{
    VIRTUAL_MODULE *module = *state;
    module->_command_mode = false;
    virtualModuleReceiveData(module, "Test to send some data !\r\n");
    assert_string_equal(module->_global_buffer, "Test to send some data !\r\n");
}

void sendData_EmptyString(void **state)
{
    VIRTUAL_MODULE *module = *state;;
    char dataToSend[256] = "";
    uint16_t dataToSendLen = 0;
    module->_command_mode = false;
    virtualModuleSendData(module, dataToSend, &dataToSendLen);
    assert_string_equal(dataToSend, "");
    assert_int_equal(dataToSendLen, 0);
}

void sendData_AmountOfData(void **state)
{
    VIRTUAL_MODULE *module = *state;
    char dataToSend[256] = "";
    uint16_t dataToSendLen = 0;
    module->_command_mode = false;
    strncpy(module->_global_buffer, "Test to send some data", 255);
    virtualModuleSendData(module, dataToSend, &dataToSendLen);
    assert_string_equal(dataToSend, "Test to send some data");
    assert_int_equal(dataToSendLen, 22);
}

void simulateBleExternal_Connection(void **state)
{
    VIRTUAL_MODULE *module = *state;
    RN4871_MODULE *device = malloc(sizeof(RN4871_MODULE));
    device->delayMs = rn4871DelayMsCb;
	device->uartRx = rn4871UartRxCb;
    device->uartTx = rn4871UartTxCb;
    assert_int_equal(rn4871Init(device), CODE_RETURN_SUCCESS);
    mock_rn4871UartTxCb("\%CONNECT,0,AABBCCDDEEFF\%", CODE_RETURN_SUCCESS);
    assert_int_equal(virtualModuleConnect(module, device), CODE_RETURN_SUCCESS);
    free(device);
}

void simulateBleExternal_Streaming(void **state)
{
    VIRTUAL_MODULE *module = *state;
    RN4871_MODULE *device = malloc(sizeof(RN4871_MODULE));
    device->delayMs = rn4871DelayMsCb;
	device->uartRx = rn4871UartRxCb;
    device->uartTx = rn4871UartTxCb;
    assert_int_equal(rn4871Init(device), CODE_RETURN_SUCCESS);
    mock_rn4871UartTxCb("\%STREAM_OPEN\%", CODE_RETURN_SUCCESS);
    assert_int_equal(virtualModuleStream(module, device), CODE_RETURN_SUCCESS);
    free(device);
}

void simulateBleExternal_Disconnection(void **state)
{
    VIRTUAL_MODULE *module = *state;
    RN4871_MODULE *device = malloc(sizeof(RN4871_MODULE));
    device->delayMs = rn4871DelayMsCb;
	device->uartRx = rn4871UartRxCb;
    device->uartTx = rn4871UartTxCb;
    assert_int_equal(rn4871Init(device), CODE_RETURN_SUCCESS);
    mock_rn4871UartTxCb("\%DISCONNECT\%", CODE_RETURN_SUCCESS);
    assert_int_equal(virtualModuleDisconnect(module, device), CODE_RETURN_SUCCESS);
    free(device);
}

void setForceDataMode_Success(void **state)
{
    VIRTUAL_MODULE *module = *state;
    module->_command_mode = true;
    virtualModuleSetForceDataMode(module);
    assert_false(module->_command_mode);
}