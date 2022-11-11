#ifndef VIRTUAL_MODULE_H
#define VIRTUAL_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "rn4871_api.h"

#define BUFFER_MAX_LEN 255
#define MODULE_NAME_LEN 21
#define MAC_ADDRESS_LEN 25
#define FIRMWARE_VERSION_LEN 5
struct virtual_module_s
{
    char moduleName[MODULE_NAME_LEN+1];
    char macAddress[MAC_ADDRESS_LEN+1];
    char firmwareVersion[FIRMWARE_VERSION_LEN+1];
    uint16_t services;
    bool _command_mode;
    char _global_buffer[BUFFER_MAX_LEN+1];
    char _uartTx[BUFFER_MAX_LEN+1];
};

typedef struct virtual_module_s VIRTUAL_MODULE;

void virtualModuleInit(VIRTUAL_MODULE *virtualModule);
void virtualModuleReceiveData(VIRTUAL_MODULE *virtualModule, const char *dataReceived);
void virtualModuleSendData(VIRTUAL_MODULE*virtualModule, char *dataToSend, uint16_t *dataToSendLen);
RN4871_CODE_RETURN virtualModuleConnect(VIRTUAL_MODULE *virtualModule, RN4871_MODULE *dev);
RN4871_CODE_RETURN virtualModuleStream(VIRTUAL_MODULE *virtualModule, RN4871_MODULE *dev);
RN4871_CODE_RETURN virtualModuleDisconnect(VIRTUAL_MODULE *virtualModule, RN4871_MODULE *dev);
void virtualModuleSetForceDataMode(VIRTUAL_MODULE *virtualModule);

#ifdef __cplusplus
}
#endif

#endif /* VIRTUAL_MODULE_H */