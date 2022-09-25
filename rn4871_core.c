#include "rn4871_defs.h"
#include "rn4871_api.h"
#include "rn4871_logger.h"

#define RN4871_DELAY_TO_RESPECT_MS 100
#define BASE_HEXADECIMAL 16

enum dump_infos_field_e
{
    FIELD_MAC_ADDRESS,
    FIELD_DEVICE_NAME,
    FIELD_CONNECTION,
    FIELD_AUTHENTIFICATION,
    FIELD_FEATURES,
    FIELD_SERVICES,
};

static const char DUMP_INFOS_FIELD[][10] =
{
    "BTA=",
    "Name=",
    "Connected=",
    "Authen=",
    "Features=",
    "Services=",
};

static uint8_t rn4871SendCmd(struct rn4871_dev_s *dev, enum rn4871_cmd_e cmd, const char *format, ...);
static uint8_t rn4871ResponseProcess(struct rn4871_dev_s *dev, const char *response);
static uint8_t rn4871ParseDumpInfos(const char *infos, enum dump_infos_field_e field, char *result);
static uint8_t rn4871ParseFirmwareVersion(const char *firmwareVersion, char *result);

uint8_t rn4871Init(struct rn4871_dev_s *dev)
{
    assert(NULL != dev);

    if((NULL == dev->delayMs) || (NULL == dev->uartTx) || (NULL == dev->uartRx))
    {
        return CODE_RETURN_ERROR;
    }

    dev->_currentCmd = CMD_NONE;
    dev->_currentMode = DATA_MODE;
    dev->_fsmState = FSM_STATE_NONE;

    return CODE_RETURN_SUCCESS;
}

uint8_t rn4871SendCmd(struct rn4871_dev_s *dev, enum rn4871_cmd_e cmd, const char *format, ...)
{
    assert(NULL != dev);

    va_list args;
    if(NULL != format)
    {
        va_start(args, format);
    }

	char command[BUFFER_UART_LEN_MAX+1] = "";
	uint16_t commandLen = 0;
	uint8_t ret = CODE_RETURN_ERROR;

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
            commandLen = snprintf(command, BUFFER_UART_LEN_MAX, "%s\r\n", TABLE_COMMAND[cmd]);
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
            char pArgs[BUFFER_UART_LEN_MAX] = "";
            vsnprintf(pArgs, BUFFER_UART_LEN_MAX, format, args);
			commandLen = snprintf(command, BUFFER_UART_LEN_MAX, "%s,%s\r\n", TABLE_COMMAND[cmd], pArgs);
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

uint8_t rn4871ResponseProcess(struct rn4871_dev_s *dev, const char *response)
{
    assert((NULL != dev) || (NULL != response));

	uint8_t ret = CODE_RETURN_ERROR;
    enum rn4871_cmd_e cmd = dev->_currentCmd;

    logger(LOG_DEBUG, "rn4871ResponseProcess: [%d] \"%s\"\r\n", strlen(response), response);

    if((NULL != strstr(response, "AOK")) || (NULL != strstr(response, "CMD>")) || (NULL != strstr(response, "END")))
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

uint8_t rn4871WaitReceivedData(struct rn4871_dev_s *dev, char *receivedData, uint16_t *receivedDataLen)
{
    assert(NULL != dev);

    if(DATA_MODE != dev->_currentMode)
    {
        return CODE_RETURN_NO_DATA_MODE;
    }

    dev->uartRx(receivedData, receivedDataLen);

    logger(LOG_DEBUG, "rn4871ReceivedDataProcess: [%d] \"%s\"\r\n", *receivedDataLen, receivedData);

    if(NULL != strstr(receivedData, "REBOOT"))
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

uint8_t rn4871EnterCommandMode(struct rn4871_dev_s *dev)
{
    assert(NULL != dev);

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
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

uint8_t rn4871QuitCommandMode(struct rn4871_dev_s *dev)
{
    assert(NULL != dev);

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
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

uint8_t rn4871RebootModule(struct rn4871_dev_s *dev)
{
    assert(NULL != dev);

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
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
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    return ret;
}

uint8_t rn4871SetServices(struct rn4871_dev_s *dev, uint16_t service)
{
    assert(NULL != dev);

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
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

uint8_t rn4871SetDeviceName(struct rn4871_dev_s *dev, const char *deviceName)
{
    assert((NULL != dev) || (NULL != deviceName));

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    uint16_t deviceNameLen = (uint16_t) strlen(deviceName);
    if((0 >= deviceNameLen) || (DEVICE_NAME_CHARACTER_MAX < deviceNameLen))
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

uint8_t rn4871GetDeviceName(struct rn4871_dev_s *dev, char *deviceName)
{
    assert((NULL != dev) || (NULL != deviceName));

    char infos[BUFFER_UART_LEN_MAX+1] = "";
    uint8_t ret = rn4871DumpInfos(dev, infos);
    uint16_t infosSize = strlen(infos);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = rn4871ParseDumpInfos(infos, FIELD_DEVICE_NAME, deviceName);
    return ret;
}

uint8_t rn4871ParseFirmwareVersion(const char *firmwareVersion, char *result)
{
    assert((NULL != firmwareVersion) || (NULL != result));

    uint8_t ret = CODE_RETURN_ERROR;
    char *saveptr;
    char delimiter[] = " \r\n";
    char *token = strtok_r((char*)firmwareVersion, delimiter, &saveptr);
    do
    {
        if(NULL != strstr(token, "V"))
        {
            ret = CODE_RETURN_SUCCESS;
            break;
        }
        token = strtok_r(NULL, delimiter, &saveptr);
    } while(NULL != token);

    if(CODE_RETURN_SUCCESS == ret)
    {
        strncpy(result, token, BUFFER_UART_LEN_MAX);
    }
    return ret;
}

uint8_t rn4871GetFirmwareVersion(struct rn4871_dev_s *dev, char *firmwareVersion)
{
    assert((NULL != dev) || (NULL != firmwareVersion));

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
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
    ret = rn4871ParseFirmwareVersion(response, firmwareVersion);
    return ret;
}

uint8_t rn4871DumpInfos(struct rn4871_dev_s *dev, char *infos)
{
    assert((NULL != dev) || (NULL != infos));

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
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

    strncpy(infos, response, BUFFER_UART_LEN_MAX);
    return ret;
}

uint8_t rn4871ParseDumpInfos(const char *infos, enum dump_infos_field_e field, char *result)
{
    assert((NULL != infos) || (NULL != result));

    char *saveptr;
    char delimiter[] = "\r\n";
    char *token = strtok_r((char*)infos, delimiter, &saveptr);
    do
    {
        if(NULL != strstr(token, DUMP_INFOS_FIELD[field]))
        {
            break;
        }
        token = strtok_r(NULL, delimiter, &saveptr);
    } while(NULL != token);

    if(NULL == token)
    {
        logger(LOG_ERROR, "rn4871ParseDumpInfos: string infos is empty ...\r\n");
        return CODE_RETURN_ERROR;
    }
    char *tmp;
    tmp = strtok_r(token, "=", &saveptr);
    tmp = strtok_r(NULL, "=", &saveptr);
    if(NULL == tmp)
    {
        return CODE_RETURN_ERROR;
    }
    strncpy(result, tmp, BUFFER_UART_LEN_MAX);
    return CODE_RETURN_SUCCESS;
}

uint8_t rn4871GetMacAddress(struct rn4871_dev_s *dev, char *macAddress)
{
    assert((NULL != dev) || (NULL != macAddress));

    char infos[BUFFER_UART_LEN_MAX+1] = "";
    uint8_t ret = rn4871DumpInfos(dev, infos);
    uint16_t infosSize = strlen(infos);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    ret = rn4871ParseDumpInfos(infos, FIELD_MAC_ADDRESS, macAddress);
    return ret;
}

uint8_t rn4871GetServices(struct rn4871_dev_s *dev, uint16_t *services)
{
    assert((NULL != dev) || (NULL != services));

    char infos[BUFFER_UART_LEN_MAX+1] = "";
    uint8_t ret = rn4871DumpInfos(dev, infos);
    uint16_t infosSize = strlen(infos);
    if(CODE_RETURN_SUCCESS != ret)
    {
        return ret;
    }
    char tmp[BUFFER_UART_LEN_MAX+1] = "";
    ret = rn4871ParseDumpInfos(infos, FIELD_SERVICES, tmp);
    *services = (uint16_t)strtol(tmp, NULL, BASE_HEXADECIMAL);
    return ret;
}

uint8_t rn4871EraseAllGattServices(struct rn4871_dev_s *dev)
{
    assert(NULL != dev);

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
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

uint8_t rn4871TransparentUartSendData(struct rn4871_dev_s *dev, const char *dataToSend, uint16_t dataToSendLen)
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

enum rn4871_fsm_e rn4871GetFsmState(struct rn4871_dev_s *dev)
{
    assert(NULL != dev);
    return dev->_fsmState;
}

void rn4871SetForceFsmState(struct rn4871_dev_s *dev, enum rn4871_fsm_e fsmForceState)
{
    assert(NULL != dev);
    dev->_fsmState = fsmForceState;
}

void rn4871SetForceDataMode(struct rn4871_dev_s *dev)
{
    assert(NULL != dev);
    dev->_currentMode = DATA_MODE;
}

uint8_t rn4871IsOnTransparentUart(struct rn4871_dev_s *dev, bool *result)
{
    assert((NULL != dev) || (NULL != result));
    uint8_t ret = CODE_RETURN_ERROR;

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