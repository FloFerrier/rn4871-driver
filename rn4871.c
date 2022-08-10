#include "rn4871.h"
#include "rn4871_defs.h"
#include "utils.h"
#include "logs.h"

enum rn4871_mode_e {
    DATA_MODE,
    COMMAND_MODE,
};

enum dump_infos_field_e {
    FIELD_MAC_ADDRESS,
    FIELD_DEVICE_NAME,
    FIELD_CONNECTION,
    FIELD_AUTHENTIFICATION,
    FIELD_FEATURES,
    FIELD_SERVICES,
};

static const char DUMP_INFOS_FIELD[][10] = {
    "BTA=",
    "Name=",
    "Connected=",
    "Authen=",
    "Features=",
    "Services=",
};

static enum rn4871_mode_e _currentMode = DATA_MODE;
static enum rn4871_cmd_e _currentCmd = CMD_NONE;
static enum rn4871_fsm_e _fsmState = FSM_STATE_NONE;

static uint8_t rn4871SendCmd(struct rn4871_dev_s *dev, enum rn4871_cmd_e cmd, const char *format, ...);
static void rn4871ParseDumpInfos(const char *infos, enum dump_infos_field_e field, char *result);
static void rn4871ParseFirmwareVersion(const char *firmwareVersion, char *result);

uint8_t rn4871SendCmd(struct rn4871_dev_s *dev, enum rn4871_cmd_e cmd, const char *format, ...) {
    assert(NULL != dev);

    va_list args;
    if(NULL != format)
        va_start(args, format);

	uint8_t command[BUFFER_UART_LEN_MAX+1] = "";
	uint16_t commandLen = 0;
	uint8_t ret = CODE_RETURN_ERROR;

    if((COMMAND_MODE != _currentMode) && (CMD_MODE_ENTER != cmd)) {
        logger(LOG_ERROR, "rn4871SendCmd: module is not on command mode ...\r\n");
        va_end(args);
        return CODE_RETURN_NO_COMMAND_MODE;
    }

    switch(cmd) {
        /* Commands without argument */
		case CMD_MODE_ENTER: {
            command[0] = '$';
            commandLen = 1;
            ret = dev->uartTx(command, &commandLen);
            dev->delayMs(100);
            ret = dev->uartTx(command, &commandLen);
            dev->delayMs(100);
            ret = dev->uartTx(command, &commandLen);
            break;
        }
        case CMD_MODE_QUIT:
        case CMD_DUMP_INFOS:
        case CMD_GET_DEVICE_NAME:
        case CMD_GET_VERSION:
        case CMD_CLEAR_ALL_SERVICES: {
            commandLen = snprintf(command, BUFFER_UART_LEN_MAX, "%s\r\n", TABLE_COMMAND[cmd]);
            ret = dev->uartTx(command, &commandLen);
            break;
        }
        /* Commands with arguments */
        case CMD_REBOOT :
		case CMD_RESET_FACTORY :
        case CMD_SET_SERVICES:
        case CMD_SET_DEVICE_NAME:
		case CMD_SET_BT_NAME:
        case CMD_CREATE_PRIVATE_SERVICE:
        case CMD_SERVER_READ_CHARACTERISTIC:
        case CMD_SERVER_WRITE_CHARACTERISTIC:
        case CMD_CREATE_PRIVATE_CHARACTERISTIC: {
            uint8_t pArgs[BUFFER_UART_LEN_MAX] = "";
            vsnprintf(pArgs, BUFFER_UART_LEN_MAX, format, args);
			commandLen = snprintf(command, BUFFER_UART_LEN_MAX, "%s,%s\r\n", TABLE_COMMAND[cmd], pArgs);
            ret = dev->uartTx(command, &commandLen);
			break;
        }
        /* Unknown command */
        default:
			ret = CODE_RETURN_CMD_UNKNOWN;
        	break;
    }
    _currentCmd = cmd;
    va_end(args);

    logger(LOG_DEBUG, "rn4871SendCmd: [%d] \"%s\"\r\n", commandLen, command);

	return ret;
}

uint8_t rn4871ResponseProcess(struct rn4871_dev_s *dev, const char *response) {
    assert((NULL != dev) || (NULL != response));

	uint8_t ret = CODE_RETURN_ERROR;
    enum rn4871_cmd_e cmd = _currentCmd;

    if((NULL != strstr(response, "AOK")) || (NULL != strstr(response, "CMD>")) || (NULL != strstr(response, "REBOOT"))) {
        _fsmState = FSM_STATE_INIT;

        /* Check if error is returned */
        if(NULL != strstr(response, "Err"))
            ret = CODE_RETURN_ERROR;
        /* Parse and get data from response */
        else {
            switch(cmd) {
                case CMD_MODE_ENTER: {
                    ret = CODE_RETURN_SUCCESS;
                    _currentMode = COMMAND_MODE;
                    break;
                }
                case CMD_MODE_QUIT: {
                    ret = CODE_RETURN_SUCCESS;
                    _currentMode = DATA_MODE;
                    break;
                }
                case CMD_SET_BT_NAME:
                case CMD_SET_DEVICE_NAME:
                case CMD_SET_SERVICES:
                case CMD_CLEAR_ALL_SERVICES:
                case CMD_CREATE_PRIVATE_SERVICE:
                case CMD_CREATE_PRIVATE_CHARACTERISTIC:
                case CMD_SERVER_WRITE_CHARACTERISTIC:
                case CMD_DUMP_INFOS:
                case CMD_GET_DEVICE_NAME:
                case CMD_GET_VERSION:
                case CMD_SERVER_READ_CHARACTERISTIC: {
                    ret = CODE_RETURN_SUCCESS;
                    break;
                }
                case CMD_REBOOT:
                case CMD_RESET_FACTORY: {
                    _fsmState = FSM_STATE_IDLE;
                    _currentMode = DATA_MODE;
                    ret = CODE_RETURN_SUCCESS;
                    break;
                }
                default:
                    ret = CODE_RETURN_CMD_UNKNOWN;
                    break;
            }
        }
    }
    else if(NULL != strstr(response, "STREAM_OPEN")) {
        _fsmState = FSM_STATE_STREAMING;
		ret = CODE_RETURN_SUCCESS;
    }
    else if(NULL != strstr(response, "DISCONNECT")) {
        _fsmState = FSM_STATE_IDLE;
		ret = CODE_RETURN_SUCCESS;
    }
    else if(NULL != strstr(response, "CONNECT")) {
        _fsmState = FSM_STATE_CONNECTED;
		ret = CODE_RETURN_SUCCESS;
    }
	return ret;
}

uint8_t rn4871EnterCommandMode(struct rn4871_dev_s *dev) {
    assert(NULL != dev);

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_MODE_ENTER, NULL);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = rn4871ResponseProcess(dev, response);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    return ret;
}

uint8_t rn4871QuitCommandMode(struct rn4871_dev_s *dev) {
    assert(NULL != dev);

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_MODE_QUIT, NULL);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = rn4871ResponseProcess(dev, response);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    return ret;
}

uint8_t rn4871RebootModule(struct rn4871_dev_s *dev) {
    assert(NULL != dev);

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
    char proceededResponse[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_REBOOT, "1");
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = rn4871ResponseProcess(dev, response);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    _fsmState = FSM_STATE_IDLE;
    return ret;
}

uint8_t rn4871SetServices(struct rn4871_dev_s *dev, uint16_t service) {
    assert(NULL != dev);

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_SET_SERVICES, "%X", service);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = rn4871ResponseProcess(dev, response);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    return ret;
}

uint8_t rn4871SetDeviceName(struct rn4871_dev_s *dev, const char *deviceName, uint16_t deviceNameLen) {
    assert((NULL != dev) || (NULL != deviceName) || (DEVICE_NAME_CHARACTER_MAX >= deviceNameLen));

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_SET_DEVICE_NAME, deviceName);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = rn4871ResponseProcess(dev, response);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    if(0 >= strlen(deviceName)) {
        logger(LOG_ERROR, "rn4871SetDeviceName: string device name is empty ...\r\n");
        return CODE_RETURN_ERROR;
    }

    return ret;
}

uint8_t rn4871GetDeviceName(struct rn4871_dev_s *dev, char *deviceName) {
    assert((NULL != dev) || (NULL != deviceName));

    char infos[BUFFER_UART_LEN_MAX+1] = "";
    uint8_t ret = rn4871DumpInfos(dev, infos);
    uint16_t infosSize = strlen(infos);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    if(0 >= infosSize) {
        logger(LOG_ERROR, "rn4871GetDeviceName: string device name is empty ...\r\n");
        return CODE_RETURN_ERROR;
    }

    rn4871ParseDumpInfos(infos, FIELD_DEVICE_NAME, deviceName);
    return ret;
}

void rn4871ParseFirmwareVersion(const char *firmwareVersion, char *result) {
    assert((NULL != firmwareVersion) || (NULL != result));

    char *saveptr;
    char delimiter[] = " \r\n";
    char *token = strtok_r((char*)firmwareVersion, delimiter, &saveptr);
    do {
        if(NULL != strstr(token, "V")) {
            break;
        }
        token = strtok_r(NULL, delimiter, &saveptr);
    } while(NULL != token);

    strncpy(result, token, BUFFER_UART_LEN_MAX);
}

uint8_t rn4871GetFirmwareVersion(struct rn4871_dev_s *dev, char *firmwareVersion) {
    assert((NULL != dev) || (NULL != firmwareVersion));

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_GET_VERSION, NULL);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = rn4871ResponseProcess(dev, response);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    rn4871ParseFirmwareVersion(response, firmwareVersion);
    return ret;
}

uint8_t rn4871DumpInfos(struct rn4871_dev_s *dev, char *infos) {
    assert((NULL != dev) || (NULL != infos));

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_DUMP_INFOS, NULL);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    if(0 >= responseSize) {
        logger(LOG_ERROR, "rn4871DumpInfos: string infos is empty ...\r\n");
        return CODE_RETURN_ERROR;
    }
    ret = rn4871ResponseProcess(dev, response);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    strncpy(infos, response, BUFFER_UART_LEN_MAX);
    return ret;
}

void rn4871ParseDumpInfos(const char *infos, enum dump_infos_field_e field, char *result) {
    assert((NULL != infos) || (NULL != result));

    char *saveptr;
    char delimiter[] = "\r\n";
    char *token = strtok_r((char*)infos, delimiter, &saveptr);
    do {
        if(NULL != strstr(token, DUMP_INFOS_FIELD[field])) {
            break;
        }
        token = strtok_r(NULL, delimiter, &saveptr);
    } while(NULL != token);

    if(NULL == token) {
        logger(LOG_ERROR, "rn4871ParseDumpInfos: string infos is empty ...\r\n");
        return;
    }

    char *tmp;
    tmp = strtok_r(token, "=", &saveptr);
    tmp = strtok_r(NULL, "=", &saveptr);
    strncpy(result, tmp, BUFFER_UART_LEN_MAX);
}

uint8_t rn4871GetMacAddress(struct rn4871_dev_s *dev, char *macAddress) {
    assert((NULL != dev) || (NULL != macAddress));

    char infos[BUFFER_UART_LEN_MAX+1] = "";
    uint8_t ret = rn4871DumpInfos(dev, infos);
    uint16_t infosSize = strlen(infos);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    if(0 >= infosSize) {
        logger(LOG_ERROR, "rn4871GetMacAddress: string infos is empty ...\r\n");
        return CODE_RETURN_ERROR;
    }

    rn4871ParseDumpInfos(infos, FIELD_MAC_ADDRESS, macAddress);
    return ret;
}

uint8_t rn4871GetServices(struct rn4871_dev_s *dev, uint16_t *services) {
    assert((NULL != dev) || (NULL != services));

    char infos[BUFFER_UART_LEN_MAX+1] = "";
    uint8_t ret = rn4871DumpInfos(dev, infos);
    uint16_t infosSize = strlen(infos);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    if(0 >= infosSize) {
        logger(LOG_ERROR, "rn4871GetServices: string infos is empty ...\r\n");
        return CODE_RETURN_ERROR;
    }

    char tmp[BUFFER_UART_LEN_MAX+1] = "";
    rn4871ParseDumpInfos(infos, FIELD_SERVICES, tmp);
    *services = (uint16_t)strtol(tmp, NULL, 16);
    return ret;
}

uint8_t rn4871EraseAllGattServices(struct rn4871_dev_s *dev) {
    assert(NULL != dev);

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_CLEAR_ALL_SERVICES, NULL);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = rn4871ResponseProcess(dev, response);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    return ret;
}

uint8_t rn4871TransparentUartSendData(struct rn4871_dev_s *dev, const char *dataToSend, uint16_t dataToSendLen) {
    assert((NULL != dev) || (NULL != dataToSend));

    /* Must on data mode */
    if(DATA_MODE != _currentMode) {
        logger(LOG_ERROR, "rn4871TransparentUartSendData: module is not on DATA mode ...\r\n");
        return CODE_RETURN_NO_DATA_MODE;
    }

    /* Must on streaming state */
    if(FSM_STATE_STREAMING != rn4871GetFsmState()) {
        logger(LOG_ERROR, "rn4871TransparentUartSendData: fsm is not on STREAMING mode ...\r\n");
        return CODE_RETURN_NO_STREAMING;
    }

    logger(LOG_DEBUG, "rn4871TransparentUartSendData: [%d] \"%s\"\r\n", dataToSendLen, dataToSend);
    return dev->uartTx((char*)dataToSend, &dataToSendLen);
}

enum rn4871_fsm_e rn4871GetFsmState(void) {
    return _fsmState;
}

void rn4871SetForceFsmState(enum rn4871_fsm_e fsmForceState) {
    _fsmState = fsmForceState;
}

void rn4871SetForceDataMode(void) {
    _currentMode = DATA_MODE;
}

uint8_t rn4871IsOnTransparentUart(struct rn4871_dev_s *dev, bool *result) {
    assert((NULL != dev) || (NULL != result));
    uint8_t ret = CODE_RETURN_ERROR;

    *result = false;
    uint16_t services = 0x00;
    ret = rn4871GetServices(dev, &services);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    if(UART_TRANSPARENT & services)
        *result = true;

    return CODE_RETURN_SUCCESS;
}