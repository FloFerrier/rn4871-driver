#include "test_rn4871.h"
#include "mock_rn4871.h"

#include <string.h>
#include <stdio.h>

#define NB_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

void test_rn4871Init(void **state)
{
    struct rn4871_dev_s *dev = malloc(sizeof(struct rn4871_dev_s));

    assert_int_equal(rn4871Init(dev), CODE_RETURN_ERROR);

    dev->delayMs = rn4871DelayMsCb;
	dev->uartRx = rn4871UartRxCb;
    dev->uartTx = rn4871UartTxCb;
    dev->_currentCmd = CMD_NONE;
    dev->_currentMode = DATA_MODE;
    dev->_fsmState = FSM_STATE_NONE;
    assert_int_equal(rn4871Init(dev), CODE_RETURN_SUCCESS);

    free(dev);
}

void test_rn4871EnterCommandMode(void **state)
{
    struct rn4871_dev_s *dev = *state;

    mock_rn4871UartTxCb("$", CODE_RETURN_UART_FAIL);
    mock_rn4871UartTxCb("$", CODE_RETURN_UART_FAIL);
    mock_rn4871UartTxCb("$", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871EnterCommandMode(dev), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
    mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
    mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871EnterCommandMode(dev), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
    mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
    mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("Err\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871EnterCommandMode(dev), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
    mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
    mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871EnterCommandMode(dev), CODE_RETURN_SUCCESS);
}

void test_rn4871WaitReceivedData(void **state)
{
    struct rn4871_dev_s *dev = *state;
    char receivedData[BUFFER_SIZE_MAX] = "";
    uint16_t receivedDataLen = 0;

    struct result_s 
    {
        char data[BUFFER_SIZE_MAX];
        enum rn4871_code_return_e codeReturn;
    };

    struct result_s expectedResult[] =
    {
        {"Test received data", CODE_RETURN_SUCCESS },
        {"REBOOT", CODE_RETURN_SUCCESS },
        {"DISCONNECT", CODE_RETURN_SUCCESS },
        {"CONNECT", CODE_RETURN_SUCCESS },
        {"STREAM_OPEN", CODE_RETURN_SUCCESS },
        {"", CODE_RETURN_NO_DATA_MODE },
    };

    int index = 0;
    for(index; index< (NB_ELEMENTS(expectedResult) - 1); index++)
    {
        memset(receivedData, '\0', BUFFER_SIZE_MAX);
        mock_rn4871UartRxCb(expectedResult[index].data, CODE_RETURN_SUCCESS);
        assert_int_equal(rn4871WaitReceivedData(dev, receivedData, &receivedDataLen), expectedResult[index].codeReturn);
        assert_string_equal(receivedData, expectedResult[index].data);
        assert_int_equal(receivedDataLen, strlen(expectedResult[index].data));
    }

    mock_rn4871EnterCommandMode(dev);
    assert_int_equal(rn4871WaitReceivedData(dev, receivedData, &receivedDataLen), expectedResult[index].codeReturn);
}

void test_rn4871QuitCommandMode(void **state)
{
    struct rn4871_dev_s *dev = *state;

    assert_int_equal(rn4871QuitCommandMode(dev), CODE_RETURN_NO_COMMAND_MODE);

    mock_rn4871EnterCommandMode(dev);

    mock_rn4871UartTxCb("---\r\n", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871QuitCommandMode(dev), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("---\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871QuitCommandMode(dev), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("---\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("Err\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871QuitCommandMode(dev), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("---\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871QuitCommandMode(dev), CODE_RETURN_SUCCESS);
}

void test_rn4871RebootModule(void **state)
{
    struct rn4871_dev_s *dev = *state;

    assert_int_equal(rn4871RebootModule(dev), CODE_RETURN_NO_COMMAND_MODE);

    mock_rn4871EnterCommandMode(dev);

    mock_rn4871UartTxCb("R,1\r\n", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871RebootModule(dev), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("R,1\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871RebootModule(dev), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("R,1\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("Err\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871RebootModule(dev), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("R,1\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871RebootModule(dev), CODE_RETURN_SUCCESS);
}

void test_rn4871SetServices(void **state)
{
    struct rn4871_dev_s *dev = *state;

    assert_int_equal(rn4871SetServices(dev, UART_TRANSPARENT | DEVICE_INFORMATION), CODE_RETURN_NO_COMMAND_MODE);

    mock_rn4871EnterCommandMode(dev);

    mock_rn4871UartTxCb("SS,C0\r\n", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871SetServices(dev, UART_TRANSPARENT | DEVICE_INFORMATION), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("SS,C0\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871SetServices(dev, UART_TRANSPARENT | DEVICE_INFORMATION), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("SS,C0\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("Err\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871SetServices(dev, UART_TRANSPARENT | DEVICE_INFORMATION), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("SS,C0\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871SetServices(dev, UART_TRANSPARENT | DEVICE_INFORMATION), CODE_RETURN_SUCCESS);
}

void test_rn4871SetDeviceName(void **state)
{
    struct rn4871_dev_s *dev = *state;

    char deviceName[] = "test_rn4871";

    assert_int_equal(rn4871SetDeviceName(dev, deviceName), CODE_RETURN_NO_COMMAND_MODE);

    mock_rn4871EnterCommandMode(dev);

    mock_rn4871UartTxCb("SN,test_rn4871\r\n", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871SetDeviceName(dev, deviceName), CODE_RETURN_UART_FAIL);

    assert_int_equal(rn4871SetDeviceName(dev, ""), CODE_RETURN_ERROR);

    assert_int_equal(rn4871SetDeviceName(dev, "this device name is too long"), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("SN,test_rn4871\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871SetDeviceName(dev, deviceName), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("SN,test_rn4871\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("Err\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871SetDeviceName(dev, deviceName), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("SN,test_rn4871\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871SetDeviceName(dev, deviceName), CODE_RETURN_SUCCESS);
}

void test_rn4871EraseAllGattServices(void **state)
{
    struct rn4871_dev_s *dev = *state;
    assert_int_equal(rn4871EraseAllGattServices(dev), CODE_RETURN_NO_COMMAND_MODE);

    mock_rn4871EnterCommandMode(dev);

    mock_rn4871UartTxCb("PZ\r\n", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871EraseAllGattServices(dev), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("PZ\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871EraseAllGattServices(dev), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("PZ\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("Err\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871EraseAllGattServices(dev), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("PZ\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871EraseAllGattServices(dev), CODE_RETURN_SUCCESS);
}

void test_rn4871GetServices(void **state)
{
    struct rn4871_dev_s *dev = *state;
    uint16_t services = 0xFF;

    assert_int_equal(rn4871GetServices(dev, &services), CODE_RETURN_NO_COMMAND_MODE);

    mock_rn4871EnterCommandMode(dev);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871GetServices(dev, &services), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871GetServices(dev, &services), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("Err\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetServices(dev, &services), CODE_RETURN_ERROR);

    /* Response without hexadecimal service data */
    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetServices(dev, &services), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("BTA=none\r\nName=none\r\nConnected=no\r\nAuthen=0\r\nFeatures=0000\r\nServices=\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetServices(dev, &services), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("BTA=none\r\nName=none\r\nConnected=no\r\nAuthen=0\r\nFeatures=0000\r\nServices=C0\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetServices(dev, &services), CODE_RETURN_SUCCESS);
    assert_int_equal(services, 0xC0);
}

void test_rn4871GetDeviceName(void **state)
{
    struct rn4871_dev_s *dev = *state;
    char deviceName[BUFFER_SIZE_MAX] = "";

    assert_int_equal(rn4871GetDeviceName(dev, deviceName, BUFFER_SIZE_MAX-1), CODE_RETURN_NO_COMMAND_MODE);

    mock_rn4871EnterCommandMode(dev);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871GetDeviceName(dev, deviceName, BUFFER_SIZE_MAX-1), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871GetDeviceName(dev, deviceName, BUFFER_SIZE_MAX-1), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("Err\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetDeviceName(dev, deviceName, BUFFER_SIZE_MAX-1), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetDeviceName(dev, deviceName, BUFFER_SIZE_MAX-1), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("BTA=none\r\nName=\r\nConnected=no\r\nAuthen=0\r\nFeatures=0000\r\nServices=00\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetDeviceName(dev, deviceName, BUFFER_SIZE_MAX-1), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("BTA=none\r\nName=test_rn4871\r\nConnected=no\r\nAuthen=0\r\nFeatures=0000\r\nServices=00\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetDeviceName(dev, deviceName, BUFFER_SIZE_MAX-1), CODE_RETURN_SUCCESS);
    assert_string_equal(deviceName, "test_rn4871");
}

void test_rn4871GetFirmwareVersion(void **state)
{
    struct rn4871_dev_s *dev = *state;
    char firmwareVersion[BUFFER_SIZE_MAX] = "";

    assert_int_equal(rn4871GetFirmwareVersion(dev, firmwareVersion, BUFFER_SIZE_MAX-1), CODE_RETURN_NO_COMMAND_MODE);

    mock_rn4871EnterCommandMode(dev);

    mock_rn4871UartTxCb("V\r\n", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871GetFirmwareVersion(dev, firmwareVersion, BUFFER_SIZE_MAX-1), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("V\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871GetFirmwareVersion(dev, firmwareVersion, BUFFER_SIZE_MAX-1), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("V\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("Err\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetFirmwareVersion(dev, firmwareVersion, BUFFER_SIZE_MAX-1), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("V\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetFirmwareVersion(dev, firmwareVersion, BUFFER_SIZE_MAX-1), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("V\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetFirmwareVersion(dev, firmwareVersion, BUFFER_SIZE_MAX-1), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("V\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("RN4871 V1.40 7/9/2019 (c)Microship Technology Inc\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetFirmwareVersion(dev, firmwareVersion, BUFFER_SIZE_MAX-1), CODE_RETURN_SUCCESS);
    assert_string_equal(firmwareVersion, "V1.40");
}

void test_rn4871DumpInfos(void **state)
{
    struct rn4871_dev_s *dev = *state;
    char infos[BUFFER_SIZE_MAX] = "";

    assert_int_equal(rn4871DumpInfos(dev, infos), CODE_RETURN_NO_COMMAND_MODE);

    mock_rn4871EnterCommandMode(dev);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871DumpInfos(dev, infos), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871DumpInfos(dev, infos), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("Err\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871DumpInfos(dev, infos), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871DumpInfos(dev, infos), CODE_RETURN_SUCCESS);
    assert_string_equal(infos, "AOK\r\nCMD>");

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("BTA=none\r\nName=test_rn4871\r\nConnected=no\r\nAuthen=0\r\nFeatures=0000\r\nServices=00\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871DumpInfos(dev, infos), CODE_RETURN_SUCCESS);
    assert_string_equal(infos, "BTA=none\r\nName=test_rn4871\r\nConnected=no\r\nAuthen=0\r\nFeatures=0000\r\nServices=00\r\nCMD>");
}

void test_rn4871GetMacAddress(void **state)
{
    struct rn4871_dev_s *dev = *state;
    char macAddress[BUFFER_SIZE_MAX] = "";

    assert_int_equal(rn4871GetMacAddress(dev, macAddress, BUFFER_SIZE_MAX-1), CODE_RETURN_NO_COMMAND_MODE);

    mock_rn4871EnterCommandMode(dev);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871GetMacAddress(dev, macAddress, BUFFER_SIZE_MAX-1), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871GetMacAddress(dev, macAddress, BUFFER_SIZE_MAX-1), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("Err\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetMacAddress(dev, macAddress, BUFFER_SIZE_MAX-1), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetMacAddress(dev, macAddress, BUFFER_SIZE_MAX-1), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("BTA=\r\nName=test_rn4871\r\nConnected=no\r\nAuthen=0\r\nFeatures=0000\r\nServices=00\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetMacAddress(dev, macAddress, BUFFER_SIZE_MAX-1), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("BTA=001122334455\r\nName=test_rn4871\r\nConnected=no\r\nAuthen=0\r\nFeatures=0000\r\nServices=00\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871GetMacAddress(dev, macAddress, BUFFER_SIZE_MAX-1), CODE_RETURN_SUCCESS);
    assert_string_equal(macAddress, "001122334455");
}

void test_rn4871IsOnTransparentUart(void **state)
{
    struct rn4871_dev_s *dev = *state;
    bool result = false;

    assert_int_equal(rn4871IsOnTransparentUart(dev, &result), CODE_RETURN_NO_COMMAND_MODE);

    mock_rn4871EnterCommandMode(dev);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871IsOnTransparentUart(dev, &result), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("", CODE_RETURN_UART_FAIL);
    assert_int_equal(rn4871IsOnTransparentUart(dev, &result), CODE_RETURN_UART_FAIL);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("Err\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871IsOnTransparentUart(dev, &result), CODE_RETURN_ERROR);

    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871IsOnTransparentUart(dev, &result), CODE_RETURN_ERROR);

    /* Service is not on Transparent UART */
    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("BTA=001122334455\r\nName=test_rn4871\r\nConnected=no\r\nAuthen=0\r\nFeatures=0000\r\nServices=00\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871IsOnTransparentUart(dev, &result), CODE_RETURN_SUCCESS);
    assert_false(result);

    /* Service is on Transparent UART */
    mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
    mock_rn4871UartRxCb("BTA=001122334455\r\nName=test_rn4871\r\nConnected=no\r\nAuthen=0\r\nFeatures=0000\r\nServices=C0\r\nCMD>", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871IsOnTransparentUart(dev, &result), CODE_RETURN_SUCCESS);
    assert_true(result);
}

void test_rn4871TransparentUartSendData(void **state)
{
    struct rn4871_dev_s *dev = *state;
    char dataToSend[] = "Test data send by transparent UART";
    uint16_t dataToSendLen = (uint16_t) strlen(dataToSend);

    mock_rn4871EnterCommandMode(dev);
    assert_int_equal(rn4871TransparentUartSendData(dev, dataToSend, dataToSendLen), CODE_RETURN_NO_DATA_MODE);

    mock_rn4871IsOnTransparentUart(dev);
    assert_int_equal(rn4871TransparentUartSendData(dev, dataToSend, dataToSendLen), CODE_RETURN_NO_DATA_MODE);

    mock_rn4871QuitCommandMode(dev);
    assert_int_equal(rn4871TransparentUartSendData(dev, dataToSend, dataToSendLen), CODE_RETURN_NO_STREAMING);

    rn4871SetForceFsmState(dev, FSM_STATE_CONNECTED);
    assert_int_equal(rn4871TransparentUartSendData(dev, dataToSend, dataToSendLen), CODE_RETURN_NO_STREAMING);

    rn4871SetForceFsmState(dev, FSM_STATE_STREAMING);
    mock_rn4871UartTxCb("Test data send by transparent UART", CODE_RETURN_SUCCESS);
    assert_int_equal(rn4871TransparentUartSendData(dev, dataToSend, dataToSendLen), CODE_RETURN_SUCCESS);
}

void test_rn4871GetFsmState(void **state)
{
    struct rn4871_dev_s *dev = *state;

    assert_int_equal(rn4871GetFsmState(dev), FSM_STATE_NONE);

    mock_rn4871EnterCommandMode(dev);
    assert_int_equal(rn4871GetFsmState(dev), FSM_STATE_INIT);

    mock_rn4871RebootModule(dev);
    mock_rn4871WaitReceivedData(dev, "REBOOT");
    assert_int_equal(rn4871GetFsmState(dev), FSM_STATE_IDLE);

    mock_rn4871WaitReceivedData(dev, "CONNECT");
    assert_int_equal(rn4871GetFsmState(dev), FSM_STATE_CONNECTED);

    mock_rn4871WaitReceivedData(dev, "STREAM_OPEN");
    assert_int_equal(rn4871GetFsmState(dev), FSM_STATE_STREAMING);

    mock_rn4871WaitReceivedData(dev, "DISCONNECT");
    assert_int_equal(rn4871GetFsmState(dev), FSM_STATE_IDLE);
}

void test_rn4871SetForceFsmState(void **state)
{
    struct rn4871_dev_s *dev = *state;

    rn4871SetForceFsmState(dev, FSM_STATE_NONE);
    assert_int_equal(rn4871GetFsmState(dev), FSM_STATE_NONE);

    rn4871SetForceFsmState(dev, FSM_STATE_INIT);
    assert_int_equal(rn4871GetFsmState(dev), FSM_STATE_INIT);

     rn4871SetForceFsmState(dev, FSM_STATE_IDLE);
    assert_int_equal(rn4871GetFsmState(dev), FSM_STATE_IDLE);

    rn4871SetForceFsmState(dev, FSM_STATE_CONNECTED);
    assert_int_equal(rn4871GetFsmState(dev), FSM_STATE_CONNECTED);

    rn4871SetForceFsmState(dev, FSM_STATE_STREAMING);
    assert_int_equal(rn4871GetFsmState(dev), FSM_STATE_STREAMING);

    rn4871SetForceFsmState(dev, FSM_STATE_HALT);
    assert_int_equal(rn4871GetFsmState(dev), FSM_STATE_HALT);
}

void test_rn4871SetForceDataMode(void **state)
{
    struct rn4871_dev_s *dev = *state;

    rn4871SetForceDataMode(dev);
    assert_int_equal(dev->_currentMode, DATA_MODE);
}

void test_rn4871GetErrorCodeStr(void **state)
{
    for(int i= 0; i< 8; i++)
    {
        assert_string_equal(rn4871GetErrorCodeStr((enum rn4871_code_return_e) i), ERROR_CODE[i]);
    }
}