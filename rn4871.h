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

struct rn4871_dev_s {
    rn4871_com_fptr_t uartTx;
    rn4871_com_fptr_t uartRx;
    rn4871_delay_fptr_t delayMs;
    enum rn4871_cmd_e _current_cmd;
    enum rn4871_fsm_e fsm_state;
};

uint8_t rn4871SendCmd(struct rn4871_dev_s *dev, enum rn4871_cmd_e cmd, const char *format, ...);
uint8_t rn4871ResponseProcess(struct rn4871_dev_s *dev, const char *input, char *output);

uint8_t rn4871EnterCommandMode(struct rn4871_dev_s *dev);
uint8_t rn4871RebootModule(struct rn4871_dev_s *dev);

uint8_t rn4871SetServices(struct rn4871_dev_s *dev, uint16_t service);
uint8_t rn4871EraseAllGattServices(struct rn4871_dev_s *dev);

uint8_t rn4871GetDeviceName(struct rn4871_dev_s *dev, char *deviceName);
uint8_t rn4871GetFirmwareVersion(struct rn4871_dev_s *dev, char *firmwareVersion);

uint8_t rn4871TransparentUartSendData(struct rn4871_dev_s *dev, uint8_t *pBuffer, uint16_t bufferSize);

#endif /* RN4871_H */
