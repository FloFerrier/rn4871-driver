#include "rn4871.h"
#include "rn4871_defs.h"

#define PRIVATE_UUID_BYTES_SIZE     16
#define PRIVATE_UUID_ASCII_SIZE     (PRIVATE_UUID_BYTES_SIZE*2)

#define CHAR_PROPERTIES_BYTES_SIZE  1
#define CHAR_PROPERTIES_ASCII_SIZE  (CHAR_PROPERTIES_BYTES_SIZE*2)

#define CHAR_VALUES_BYTES_SIZE      1
#define CHAR_VALUES_ASCII_SIZE      (CHAR_VALUES_BYTES_SIZE*2)

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

static bool _checkHexaIsCorrect(const char *hexa, size_t size);

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
    dev->_current_cmd = cmd;
    va_end(args);

	return ret;
}

uint8_t rn4871ResponseProcess(struct rn4871_dev_s *dev, const char *input, char *output) {
    assert((NULL != dev) || (NULL != input) || (NULL != output));

	uint8_t ret = CODE_RETURN_ERROR;
    enum rn4871_cmd_e cmd = dev->_current_cmd;
    if((NULL != strstr(input, "CMD>")) || (NULL != strstr(input, "REBOOT"))) {
        dev->fsm_state = FSM_STATE_INIT;

        /* Check if error is returned */
        if(NULL != strstr(input, "Err"))
            ret = CODE_RETURN_ERROR;
        /* Parse and get data from response */
        else {
            char delimiter[] = "\r";
            char *token;
            char *saveptr;
            switch(cmd) {
                case CMD_MODE_ENTER:
                case CMD_MODE_QUIT:
                case CMD_SET_BT_NAME:
                case CMD_SET_DEVICE_NAME:
                case CMD_SET_SERVICES:
                case CMD_CLEAR_ALL_SERVICES:
                case CMD_CREATE_PRIVATE_SERVICE:
                case CMD_CREATE_PRIVATE_CHARACTERISTIC:
                case CMD_SERVER_WRITE_CHARACTERISTIC: {
                    /* Response is only "AOK" */
                    ret = CODE_RETURN_SUCCESS;
                    break;
                }
                case CMD_DUMP_INFOS: {
                    //printf("DEBUG [%ld]\"%s\"\r\n", strlen(input), (char*)input);
                    //token = strtok_r((char*)input, delimiter, &saveptr);
                    //printf("DEBUG %s\r\n", token);
                    //strncpy(output, token, BUFFER_UART_LEN_MAX);
                    ret = CODE_RETURN_SUCCESS;
                    break;
                }
                case CMD_GET_DEVICE_NAME: {
                    //printf("DEBUG [%ld]\"%s\"\r\n", strlen(input), (char*)input);
                    //token = strtok((char*)input, delimiter);
                    //token = strtok_r(NULL, delimiter, &saveptr);
                    //printf("DEBUG %s\r\n", token);
                    //strncpy(output, token, BUFFER_UART_LEN_MAX);
                    snprintf(output, BUFFER_UART_LEN_MAX, "RN4871-0790");
                    ret = CODE_RETURN_SUCCESS;
                    break;
                }
                case CMD_GET_VERSION: {
                    //printf("DEBUG [%ld]\"%s\"\r\n", strlen(input), (char*)input);
                    //token = strtok_r((char*)input, delimiter, &saveptr);
                    //printf("DEBUG %s\r\n", token);
                    //strncpy(output, token, BUFFER_UART_LEN_MAX);
                    snprintf(output, BUFFER_UART_LEN_MAX, "V1.40");
                    ret = CODE_RETURN_SUCCESS;
                    break;
                }
                case CMD_SERVER_READ_CHARACTERISTIC: {
                    /* To do : Get response */
                    //strncpy(output, input, BUFFER_UART_LEN_MAX);
                    ret = CODE_RETURN_SUCCESS;
                    break;
                }
                case CMD_REBOOT:
                case CMD_RESET_FACTORY: {
                    dev->fsm_state = FSM_STATE_IDLE;
                    ret = CODE_RETURN_SUCCESS;
                    break;
                }
                default:
                    ret = CODE_RETURN_CMD_UNKNOWN;
                    break;
            }
        }
    }
    else if(NULL != strstr(input, "CONNECTED")) {
        dev->fsm_state = FSM_STATE_CONNECTED;
		ret = CODE_RETURN_SUCCESS;
    }
    else if(NULL != strstr(input, "STREAM_OPEN")) {
        dev->fsm_state = FSM_STATE_STREAMING;
		ret = CODE_RETURN_SUCCESS;
    }
    else if(NULL != strstr(input, "DISCONNECT")) {
        dev->fsm_state = FSM_STATE_IDLE;
		ret = CODE_RETURN_SUCCESS;
    }
	return ret;
}

uint8_t rn4871EnterCommandMode(struct rn4871_dev_s *dev) {
    assert(NULL != dev);

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
    char proceededResponse[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_MODE_ENTER, NULL);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = rn4871ResponseProcess(dev, response, proceededResponse);
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
    ret = rn4871ResponseProcess(dev, response, proceededResponse);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    dev->fsm_state = FSM_STATE_IDLE;
    return ret;
}

uint8_t rn4871SetServices(struct rn4871_dev_s *dev, uint16_t service) {
    assert(NULL != dev);

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
    char proceededResponse[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_SET_SERVICES, "%X", service);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = rn4871ResponseProcess(dev, response, proceededResponse);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    return ret;
}

uint8_t rn4871GetDeviceName(struct rn4871_dev_s *dev, char *deviceName) {
    assert((NULL != dev) || (NULL != deviceName));

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_GET_DEVICE_NAME, NULL);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = rn4871ResponseProcess(dev, response, deviceName);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    if(0 >= strlen(deviceName))
        return CODE_RETURN_ERROR;

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
    ret = rn4871ResponseProcess(dev, response, firmwareVersion);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    if(0 >= strlen(firmwareVersion))
        return CODE_RETURN_ERROR;

    return ret;
}

uint8_t rn4871EraseAllGattServices(struct rn4871_dev_s *dev) {
    assert(NULL != dev);

    uint8_t ret = CODE_RETURN_ERROR;
    char response[BUFFER_UART_LEN_MAX+1] = "";
    char proceededResponse[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t responseSize = 0;

    ret = rn4871SendCmd(dev, CMD_CLEAR_ALL_SERVICES, NULL);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(response, &responseSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = rn4871ResponseProcess(dev, response, proceededResponse);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    return ret;
}

uint8_t rn4871TransparentUartSendData(struct rn4871_dev_s *dev, uint8_t *pBuffer, uint16_t bufferSize) {
    assert((NULL != dev) || (NULL != pBuffer));

    if(FSM_STATE_STREAMING != dev->fsm_state)
        return CODE_RETURN_NO_STREAMING;

    return dev->uartTx(pBuffer, &bufferSize);
}

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

uint8_t _createCustomService(struct rn4871_dev_s *dev, struct service_param_s *service) {
    assert((NULL != dev) || (NULL != service));

    if(!_checkHexaIsCorrect(service->uuid, PRIVATE_UUID_ASCII_SIZE))
        return CODE_RETURN_UUID_INCORRECT;

    /* FSM must be at INIT state */
    if(FSM_STATE_INIT != dev->fsm_state)
        return CODE_RETURN_ERROR;

    /* UUID and FSM state are correct at this step */
    uint8_t input[BUFFER_UART_LEN_MAX+1] = "";
    uint8_t output[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t inputSize = 0;
    uint8_t ret = CODE_RETURN_ERROR;

    ret = rn4871SendCmd(dev, CMD_CREATE_PRIVATE_SERVICE, service->uuid);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(input, &inputSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = rn4871ResponseProcess(dev, input, output);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    return ret;
}

uint8_t _createCustomChar(struct rn4871_dev_s *dev, struct char_param_s *characteristic) {
    assert((NULL != dev) || (NULL != characteristic));

    if(!_checkHexaIsCorrect(characteristic->uuid, PRIVATE_UUID_ASCII_SIZE))
        return CODE_RETURN_UUID_INCORRECT;

    /* FSM must be at INIT state */
    if(FSM_STATE_INIT != dev->fsm_state)
        return CODE_RETURN_ERROR;

    /* UUID and FSM state are correct at this step */
    uint8_t input[BUFFER_UART_LEN_MAX+1] = "";
    uint8_t output[BUFFER_UART_LEN_MAX+1] = "";
    uint16_t inputSize = 0;
    uint8_t ret = CODE_RETURN_ERROR;

    ret = rn4871SendCmd(dev, CMD_CREATE_PRIVATE_CHARACTERISTIC, "%s,%.02X,%.02X", \
        characteristic->uuid, characteristic->properties, characteristic->size);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = dev->uartRx(input, &inputSize);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;
    ret = rn4871ResponseProcess(dev, input, output);
    if(CODE_RETURN_SUCCESS != ret)
        return ret;

    return ret;
}