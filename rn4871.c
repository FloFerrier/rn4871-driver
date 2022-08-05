#include "rn4871.h"
#include "rn4871_defs.h"

static const char TABLE_COMMAND[][10] = {
    "",
    "$$$",
    "---",
    "R,1",
    "SF",
    "S-",
    "SN",
    "GN",
    "SS",
    "D",
    "V",
    "PZ",
    "PS",
    "PC",
    "SHW",
    "SHR",
};

static enum rn4871_cmd_e _current_cmd = CMD_NONE;
static enum rn4871_fsm_e _fsm_state = FSM_STATE_NONE;

static bool _checkHexaIsCorrect(const char *hexa, size_t size);

static uint8_t rn4871SendCmd(struct rn4871_dev_s *dev, enum rn4871_cmd_e cmd, const char *format, ...);
static uint8_t rn4871ResponseProcess(struct rn4871_dev_s *dev, const char *input);

bool _checkHexaIsCorrect(const char *hexa, size_t size) {
    assert(NULL != hexa);

    if(size != strlen(hexa))
        return false;

    /* Must contained only [A-F] or [0-9] characters */
    for(int idx=0; idx < size; idx++) {
        if(0 == isxdigit(hexa[idx]))
            return false;
    }
    return true;
}

uint8_t rn4871SendCmd(struct rn4871_dev_s *dev, enum rn4871_cmd_e cmd, const char *format, ...) {
    assert(NULL != dev);

    va_list args;
    if(NULL != format)
        va_start(args, format);

	uint8_t pBuffer[BUFFER_UART_LEN_MAX+1] = "";
	uint16_t bufferSize = 0;
	uint8_t ret = CODE_RETURN_ERROR;

    switch(cmd) {
        /* Commands without argument */
		case CMD_MODE_ENTER: {
            pBuffer[0] = '$';
            bufferSize = 1;
            ret = dev->uartTx(pBuffer, &bufferSize);
            dev->delayMs(100);
            ret = dev->uartTx(pBuffer, &bufferSize);
            dev->delayMs(100);
            ret = dev->uartTx(pBuffer, &bufferSize);
            break;
        }
        case CMD_MODE_QUIT:
        case CMD_REBOOT:
        case CMD_DUMP_INFOS:
        case CMD_GET_DEVICE_NAME:
        case CMD_GET_VERSION:
        case CMD_CLEAR_ALL_SERVICES: {
            bufferSize = snprintf(pBuffer, BUFFER_UART_LEN_MAX, "%s\r\n", TABLE_COMMAND[cmd]);
            ret = dev->uartTx(pBuffer, &bufferSize);
            break;
        }
        /* Commands with arguments */
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
			bufferSize = snprintf(pBuffer, BUFFER_UART_LEN_MAX, "%s,%s\r\n", TABLE_COMMAND[cmd], pArgs);
            ret = dev->uartTx(pBuffer, &bufferSize);
			break;
        }
        /* Unknown command */
        default:
			ret = CODE_RETURN_CMD_UNKNOWN;
        	break;
    }
    _current_cmd = cmd;
    va_end(args);

	return ret;
}

uint8_t rn4871ResponseProcess(struct rn4871_dev_s *dev, const char *input) {
    assert((NULL != dev) || (NULL != input));

	uint8_t ret = CODE_RETURN_ERROR;
    enum rn4871_cmd_e cmd = _current_cmd;

    if((NULL != strstr(input, "AOK")) || (NULL != strstr(input, "CMD>")) || (NULL != strstr(input, "REBOOT"))) {
        _fsm_state = FSM_STATE_INIT;

        /* Check if error is returned */
        if(NULL != strstr(input, "Err"))
            ret = CODE_RETURN_ERROR;
        /* Parse and get data from response */
        else {
            switch(cmd) {
                case CMD_MODE_ENTER:
                case CMD_MODE_QUIT:
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
                    _fsm_state = FSM_STATE_IDLE;
                    ret = CODE_RETURN_SUCCESS;
                    break;
                }
                default:
                    ret = CODE_RETURN_CMD_UNKNOWN;
                    break;
            }
        }
    }
    else if(NULL != strstr(input, "STREAM_OPEN")) {
        _fsm_state = FSM_STATE_STREAMING;
		ret = CODE_RETURN_SUCCESS;
    }
    else if(NULL != strstr(input, "DISCONNECT")) {
        _fsm_state = FSM_STATE_IDLE;
		ret = CODE_RETURN_SUCCESS;
    }
    else if(NULL != strstr(input, "CONNECT")) {
        _fsm_state = FSM_STATE_CONNECTED;
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

uint8_t rn4871RebootModule(struct rn4871_dev_s *dev) {
    assert(NULL != dev);

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
    char proceededResponse[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_REBOOT, NULL);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = rn4871ResponseProcess(dev, response);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    _fsm_state = FSM_STATE_IDLE;
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

uint8_t rn4871SetDeviceName(struct rn4871_dev_s *dev, const char *deviceName, size_t sizeDeviceName) {
    assert((NULL != dev) || (NULL != deviceName) || (DEVICE_NAME_CHARACTER_MAX >= sizeDeviceName));

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

    if(0 >= strlen(deviceName))
        return CODE_RETURN_ERROR;

    return ret;
}

uint8_t rn4871GetDeviceName(struct rn4871_dev_s *dev, char *deviceName) {
    assert((NULL != dev) || (NULL != deviceName));

    char infos[BUFFER_UART_LEN_MAX+1] = "";
    uint8_t ret = rn4871DumpInfos(dev, infos);
    uint16_t infosSize = strlen(infos);
    if((CODE_RETURN_SUCCESS != ret) || (0 >= infosSize))
        return CODE_RETURN_ERROR;

    /* Parse infos string to get Mac Address */
    char *saveptr;
    char delimiter[] = "\r\n";
    char *token = strtok_r(infos, delimiter, &saveptr);
    do {
        if(NULL != strstr(token, "Name=")) {
            break;
        }
        token = strtok_r(NULL, delimiter, &saveptr);
    } while(NULL != token);

    if(NULL == token)
        return CODE_RETURN_ERROR;

    char *tmp;
    tmp = strtok_r(token, "=", &saveptr);
    tmp = strtok_r(NULL, "=", &saveptr);
    strncpy(deviceName, tmp, BUFFER_UART_LEN_MAX);
    return ret;
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

    /* Parse version string to get version number only*/
    char *saveptr;
    char delimiter[] = " \r\n";
    char *token = strtok_r(response, delimiter, &saveptr);
    do {
        if(NULL != strstr(token, "V")) {
            break;
        }
        token = strtok_r(NULL, delimiter, &saveptr);
    } while(NULL != token);

    if(NULL == token)
        return CODE_RETURN_ERROR;

    strncpy(firmwareVersion, token, BUFFER_UART_LEN_MAX);
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
    if((CODE_RETURN_SUCCESS != ret) || (0 >= responseSize))
        return CODE_RETURN_ERROR;
    ret = rn4871ResponseProcess(dev, response);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    strncpy(infos, response, BUFFER_UART_LEN_MAX);
    return ret;
}

uint8_t rn4871GetMacAddress(struct rn4871_dev_s *dev, char *macAddress) {
    assert((NULL != dev) || (NULL != macAddress));

    char infos[BUFFER_UART_LEN_MAX+1] = "";
    uint8_t ret = rn4871DumpInfos(dev, infos);
    uint16_t infosSize = strlen(infos);
    if((CODE_RETURN_SUCCESS != ret) || (0 >= infosSize))
        return CODE_RETURN_ERROR;

    /* Parse infos string to get Mac Address */
    char *saveptr;
    char delimiter[] = "\r\n";
    char *token = strtok_r(infos, delimiter, &saveptr);
    do {
        if(NULL != strstr(token, "BTA=")) {
            break;
        }
        token = strtok_r(NULL, delimiter, &saveptr);
    } while(NULL != token);

    if(NULL == token)
        return CODE_RETURN_ERROR;

    char *tmp;
    tmp = strtok_r(token, "=", &saveptr);
    tmp = strtok_r(NULL, "=", &saveptr);
    strncpy(macAddress, tmp, BUFFER_UART_LEN_MAX);
    return ret;
}

uint8_t rn4871GetServices(struct rn4871_dev_s *dev, uint16_t *services) {
    assert((NULL != dev) || (NULL != services));

    char infos[BUFFER_UART_LEN_MAX+1] = "";
    uint8_t ret = rn4871DumpInfos(dev, infos);
    uint16_t infosSize = strlen(infos);
    if((CODE_RETURN_SUCCESS != ret) || (0 >= infosSize))
        return CODE_RETURN_ERROR;

    /* Parse infos string to get Services */
    char *saveptr;
    char delimiter[] = "\r\n";
    char *token = strtok_r(infos, delimiter, &saveptr);
    do {
        if(NULL != strstr(token, "Services=")) {
            break;
        }
        token = strtok_r(NULL, delimiter, &saveptr);
    } while(NULL != token);

    if(NULL == token)
        return CODE_RETURN_ERROR;

    char *tmp;
    tmp = strtok_r(token, "=", &saveptr);
    tmp = strtok_r(NULL, "=", &saveptr);
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

uint8_t rn4871TransparentUartSendData(struct rn4871_dev_s *dev, uint8_t *pBuffer, uint16_t bufferSize) {
    assert((NULL != dev) || (NULL != pBuffer));

    if(FSM_STATE_STREAMING != _fsm_state)
        return CODE_RETURN_NO_STREAMING;

    return dev->uartTx(pBuffer, &bufferSize);
}

enum rn4871_fsm_e rn4871GetFsmState(void) {
    return _fsm_state;
}

void rn4871SetForceFsmState(enum rn4871_fsm_e fsmForceState) {
    _fsm_state = fsmForceState;
}