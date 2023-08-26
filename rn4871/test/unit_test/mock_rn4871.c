#include "mock_rn4871.h"

#include <stdbool.h>
#include <string.h>

#include "test_rn4871.h"

void mock_rn4871UartTxCb(char *buffer, RN4871_CODE_RETURN codeReturn) {
  uint16_t len = strlen(buffer);
  expect_value(rn4871UartTxCb, size, len);
  expect_string(rn4871UartTxCb, buf, buffer);
  will_return(rn4871UartTxCb, codeReturn);
}

void mock_rn4871UartRxCb(char *buffer, RN4871_CODE_RETURN codeReturn) {
  will_return(rn4871UartRxCb, buffer);
  will_return(rn4871UartRxCb, codeReturn);
}

void mock_rn4871EnterCommandMode(RN4871_MODULE *dev) {
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartTxCb("$", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871EnterCommandMode(dev), CODE_RETURN_SUCCESS);
}

void mock_rn4871WaitReceivedData(RN4871_MODULE *dev, char *mockReceivedData) {
  char receivedData[BUFFER_SIZE_MAX] = "";
  uint16_t receivedDataLen = 0;
  mock_rn4871UartRxCb(mockReceivedData, CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871WaitReceivedData(dev, receivedData, &receivedDataLen),
                   CODE_RETURN_SUCCESS);
  assert_string_equal(receivedData, mockReceivedData);
  assert_int_equal(receivedDataLen, strlen(mockReceivedData));
}

void mock_rn4871QuitCommandMode(RN4871_MODULE *dev) {
  mock_rn4871UartTxCb("---\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871QuitCommandMode(dev), CODE_RETURN_SUCCESS);
}

void mock_rn4871RebootModule(RN4871_MODULE *dev) {
  mock_rn4871UartTxCb("R,1\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb("AOK\r\nCMD>", CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871RebootModule(dev), CODE_RETURN_SUCCESS);
}

void mock_rn4871IsOnTransparentUart(RN4871_MODULE *dev) {
  bool result = false;
  mock_rn4871UartTxCb("D\r\n", CODE_RETURN_SUCCESS);
  mock_rn4871UartRxCb(
      "BTA=001122334455\r\nName=test_rn4871\r\nConnected=no\r\nAuthen="
      "0\r\nFeatures=0000\r\nServices=C0\r\nCMD>",
      CODE_RETURN_SUCCESS);
  assert_int_equal(rn4871IsOnTransparentUart(dev, &result),
                   CODE_RETURN_SUCCESS);
  assert_true(result);
}