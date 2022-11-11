#include "rn4871_defs.h"
#include "rn4871_api.h"
#include "rn4871_logger.h"

#define RN4871_DELAY_TO_RESPECT_MS 100
#define BASE_HEXADECIMAL 16

typedef enum
{
    FIELD_MAC_ADDRESS,
    FIELD_DEVICE_NAME,
    FIELD_CONNECTION,
    FIELD_AUTHENTIFICATION,
    FIELD_FEATURES,
    FIELD_SERVICES,
} RN4871_DUMP_INFOS_FIELD;

static const char DUMP_INFOS_FIELD[][10] =
{
    "BTA=",
    "Name=",
    "Connected=",
    "Authen=",
    "Features=",
    "Services=",
};

static RN4871_CODE_RETURN rn4871SendCmd(RN4871_MODULE *dev, RN4871_CMD cmd, const char *format, ...);
static RN4871_CODE_RETURN rn4871ResponseProcess(RN4871_MODULE *dev, const char *response);
static RN4871_CODE_RETURN rn4871ParseDumpInfos(const char *infos, RN4871_DUMP_INFOS_FIELD field, char *result, uint16_t resultMaxLen);
static RN4871_CODE_RETURN rn4871ParseFirmwareVersion(const char *firmwareVersion, char *result, uint16_t resultMaxLen);

RN4871_CODE_RETURN rn4871Init(RN4871_MODULE *dev)
{
    assert(NULL != dev);

    if((NULL == dev->delayMs) || (NULL == dev->uartTx) || (NULL == dev->uartRx))
    {
        return CODE_RETURN_ERROR;
    }

    dev->_currentCmd = CMD_NONE;
    dev->_currentMode = DATA_MODE;
    dev->_fsmState = FSM_STATE_NONE;

    logger(LOG_INFO, "rn4871 Init pass with success !\r\n");

    return CODE_RETURN_SUCCESS;
}

RN4871_CODE_RETURN rn4871SendCmd(RN4871_MODULE *dev, RN4871_CMD cmd, const char *format, ...)
{
    assert(NULL != dev);

    va_list args;
    if(NULL != format)
    {
        va_start(args, format);
    }

	char command[RN4871_BUFFER_UART_LEN_MAX+1] = "";
	uint16_t commandLen = 0;
	RN4871_CODE_RETURN ret = CODE_RETURN_ERROR;

    if((COMMAND_MODE != dev->_currentMode) && (CMD_MODE_ENTER != cmd))
    {
        logger(LOG_ERROR, "rn4871SendCmd: module is not on command mode ...\r\n");
        if(NULL != format)
        {
            va_end(args);
        }
        return CODE_RETURN_NO_COMMAND_MODE;
    }

    switch(cmd)
    {
        /* Commands without argument */
		case CMD_MODE_ENTER :
        {
            command[0] = '$';
            commandLen = 1;
            ret = dev->uartTx(command, &commandLen);
            dev->delayMs(RN4871_DELAY_TO_RESPECT_MS);
            ret = dev->uartTx(command, &commandLen);
            dev->delayMs(RN4871_DELAY_TO_RESPECT_MS);
            ret = dev->uartTx(command, &commandLen);
            break;
        }
        case CMD_MODE_QUIT :
        case CMD_DUMP_INFOS :
        case CMD_GET_DEVICE_NAME :
        case CMD_GET_VERSION :
        case CMD_CLEAR_ALL_SERVICES :
        {
            commandLen = snprintf(command, RN4871_BUFFER_UART_LEN_MAX, "%s\r\n", TABLE_COMMAND_STR[cmd]);
            ret = dev->uartTx(command, &commandLen);
            break;
        }
        /* Commands with arguments */
        case CMD_REBOOT :
		case CMD_RESET_FACTORY :
        case CMD_SET_SERVICES :
        case CMD_SET_DEVICE_NAME :
		case CMD_SET_BT_NAME :
        case CMD_CREATE_PRIVATE_SERVICE :
        case CMD_SERVER_READ_CHARACTERISTIC :
        case CMD_SERVER_WRITE_CHARACTERISTIC :
        case CMD_CREATE_PRIVATE_CHARACTERISTIC :
        {
            char pArgs[RN4871_BUFFER_UART_LEN_MAX] = "";
            vsnprintf(pArgs, RN4871_BUFFER_UART_LEN_MAX, format, args);
			commandLen = snprintf(command, RN4871_BUFFER_UART_LEN_MAX, "%s,%s\r\n", TABLE_COMMAND_STR[cmd], pArgs);
            ret = dev->uartTx(command, &commandLen);
			break;
        }
        /* Unknown command */
        default :
			ret = CODE_RETURN_CMD_UNKNOWN;
        	break;
    }
    dev->_currentCmd = cmd;
    if(NULL != format)
    {
        va_end(args);
    }

    logger(LOG_DEBUG, "rn4871SendCmd: [%d] \"%s\"\r\n", commandLen, command);

	return ret;
}

RN4871_CODE_RETURN rn4871ResponseProcess(RN4871_MODULE *dev, const char *response)
{
    assert((NULL != dev) || (NULL != response));

	RN4871_CODE_RETURN ret = CODE_RETURN_ERROR;
    RN4871_CMD cmd = dev->_currentCmd;

    logger(LOG_DEBUG, "rn4871ResponseProcess: [%d] \"%s\"\r\n", strlen(response), response);

    if((NULL != strstr(response, "AOK")) || (NULL != strstr(response, "CMD>")) || (NULL != strstr(response, "END")) || (NULL != strstr(response, "Rebooting")))
    {
        dev->_fsmState = FSM_STATE_INIT;

        /* Check if error is returned */
        if(NULL != strstr(response, "Err"))
        {
            ret = CODE_RETURN_ERROR;
        }
        /* Parse and get data from response */
        else
        {
            switch(cmd)
            {
                case CMD_MODE_ENTER :
                {
                    ret = CODE_RETURN_SUCCESS;
                    dev->_currentMode = COMMAND_MODE;
                    break;
                }
                case CMD_MODE_QUIT :
                {
                    ret = CODE_RETURN_SUCCESS;
                    dev->_currentMode = DATA_MODE;
                    break;
                }
                case CMD_SET_BT_NAME :
                case CMD_SET_DEVICE_NAME :
                case CMD_SET_SERVICES :
                case CMD_CLEAR_ALL_SERVICES :
                case CMD_CREATE_PRIVATE_SERVICE :
                case CMD_CREATE_PRIVATE_CHARACTERISTIC :
                case CMD_SERVER_WRITE_CHARACTERISTIC :
                case CMD_DUMP_INFOS :
                case CMD_GET_DEVICE_NAME :
                case CMD_GET_VERSION :
                case CMD_SERVER_READ_CHARACTERISTIC :
                {
                    ret = CODE_RETURN_SUCCESS ;
                    break;
                }
                case CMD_REBOOT :
                case CMD_RESET_FACTORY :
                {
                    dev->_currentMode = DATA_MODE;
                    dev->_fsmState = FSM_STATE_IDLE;
                    ret = CODE_RETURN_SUCCESS;
                    break;
                }
                default :
                    ret = CODE_RETURN_CMD_UNKNOWN;
                    break;
            }
        }
    }
	return ret;
}

RN4871_CODE_RETURN rn4871WaitReceivedData(RN4871_MODULE *dev, char *receivedData, uint16_t *receivedDataLen)
{
    assert(NULL != dev);

    if(DATA_MODE != dev->_currentMode)
    {
        return CODE_RETURN_NO_DATA_MODE;
    }

    dev->uartRx(receivedData, receivedDataLen);

    logger(LOG_DEBUG, "rn4871ReceivedDataProcess: [%d] \"%s\"\r\n", *receivedDataLen, receivedData);

    if(NULL != strstr(receivedData, "Rebooting"))
    {
        dev->_fsmState = FSM_STATE_IDLE;
    }
    else if(NULL != strstr(receivedData, "DISCONNECT"))
    {
        dev->_fsmState = FSM_STATE_IDLE;
    }
    else if(NULL != strstr(receivedData, "CONNECT"))
    {
        dev->_fsmState = FSM_STATE_CONNECTED;
    }
    else if(NULL != strstr(receivedData, "STREAM_OPEN"))
    {
        dev->_fsmState = FSM_STATE_STREAMING;
    }

    return CODE_RETURN_SUCCESS;
}

RN4871_CODE_RETURN rn4871EnterCommandMode(RN4871_MODULE *dev)
{
    assert(NULL != dev);

    RN4871_CODE_RETURN ret = CODE_RETURN_ERROR;
    char response[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_MODE_ENTER, NULL);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = rn4871ResponseProcess(dev, response);
    return ret;
}

RN4871_CODE_RETURN rn4871QuitCommandMode(RN4871_MODULE *dev)
{
    assert(NULL != dev);

    RN4871_CODE_RETURN ret = CODE_RETURN_ERROR;
    char response[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_MODE_QUIT, NULL);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = rn4871ResponseProcess(dev, response);
    return ret;
}

RN4871_CODE_RETURN rn4871RebootModule(RN4871_MODULE *dev)
{
    assert(NULL != dev);

    RN4871_CODE_RETURN ret = CODE_RETURN_ERROR;
    char response[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_REBOOT, "1");
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = rn4871ResponseProcess(dev, response);
    return ret;
}

RN4871_CODE_RETURN rn4871SetServices(RN4871_MODULE *dev, uint16_t service)
{
    assert(NULL != dev);

    RN4871_CODE_RETURN ret = CODE_RETURN_ERROR;
    char response[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_SET_SERVICES, "%X", service);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = rn4871ResponseProcess(dev, response);
    return ret;
}

RN4871_CODE_RETURN rn4871SetDeviceName(RN4871_MODULE *dev, const char *deviceName)
{
    assert((NULL != dev) || (NULL != deviceName));

    RN4871_CODE_RETURN ret = CODE_RETURN_ERROR;
    char response[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    uint16_t deviceNameLen = (uint16_t) strlen(deviceName);
    if((0 >= deviceNameLen) || (DEVICE_NAME_LEN_MAX < deviceNameLen))
    {
        logger(LOG_ERROR, "rn4871SetDeviceName: deviceNameLen is incorrect ... [%d]\r\n", deviceNameLen);
        return CODE_RETURN_ERROR;
    }

    ret = rn4871SendCmd(dev, CMD_SET_DEVICE_NAME, deviceName);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = rn4871ResponseProcess(dev, response);
    return ret;
}

RN4871_CODE_RETURN rn4871SetConfig(RN4871_MODULE *dev, RN4871_CONFIG *config)
{
    assert((NULL != dev) || (NULL != config));

    RN4871_CODE_RETURN ret = CODE_RETURN_ERROR;
    ret = rn4871SetDeviceName(dev, config->deviceName);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }

    ret = rn4871SetServices(dev, config->services);
    return ret;
}

RN4871_CODE_RETURN rn4871GetDeviceName(RN4871_MODULE *dev, char *deviceName, uint16_t deviceNameMaxLen)
{
    assert((NULL != dev) || (NULL != deviceName));

    char infos[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    RN4871_CODE_RETURN ret = rn4871DumpInfos(dev, infos);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = rn4871ParseDumpInfos(infos, FIELD_DEVICE_NAME, deviceName, deviceNameMaxLen);
    return ret;
}

RN4871_CODE_RETURN rn4871ParseFirmwareVersion(const char *firmwareVersion, char *result, uint16_t resultMaxLen)
{
    assert((NULL != firmwareVersion) || (NULL != result));

    RN4871_CODE_RETURN ret = CODE_RETURN_ERROR;
    char delimiter[] = " \r\n";
    char *token = strtok((char*)firmwareVersion, delimiter);
    do
    {
        if(NULL != strstr(token, "V"))
        {
            ret = CODE_RETURN_SUCCESS;
            break;
        }
        token = strtok(NULL, delimiter);
    } while(NULL != token);

    if(CODE_RETURN_SUCCESS == ret)
    {
        strncpy(result, token, resultMaxLen);
    }
    return ret;
}

RN4871_CODE_RETURN rn4871GetFirmwareVersion(RN4871_MODULE *dev, char *firmwareVersion, uint16_t firmwareVersionMaxLen)
{
    assert((NULL != dev) || (NULL != firmwareVersion));

    RN4871_CODE_RETURN ret = CODE_RETURN_ERROR;
    char response[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_GET_VERSION, NULL);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = rn4871ResponseProcess(dev, response);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = rn4871ParseFirmwareVersion(response, firmwareVersion, firmwareVersionMaxLen);
    return ret;
}

RN4871_CODE_RETURN rn4871DumpInfos(RN4871_MODULE *dev, char *infos)
{
    assert((NULL != dev) || (NULL != infos));

    RN4871_CODE_RETURN ret = CODE_RETURN_ERROR;
    char response[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_DUMP_INFOS, NULL);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = rn4871ResponseProcess(dev, response);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }

    strncpy(infos, response, RN4871_BUFFER_UART_LEN_MAX);
    return ret;
}

RN4871_CODE_RETURN rn4871ParseDumpInfos(const char *infos, RN4871_DUMP_INFOS_FIELD field, char *result, uint16_t resultMaxLen)
{
    assert((NULL != infos) || (NULL != result));

    char delimiter[] = "\r\n";
    char *token = strtok((char*)infos, delimiter);
    do
    {
        if(NULL != strstr(token, DUMP_INFOS_FIELD[field]))
        {
            break;
        }
        token = strtok(NULL, delimiter);
    } while(NULL != token);

    if(NULL == token)
    {
        logger(LOG_ERROR, "rn4871ParseDumpInfos: string infos is empty ...\r\n");
        return CODE_RETURN_ERROR;
    }
    char *tmp;
    tmp = strtok(token, "=");
    tmp = strtok(NULL, "=");
    if(NULL == tmp)
    {
        return CODE_RETURN_ERROR;
    }
    strncpy(result, tmp, resultMaxLen);
    return CODE_RETURN_SUCCESS;
}

RN4871_CODE_RETURN rn4871GetMacAddress(RN4871_MODULE *dev, char *macAddress, uint16_t macAddressMaxLen)
{
    assert((NULL != dev) || (NULL != macAddress));

    char infos[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    RN4871_CODE_RETURN ret = rn4871DumpInfos(dev, infos);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = rn4871ParseDumpInfos(infos, FIELD_MAC_ADDRESS, macAddress, macAddressMaxLen);
    return ret;
}

RN4871_CODE_RETURN rn4871GetServices(RN4871_MODULE *dev, uint16_t *services)
{
    assert((NULL != dev) || (NULL != services));

    char infos[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    RN4871_CODE_RETURN ret = rn4871DumpInfos(dev, infos);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    char tmp[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    ret = rn4871ParseDumpInfos(infos, FIELD_SERVICES, tmp, RN4871_BUFFER_UART_LEN_MAX);
    *services = (uint16_t)strtol(tmp, NULL, BASE_HEXADECIMAL);
    return ret;
}

RN4871_CODE_RETURN rn4871EraseAllGattServices(RN4871_MODULE *dev)
{
    assert(NULL != dev);

    RN4871_CODE_RETURN ret = CODE_RETURN_ERROR;
    char response[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_CLEAR_ALL_SERVICES, NULL);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = rn4871ResponseProcess(dev, response);
    return ret;
}

RN4871_CODE_RETURN rn4871GetConfig(RN4871_MODULE *dev, RN4871_CONFIG *config)
{
    assert((NULL != dev) || (NULL != config));

    RN4871_CODE_RETURN ret = CODE_RETURN_ERROR;
    char infos[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    ret = rn4871DumpInfos(dev, infos);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    char save[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    strncpy(save, infos, RN4871_BUFFER_UART_LEN_MAX);

    char tmp[RN4871_BUFFER_UART_LEN_MAX+1] = "";
    ret = rn4871ParseDumpInfos(save, FIELD_SERVICES, tmp, RN4871_BUFFER_UART_LEN_MAX);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    strncpy(save, infos, RN4871_BUFFER_UART_LEN_MAX);
    config->services = (uint16_t)strtol(tmp, NULL, BASE_HEXADECIMAL);
    ret = rn4871ParseDumpInfos(save, FIELD_MAC_ADDRESS, config->macAddress, MAC_ADDRESS_LEN_MAX);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    strncpy(save, infos, RN4871_BUFFER_UART_LEN_MAX);
    ret = rn4871ParseDumpInfos(save, FIELD_DEVICE_NAME, config->deviceName, DEVICE_NAME_LEN_MAX);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = rn4871GetFirmwareVersion(dev, config->firmwareVersion, FW_VERSION_LEN_MAX);
    return ret;
}

RN4871_CODE_RETURN rn4871TransparentUartSendData(RN4871_MODULE *dev, const char *dataToSend, uint16_t dataToSendLen)
{
    assert((NULL != dev) || (NULL != dataToSend));

    /* Must on data mode */
    if(DATA_MODE != dev->_currentMode)
    {
        logger(LOG_ERROR, "rn4871TransparentUartSendData: module is not on DATA mode ...\r\n");
        return CODE_RETURN_NO_DATA_MODE;
    }

    /* Must on streaming state */
    if(FSM_STATE_STREAMING != rn4871GetFsmState(dev))
    {
        logger(LOG_ERROR, "rn4871TransparentUartSendData: fsm is not on STREAMING mode ...\r\n");
        return CODE_RETURN_NO_STREAMING;
    }

    logger(LOG_DEBUG, "rn4871TransparentUartSendData: [%d] \"%s\"\r\n", dataToSendLen, dataToSend);
    return dev->uartTx((char*)dataToSend, &dataToSendLen);
}

RN4871_FSM rn4871GetFsmState(RN4871_MODULE *dev)
{
    assert(NULL != dev);
    return dev->_fsmState;
}

void rn4871SetForceFsmState(RN4871_MODULE *dev, RN4871_FSM fsmForceState)
{
    assert(NULL != dev);
    dev->_fsmState = fsmForceState;
}

void rn4871SetForceDataMode(RN4871_MODULE *dev)
{
    assert(NULL != dev);
    dev->_currentMode = DATA_MODE;
}

RN4871_CODE_RETURN rn4871IsOnTransparentUart(RN4871_MODULE *dev, bool *result)
{
    assert((NULL != dev) || (NULL != result));
    RN4871_CODE_RETURN ret = CODE_RETURN_ERROR;

    *result = false;
    uint16_t services = 0x00;
    ret = rn4871GetServices(dev, &services);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }

    if(UART_TRANSPARENT & services)
    {
        *result = true;
    }

    return CODE_RETURN_SUCCESS;
}

char* rn4871GetErrorCodeStr(RN4871_CODE_RETURN errorCode)
{
    return ((char*) ERROR_CODE_STR[errorCode]);
}