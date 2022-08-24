#include "virtual_module.h"
#include "logs.h"

#define BUFFER_MAX_LEN (255)
#define MODULE_NAME_LEN (21)
#define MAC_ADDRESS_LEN (25)
#define FIRMWARE_VERSION_LEN (5)

struct virtual_module_s {
    char moduleName[MODULE_NAME_LEN+1];
    char macAddress[MAC_ADDRESS_LEN+1];
    char firmwareVersion[FIRMWARE_VERSION_LEN+1];
    uint16_t services;
};

struct virtual_module_s virtualModule = {
    .moduleName = "RN4871-VM",
    .macAddress = "001122334455",
    .firmwareVersion = "V1.40",
    .services = DEVICE_INFORMATION,
};

char pGlobalBuffer[BUFFER_MAX_LEN+1] = "";
char saveBuffer[BUFFER_MAX_LEN+1] = "";
char tmpBuffer[BUFFER_MAX_LEN+1] = "";

static bool _command_mode = false;

static enum rn4871_cmd_e getCommand(char *command) {
    assert(NULL != command);

    char delimiter[] = ",\r\n";
    char *saveptr;
    char *token = strtok_r(command, delimiter, &saveptr);
    for(int i=0; i< CMD_NONE; i++) {
        if(NULL != token) {
            if(0 == strcmp(token, TABLE_COMMAND[i])) {
                logger(LOG_DEBUG, "getCommand: find command %s\r\n", TABLE_COMMAND[i]);
                return i;
            }
        }
    }
    return CMD_NONE;
}

static char *parseArgCommand(char *command) {
    assert(NULL != command);

    char delimiter[] = ",\r\n";
    char *saveptr;
    char *token = strtok_r(command, delimiter, &saveptr);
    if(NULL != token) {
        token = strtok_r(NULL, delimiter, &saveptr);
        logger(LOG_DEBUG, "parseArgCommand: find arg %s\r\n", token);
        return token;
    }
    return NULL;
}

void virtualModuleReceiveData(char *dataReceived, uint16_t dataReceivedLen) {
    assert(NULL != dataReceived);

    logger(LOG_DEBUG, "virtualModuleReceiveData: [%d] \"%s\"\r\n", dataReceivedLen, dataReceived);

    memset(pGlobalBuffer, '\0', BUFFER_MAX_LEN);
    memset(tmpBuffer, '\0', BUFFER_MAX_LEN);
    static uint8_t cnt_cmd_mode = 0;
    if(_command_mode) {
        char *arg;
        strncpy(tmpBuffer, dataReceived, BUFFER_MAX_LEN);
        enum rn4871_cmd_e command = getCommand(tmpBuffer);
        if(CMD_NONE != command) {
            switch(command) {
                case CMD_MODE_QUIT: {
                    snprintf(pGlobalBuffer, BUFFER_MAX_LEN, "END\r\n");
                    _command_mode = false;
                    break;
                }
                case CMD_CLEAR_ALL_SERVICES: {
                    snprintf(pGlobalBuffer, BUFFER_MAX_LEN, "AOK\r\nCMD>");
                    break;
                }
                case CMD_RESET_FACTORY:
                case CMD_REBOOT: {
                    strncpy(pGlobalBuffer, "Rebooting\r\nCMD>", BUFFER_MAX_LEN);
                    _command_mode = false;
                    break;
                }
                case CMD_GET_DEVICE_NAME: {
                    snprintf(pGlobalBuffer, BUFFER_MAX_LEN, "%s\r\nCMD>", virtualModule.moduleName);
                }
                case CMD_DUMP_INFOS: {
                    snprintf(pGlobalBuffer, BUFFER_MAX_LEN,
                        "BTA=%s\r\nName=%s\r\nConnected=no\r\nAuthen=0\r\nFeatures=0000\r\nServices=%X\r\nCMD>",
                        virtualModule.macAddress, virtualModule.moduleName, virtualModule.services);
                    break;
                }
                case CMD_GET_VERSION: {
                    snprintf(pGlobalBuffer, BUFFER_MAX_LEN, "RN4871 %s 7/9/2019 (c)Microship Technology Inc\r\nCMD>",
                        virtualModule.firmwareVersion);
                    break;
                }
                case CMD_SET_SERVICES: {
                    arg = parseArgCommand(dataReceived);
                    virtualModule.services = (uint16_t)strtol(arg, NULL, 16);
                    snprintf(pGlobalBuffer, BUFFER_MAX_LEN, "AOK\r\nCMD>");
                    break;
                }
                case CMD_SET_DEVICE_NAME: {
                    arg = parseArgCommand(dataReceived);
                    if(BUFFER_MAX_LEN >= strlen(arg)) {
                        strcpy(virtualModule.moduleName, arg);
                    }
                    snprintf(pGlobalBuffer, BUFFER_MAX_LEN, "AOK\r\nCMD>");
                    break;
                }
                default:
                    snprintf(pGlobalBuffer, BUFFER_MAX_LEN, "Err\r\nCMD>");
                    break;
            }
        }
        else {
            snprintf(pGlobalBuffer, BUFFER_MAX_LEN, "Err\r\nCMD>");
        }
    }
    else if(0 == strcmp(dataReceived, "$")) {
        ++cnt_cmd_mode;
        if (3 == cnt_cmd_mode) {
            snprintf(pGlobalBuffer, BUFFER_MAX_LEN, "CMD>");
            cnt_cmd_mode = 0;
            _command_mode = true;
        }
    }
    /* Data mode */
    else {
        strncpy(pGlobalBuffer, dataReceived, BUFFER_MAX_LEN);
    }
}

void virtualModuleSendData(char *dataToSend, uint16_t *dataToSendLen) {
    assert((NULL != dataToSend) || (NULL != dataToSendLen));

    strncpy(dataToSend, pGlobalBuffer, BUFFER_MAX_LEN);
    *dataToSendLen = strnlen(pGlobalBuffer, BUFFER_MAX_LEN);
    logger(LOG_DEBUG, "virtualModuleSendData: [%d] \"%s\"\r\n", *dataToSendLen, dataToSend);
}

void virtualModuleConnect(struct rn4871_dev_s *dev) {
    assert(NULL != dev);

    strncpy(saveBuffer, "\%CONNECT,0,AABBCCDDEEFF\%", BUFFER_MAX_LEN);
    uint16_t bufferLen = strlen(saveBuffer);
    dev->uartTx(saveBuffer, &bufferLen);
}

void virtualModuleStream(struct rn4871_dev_s *dev) {
    assert(NULL != dev);

    strncpy(saveBuffer, "\%STREAM_OPEN\%", BUFFER_MAX_LEN);
    uint16_t bufferLen = strlen(saveBuffer);
    dev->uartTx(saveBuffer, &bufferLen);
}

void virtualModuleDisconnect(struct rn4871_dev_s *dev) {
    assert(NULL != dev);

    strncpy(saveBuffer, "\%DISCONNECT\%", BUFFER_MAX_LEN);
    uint16_t bufferLen = strlen(saveBuffer);
    dev->uartTx(saveBuffer, &bufferLen);
}

void virtualModuleSetForceDataMode(void) {
    _command_mode = false;
}