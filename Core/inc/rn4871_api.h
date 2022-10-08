#ifndef RN4871_API_H
#define RN4871_API_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

#include "rn4871_defs.h"

struct rn4871_conf_s
{
    char deviceName[DEVICE_NAME_LEN_MAX+1];
    char macAddress[MAC_ADDRESS_LEN_MAX+1];
    char firmwareVersion[FW_VERSION_LEN_MAX+1];
    uint16_t services;
};
struct rn4871_dev_s
{
    rn4871_com_fptr_t   uartTx;
    rn4871_com_fptr_t   uartRx;
    rn4871_delay_fptr_t delayMs;
    enum rn4871_mode_e _currentMode;
    enum rn4871_cmd_e _currentCmd;
    enum rn4871_fsm_e _fsmState;
};

uint8_t rn4871Init(struct rn4871_dev_s *dev);
uint8_t rn4871WaitReceivedData(struct rn4871_dev_s *dev, char *receivedData, uint16_t *receivedDataLen);
uint8_t rn4871EnterCommandMode(struct rn4871_dev_s *dev);
uint8_t rn4871QuitCommandMode(struct rn4871_dev_s *dev);
uint8_t rn4871RebootModule(struct rn4871_dev_s *dev);
uint8_t rn4871TransparentUartSendData(struct rn4871_dev_s *dev, const char *dataToSend, uint16_t dataToSendLen);

/* Setter functions */
uint8_t rn4871SetServices(struct rn4871_dev_s *dev, uint16_t service);
uint8_t rn4871SetDeviceName(struct rn4871_dev_s *dev, const char *deviceName);
uint8_t rn4871EraseAllGattServices(struct rn4871_dev_s *dev);
uint8_t rn4871SetConfig(struct rn4871_dev_s *dev, struct rn4871_conf_s *config);
void rn4871SetForceFsmState(struct rn4871_dev_s *dev, enum rn4871_fsm_e fsmForceState);
void rn4871SetForceDataMode(struct rn4871_dev_s *dev);

/* Getter functions */
uint8_t rn4871GetServices(struct rn4871_dev_s *dev, uint16_t *services);
uint8_t rn4871GetDeviceName(struct rn4871_dev_s *dev, char *deviceName, uint16_t deviceNameMaxLen);
uint8_t rn4871GetFirmwareVersion(struct rn4871_dev_s *dev, char *firmwareVersion, uint16_t deviceNameMaxLen);
uint8_t rn4871DumpInfos(struct rn4871_dev_s *dev, char *infos);
uint8_t rn4871GetMacAddress(struct rn4871_dev_s *dev, char *macAddress, uint16_t deviceNameMaxLen);
uint8_t rn4871GetConfig(struct rn4871_dev_s *dev, struct rn4871_conf_s *config);
uint8_t rn4871IsOnTransparentUart(struct rn4871_dev_s *dev, bool *result);
enum rn4871_fsm_e rn4871GetFsmState(struct rn4871_dev_s *dev);
char* rn4871GetErrorCodeStr(enum rn4871_code_return_e errorCode);

#endif /* RN4871_API_H */