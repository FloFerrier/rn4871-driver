#include "test_integration.h"

#include <stdio.h>
#include <string.h>

RN4871_CODE_RETURN rn4871UartTxCb(char *buf, uint16_t *len) {
  assert((NULL != buf) || (NULL != len));

  if (0 < len) {
    // printf("[UART_TX] %s\r\n", buf);
  }

  return CODE_RETURN_SUCCESS;
}

RN4871_CODE_RETURN rn4871UartRxCb(char *buf, uint16_t *len) {
  assert((NULL != buf) || (NULL != len));

  // printf("[UART_RX] %s\r\n", buf);

  return CODE_RETURN_SUCCESS;
}

void rn4871LogSender(char *log, int len) {
  assert(NULL != log);

  if (0 < len) {
    printf("<RN4871-LOGGER> %s", log);
  }
}

void rn4871DelayMsCb(uint32_t delay) {
  // nothing
}

void transparentUart(void **state) { print_message("To do\r\n"); }