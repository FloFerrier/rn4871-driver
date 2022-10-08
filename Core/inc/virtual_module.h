#ifndef VIRTUAL_MODULE_H
#define VIRTUAL_MODULE_H

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
};

void virtualModuleInit(struct virtual_module_s *virtualModule);
void virtualModuleReceiveData(struct virtual_module_s *virtualModule, const char *dataReceived);
void virtualModuleSendData(struct virtual_module_s *virtualModule, char *dataToSend, uint16_t *dataToSendLen);
void virtualModuleConnect(struct rn4871_dev_s *dev);
void virtualModuleStream(struct rn4871_dev_s *dev);
void virtualModuleDisconnect(struct rn4871_dev_s *dev);
void virtualModuleSetForceDataMode(void);

#endif /* VIRTUAL_MODULE_H */