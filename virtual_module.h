#ifndef VIRTUAL_MODULE_H
#define VIRTUAL_MODULE_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "rn4871_api.h"

void virtualModuleReceiveData(char *dataReceived, uint16_t dataReceivedLen);
void virtualModuleSendData(char *dataToSend, uint16_t *dataToSendLen);
void virtualModuleConnect(struct rn4871_dev_s *dev);
void virtualModuleStream(struct rn4871_dev_s *dev);
void virtualModuleDisconnect(struct rn4871_dev_s *dev);
void virtualModuleSetForceDataMode(void);

#endif /* VIRTUAL_MODULE_H */