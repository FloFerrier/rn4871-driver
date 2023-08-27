#include "mock_rn4871.h"

#include <stdbool.h>
#include <string.h>

#include "test_rn4871.h"

void mock_rn4871UartTxCb(char *buffer, rn4871_error_e codeReturn) {
  uint16_t len = strlen(buffer);
  expect_value(rn4871UartTxCb, size, len);
  expect_string(rn4871UartTxCb, buf, buffer);
  will_return(rn4871UartTxCb, codeReturn);
}

void mock_rn4871UartRxCb(char *buffer, rn4871_error_e codeReturn) {
  will_return(rn4871UartRxCb, (char *)buffer);
  will_return(rn4871UartRxCb, codeReturn);
}

void mock_rn4871EnterCommandMode(rn4871_module_s *dev) {
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartTxCb("$", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("AOK\r\nCMD>", RN4871_ERROR_NONE);
  assert_int_equal(rn4871EnterCommandMode(dev), RN4871_ERROR_NONE);
}

void mock_rn4871WaitReceivedData(rn4871_module_s *dev, char *mockReceivedData) {
  char receivedData[BUFFER_SIZE_MAX] = "";
  uint16_t receivedDataLen = 0;
  mock_rn4871UartRxCb(mockReceivedData, RN4871_ERROR_NONE);
  assert_int_equal(rn4871WaitReceivedData(dev, receivedData, &receivedDataLen),
                   RN4871_ERROR_NONE);
  assert_string_equal(receivedData, mockReceivedData);
  assert_int_equal(receivedDataLen, strlen(mockReceivedData));
}

void mock_rn4871QuitCommandMode(rn4871_module_s *dev) {
  mock_rn4871UartTxCb("---\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("AOK\r\nCMD>", RN4871_ERROR_NONE);
  assert_int_equal(rn4871QuitCommandMode(dev), RN4871_ERROR_NONE);
}

void mock_rn4871RebootModule(rn4871_module_s *dev) {
  mock_rn4871UartTxCb("R,1\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb("AOK\r\nCMD>", RN4871_ERROR_NONE);
  assert_int_equal(rn4871RebootModule(dev), RN4871_ERROR_NONE);
}

void mock_rn4871IsOnTransparentUart(rn4871_module_s *dev) {
  bool result = false;
  mock_rn4871UartTxCb("D\r\n", RN4871_ERROR_NONE);
  mock_rn4871UartRxCb(
      "BTA=001122334455\r\nName=test_rn4871\r\nConnected=no\r\nAuthen="
      "0\r\nFeatures=0000\r\nServices=C0\r\nCMD>",
      RN4871_ERROR_NONE);
  assert_int_equal(rn4871IsOnTransparentUart(dev, &result), RN4871_ERROR_NONE);
  assert_true(result);
}