#include "virtual_module.h"
#include "utils.h"

#define BUFFER_MAX_LEN (255)

struct virtual_module_s {
    char moduleName[21];
    char macAddress[25];
    char firmwareVersion[5];
    uint16_t services;
};

struct virtual_module_s virtualModule = {
    .moduleName = "RN4871-VM",
    .macAddress = "001122334455",
    .firmwareVersion = "V1.40",
    .services = 0x80,
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
            if(0 == strcmp(token, TABLE_COMMAND[i]))
                return i;
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
        return token;
    }
}

void virtualModuleReceiveData(const uint8_t *pInput, const uint16_t inputSize) {
    assert(NULL != pInput);

    memset(pGlobalBuffer, '\0', BUFFER_MAX_LEN);
    memset(tmpBuffer, '\0', BUFFER_MAX_LEN);
    static uint8_t cnt_cmd_mode = 0;
    if(_command_mode) {
        char *arg;
        strncpy(tmpBuffer, (char*)pInput, BUFFER_MAX_LEN);
        enum rn4871_cmd_e command = getCommand(tmpBuffer);
        if(CMD_NONE != command) {
            switch(command) {
                case CMD_MODE_QUIT: {
                    snprintf(pGlobalBuffer, BUFFER_MAX_LEN, "AOK\r\nCMD>");
                    _command_mode = true;
                    break;
                }
                case CMD_CLEAR_ALL_SERVICES: {
                    snprintf(pGlobalBuffer, BUFFER_MAX_LEN, "AOK\r\nCMD>");
                    break;
                }
                case CMD_RESET_FACTORY:
                case CMD_REBOOT: {
                    strncpy(pGlobalBuffer, "Rebooting\r\n%REBOOT%", BUFFER_MAX_LEN);
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
                    arg = parseArgCommand((char*)pInput);
                    virtualModule.services = (uint16_t)strtol(arg, NULL, 16);
                    snprintf(pGlobalBuffer, BUFFER_MAX_LEN, "AOK\r\nCMD>");
                    break;
                }
                case CMD_SET_DEVICE_NAME: {
                    arg = parseArgCommand((char*)pInput);
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
    else if(0 == strcmp(pInput, "$")) {
        ++cnt_cmd_mode;
        if (3 == cnt_cmd_mode) {
            snprintf(pGlobalBuffer, BUFFER_MAX_LEN, "CMD>");
            cnt_cmd_mode = 0;
            _command_mode = true;
        }
    }
}

void virtualModuleSendData(uint8_t *pOutput, uint16_t *outputSize) {
    assert((NULL != pOutput) || (NULL != outputSize));

    strncpy(pOutput, pGlobalBuffer, BUFFER_MAX_LEN);
    *outputSize = strnlen(pGlobalBuffer, BUFFER_MAX_LEN);
    //printf("[VM Send] [%d] \"%s\"\r\n", *outputSize, pGlobalBuffer);
}

void virtualModuleConnect(struct rn4871_dev_s *dev) {
    assert(NULL != dev);

    strncpy(saveBuffer, "\%CONNECT,0,AABBCCDDEEFF\%", BUFFER_MAX_LEN);
    uint16_t bufferLen = strlen(saveBuffer);
    dev->uartTx(saveBuffer, &bufferLen);
    rn4871ResponseProcess(dev, saveBuffer);
}

void virtualModuleStream(struct rn4871_dev_s *dev) {
    assert(NULL != dev);

    strncpy(saveBuffer, "\%STREAM_OPEN\%", BUFFER_MAX_LEN);
    uint16_t bufferLen = strlen(saveBuffer);
    dev->uartTx(saveBuffer, &bufferLen);
    rn4871ResponseProcess(dev, saveBuffer);
}

void virtualModuleDisconnect(struct rn4871_dev_s *dev) {
    assert(NULL != dev);

    strncpy(saveBuffer, "\%DISCONNECT\%", BUFFER_MAX_LEN);
    uint16_t bufferLen = strlen(saveBuffer);
    dev->uartTx(saveBuffer, &bufferLen);
    rn4871ResponseProcess(dev, saveBuffer);
}

void virtualModuleSetForceDataMode(void) {
    _command_mode = false;
}