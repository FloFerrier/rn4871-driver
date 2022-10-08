#include "virtual_module.h"
#include "rn4871_logger.h"

char saveBuffer[BUFFER_MAX_LEN+1] = "";
char tmpBuffer[BUFFER_MAX_LEN+1] = "";

static enum rn4871_cmd_e getCommand(const char *command);
static char *parseArgCommand(const char *command);

static enum rn4871_cmd_e getCommand(const char *command)
{
    assert(NULL != command);

    char tmp[256] = "";
    strncpy(tmp, command, 255);

    char delimiter[] = ",\r\n";
    char *saveptr;
    char *token = strtok_r(tmp, delimiter, &saveptr);
    for(int i=0; i< CMD_NONE; i++)
    {
        if(NULL != token)
        {
            if(0 == strcmp(token, TABLE_COMMAND[i]))
            {
                logger(LOG_DEBUG, "getCommand: find command %s\r\n", TABLE_COMMAND[i]);
                return i;
            }
        }
    }
    return CMD_NONE;
}

static char *parseArgCommand(const char *command)
{
    assert(NULL != command);

    char tmp[256] = "";
    strncpy(tmp, command, 255);

    char delimiter[] = ",\r\n";
    char *saveptr;
    char *token = strtok_r(tmp, delimiter, &saveptr);
    if(NULL != token)
    {
        token = strtok_r(NULL, delimiter, &saveptr);
        logger(LOG_DEBUG, "parseArgCommand: find arg %s\r\n", token);
        return token;
    }
    return NULL;
}

void virtualModuleInit(struct virtual_module_s *virtualModule)
{
    assert(NULL != virtualModule);
    strcpy(virtualModule->moduleName, "RN4871-VM");
    strcpy(virtualModule->macAddress, "001122334455");
    strcpy(virtualModule->firmwareVersion, "V1.40");
    virtualModule->services = DEVICE_INFORMATION;
    virtualModule->_command_mode = false;
    strcpy(virtualModule->_global_buffer, "");
}

void virtualModuleReceiveData(struct virtual_module_s *virtualModule, const char *dataReceived)
{
    assert(NULL != dataReceived);

    logger(LOG_DEBUG, "virtualModuleReceiveData: \"%s\"\r\n", dataReceived);

    memset(virtualModule->_global_buffer, '\0', BUFFER_MAX_LEN);
    memset(tmpBuffer, '\0', BUFFER_MAX_LEN);
    static uint8_t cnt_cmd_mode = 0;
    if(virtualModule->_command_mode)
    {
        char *arg;
        strncpy(tmpBuffer, dataReceived, BUFFER_MAX_LEN);
        enum rn4871_cmd_e command = getCommand(tmpBuffer);
        if(CMD_NONE != command)
        {
            switch(command)
            {
                case CMD_MODE_QUIT :
                {
                    snprintf(virtualModule->_global_buffer, BUFFER_MAX_LEN, "END\r\n");
                    virtualModule->_command_mode = false;
                    break;
                }
                case CMD_CLEAR_ALL_SERVICES :
                {
                    snprintf(virtualModule->_global_buffer, BUFFER_MAX_LEN, "AOK\r\nCMD> ");
                    break;
                }
                case CMD_RESET_FACTORY :
                case CMD_REBOOT :
                {
                    strncpy(virtualModule->_global_buffer, "Rebooting\r\nCMD> ", BUFFER_MAX_LEN);
                    virtualModule->_command_mode = false;
                    break;
                }
                case CMD_GET_DEVICE_NAME :
                {
                    snprintf(virtualModule->_global_buffer, BUFFER_MAX_LEN, "%s\r\nCMD> ", virtualModule->moduleName);
                    break;
                }
                case CMD_DUMP_INFOS :
                {
                    snprintf(virtualModule->_global_buffer, BUFFER_MAX_LEN,
                        "BTA=%s\r\nName=%s\r\nConnected=no\r\nAuthen=0\r\nFeatures=0000\r\nServices=%X\r\nCMD> ",
                        virtualModule->macAddress, virtualModule->moduleName, virtualModule->services);
                    break;
                }
                case CMD_GET_VERSION :
                {
                    snprintf(virtualModule->_global_buffer, BUFFER_MAX_LEN, "RN4871 %s 7/9/2019 (c)Microship Technology Inc\r\nCMD> ",
                        virtualModule->firmwareVersion);
                    break;
                }
                case CMD_SET_SERVICES :
                {
                    arg = parseArgCommand(dataReceived);
                    virtualModule->services = (uint16_t)strtol(arg, NULL, 16);
                    snprintf(virtualModule->_global_buffer, BUFFER_MAX_LEN, "AOK\r\nCMD> ");
                    break;
                }
                case CMD_SET_DEVICE_NAME :
                {
                    arg = parseArgCommand(dataReceived);
                    if(BUFFER_MAX_LEN >= strlen(arg))
                    {
                        strcpy(virtualModule->moduleName, arg);
                    }
                    snprintf(virtualModule->_global_buffer, BUFFER_MAX_LEN, "AOK\r\nCMD> ");
                    break;
                }
                default :
                    snprintf(virtualModule->_global_buffer, BUFFER_MAX_LEN, "Err\r\nCMD> ");
                    break;
            }
        }
        else
        {
            snprintf(virtualModule->_global_buffer, BUFFER_MAX_LEN, "Err\r\nCMD> ");
        }
    }
    else if(0 == strcmp(dataReceived, "$"))
    {
        ++cnt_cmd_mode;
        if (3 == cnt_cmd_mode)
        {
            snprintf(virtualModule->_global_buffer, BUFFER_MAX_LEN, "CMD> ");
            cnt_cmd_mode = 0;
            virtualModule->_command_mode = true;
        }
    }
    // Data mode
    else
    {
        strncpy(virtualModule->_global_buffer, dataReceived, BUFFER_MAX_LEN);
    }
}

void virtualModuleSendData(struct virtual_module_s *virtualModule, char *dataToSend, uint16_t *dataToSendLen)
{
    assert((NULL != dataToSend) || (NULL != dataToSendLen));

    strncpy(dataToSend, virtualModule->_global_buffer, BUFFER_MAX_LEN);
    *dataToSendLen = strnlen(virtualModule->_global_buffer, BUFFER_MAX_LEN);
    logger(LOG_DEBUG, "virtualModuleSendData: [%d] \"%s\"\r\n", *dataToSendLen, dataToSend);
}

/*void virtualModuleConnect(struct rn4871_dev_s *dev)
{
    assert(NULL != dev);

    strncpy(saveBuffer, "\%CONNECT,0,AABBCCDDEEFF\%", BUFFER_MAX_LEN);
    uint16_t bufferLen = strlen(saveBuffer);
    dev->uartTx(saveBuffer, &bufferLen);
}*/

/*void virtualModuleStream(struct rn4871_dev_s *dev)
{
    assert(NULL != dev);

    strncpy(saveBuffer, "\%STREAM_OPEN\%", BUFFER_MAX_LEN);
    uint16_t bufferLen = strlen(saveBuffer);
    dev->uartTx(saveBuffer, &bufferLen);
}*/

/*void virtualModuleDisconnect(struct rn4871_dev_s *dev)
{
    assert(NULL != dev);

    strncpy(saveBuffer, "\%DISCONNECT\%", BUFFER_MAX_LEN);
    uint16_t bufferLen = strlen(saveBuffer);
    dev->uartTx(saveBuffer, &bufferLen);
}*/

/*void virtualModuleSetForceDataMode(void)
{
    virtualModule->_command_mode = false;
}*/