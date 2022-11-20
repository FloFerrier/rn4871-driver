#ifndef RN4871_API_H
#define RN4871_API_H

#ifdef __cplusplus
extern "C" {
#endif

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

typedef struct
{
    char deviceName[DEVICE_NAME_LEN_MAX+1];
    char macAddress[MAC_ADDRESS_LEN_MAX+1];
    char firmwareVersion[FW_VERSION_LEN_MAX+1];
    uint16_t services;
} RN4871_CONFIG;
typedef struct
{
    rn4871_com_fptr_t   uartTx;
    rn4871_com_fptr_t   uartRx;
    rn4871_delay_fptr_t delayMs;
    RN4871_MODE _currentMode;
    RN4871_CMD _currentCmd;
    RN4871_FSM _fsmState;
} RN4871_MODULE;

RN4871_CODE_RETURN rn4871Init(RN4871_MODULE*dev);
RN4871_CODE_RETURN rn4871WaitReceivedData(RN4871_MODULE *dev, char *receivedData, uint16_t *receivedDataLen);
RN4871_CODE_RETURN rn4871EnterCommandMode(RN4871_MODULE *dev);
RN4871_CODE_RETURN rn4871QuitCommandMode(RN4871_MODULE *dev);
RN4871_CODE_RETURN rn4871RebootModule(RN4871_MODULE *dev);
RN4871_CODE_RETURN rn4871TransparentUartSendData(RN4871_MODULE *dev, const char *dataToSend, uint16_t dataToSendLen);

/* Setter functions */
RN4871_CODE_RETURN rn4871SetServices(RN4871_MODULE *dev, uint16_t service);
RN4871_CODE_RETURN rn4871SetDeviceName(RN4871_MODULE *dev, const char *deviceName);
RN4871_CODE_RETURN rn4871SetConfig(RN4871_MODULE *dev, RN4871_CONFIG *config);
void rn4871SetForceFsmState(RN4871_MODULE *dev, RN4871_FSM fsmForceState);
void rn4871SetForceDataMode(RN4871_MODULE *dev);

/* Getter functions */
RN4871_CODE_RETURN rn4871GetServices(RN4871_MODULE*dev, uint16_t *services);
RN4871_CODE_RETURN rn4871GetDeviceName(RN4871_MODULE *dev, char *deviceName, uint16_t deviceNameMaxLen);
RN4871_CODE_RETURN rn4871GetFirmwareVersion(RN4871_MODULE *dev, char *firmwareVersion, uint16_t deviceNameMaxLen);
RN4871_CODE_RETURN rn4871DumpInfos(RN4871_MODULE*dev, char *infos);
RN4871_CODE_RETURN rn4871GetMacAddress(RN4871_MODULE*dev, char *macAddress, uint16_t deviceNameMaxLen);
RN4871_CODE_RETURN rn4871GetConfig(RN4871_MODULE *dev, RN4871_CONFIG *config);
RN4871_CODE_RETURN rn4871IsOnTransparentUart(RN4871_MODULE *dev, bool *result);
RN4871_FSM rn4871GetFsmState(RN4871_MODULE*dev);
char* rn4871GetErrorCodeStr(RN4871_CODE_RETURN errorCode);

#ifdef __cplusplus
}
#endif

#endif /* RN4871_API_H */