#include "test_integration.h"

#include <stdio.h>
#include <string.h>

rn4871_error_e rn4871UartTxCb(char *buf, uint16_t *len) {
  assert((NULL != buf) || (NULL != len));

  if (0 < len) {
    // printf("[UART_TX] %s\r\n", buf);
  }

  return RN4871_ERROR_NONE;
}

rn4871_error_e rn4871UartRxCb(char *buf, uint16_t *len) {
  assert((NULL != buf) || (NULL != len));

  // printf("[UART_RX] %s\r\n", buf);

  return RN4871_ERROR_NONE;
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