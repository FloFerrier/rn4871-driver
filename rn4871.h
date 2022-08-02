#ifndef RN4871_H
#define RN4871_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#include "rn4871_defs.h"
#include "gatt.h"

struct rn4871_dev_s {
    rn4871_com_fptr_t uartTx;
    rn4871_com_fptr_t uartRx;
    rn4871_delay_fptr_t delayMs;
    enum rn4871_cmd_e _current_cmd;
    enum rn4871_fsm_e fsm_state;
    bool transparentUart;
};

uint8_t rn4871SendCmd(struct rn4871_dev_s *dev, enum rn4871_cmd_e cmd, const char *format, ...);
uint8_t rn4871ResponseProcess(struct rn4871_dev_s *dev, const char *pBuffer, uint16_t bufferSize);

uint8_t rn4871SetConfig(struct rn4871_dev_s *dev);
uint8_t rn4871TransparentUartSendData(struct rn4871_dev_s *dev, uint8_t *pBuffer, uint16_t bufferSize);

uint8_t _createCustomService(struct rn4871_dev_s *dev, struct service_param_s *service);
uint8_t _createCustomChar(struct rn4871_dev_s *dev, struct char_param_s *characteristic);

#endif /* RN4871_H */
