#include "rn4871_core.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rn4871_defs.h"
#include "rn4871_logger.h"
#include "rn4871_types.h"

#define RN4871_DELAY_TO_RESPECT_MS 100
#define BASE_HEXADECIMAL 16

#define CR_LF_LEN_MAX 3

RN4871_STATIC_CONST char DUMP_INFOS_FIELD[][10] = {
    "BTA=", "Name=", "Connected=", "Authen=", "Features=", "Services=",
};

RN4871_STATIC_CONST char CR_LF[CR_LF_LEN_MAX] = "\r\n";

RN4871_STATIC rn4871_error_e rn4871SendCmd(rn4871_module_s *dev,
                                           rn4871_cmd_e cmd, const char *format,
                                           ...);
RN4871_STATIC rn4871_error_e rn4871ResponseProcess(rn4871_module_s *dev,
                                                   const char *response);
RN4871_STATIC rn4871_error_e rn4871ParseDumpInfos(const char *infos,
                                                  RN4871_DUMP_INFOS_FIELD field,
                                                  char *result,
                                                  uint16_t resultMaxLen);
RN4871_STATIC rn4871_error_e rn4871ParseFirmwareVersion(
    const char *firmwareVersion, char *result, uint16_t resultMaxLen);

rn4871_error_e rn4871Init(rn4871_module_s *dev) {
  assert(NULL != dev);

  if ((NULL == dev->delayMs) || (NULL == dev->uartTx) ||
      (NULL == dev->uartRx)) {
    return RN4871_ERROR_BAD_PARAMETER;
  }

  dev->_currentCmd = RN4871_CMD_NONE;
  dev->_currentMode = RN4871_MODE_DATA;
  dev->_fsmState = RN4871_FSM_STATE_NONE;

  logger(LOG_INFO, "rn4871 Init pass with success !\r\n");

  return RN4871_ERROR_NONE;
}

rn4871_error_e rn4871SendCmd(rn4871_module_s *dev, rn4871_cmd_e cmd,
                             const char *format, ...) {
  assert(NULL != dev);

  va_list args;
  if (NULL != format) {
    va_start(args, format);
  }

  char command[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  uint16_t commandLen = 0;
  rn4871_error_e ret = RN4871_ERROR_BAD_PARAMETER;

  if ((RN4871_MODE_COMMAND != dev->_currentMode) &&
      (RN4871_CMD_MODE_ENTER != cmd)) {
    logger(LOG_ERROR, "rn4871SendCmd: module is not on command mode ...\r\n");
    if (NULL != format) {
      va_end(args);
    }
    return RN4871_ERROR_NO_MODE_COMMAND;
  }

  switch (cmd) {
      /* Commands without argument */
    case RN4871_CMD_MODE_ENTER: {
      command[0] = '$';
      commandLen = 1;
      ret = dev->uartTx(command, &commandLen);
      dev->delayMs(RN4871_DELAY_TO_RESPECT_MS);
      ret = dev->uartTx(command, &commandLen);
      dev->delayMs(RN4871_DELAY_TO_RESPECT_MS);
      ret = dev->uartTx(command, &commandLen);
      break;
    }
    case RN4871_CMD_MODE_QUIT:
    case RN4871_CMD_DUMP_INFOS:
    case RN4871_CMD_GET_DEVICE_NAME:
    case RN4871_CMD_GET_VERSION:
    case RN4871_CMD_CLEAR_ALL_SERVICES: {
      commandLen = snprintf(command, RN4871_BUFFER_UART_LEN_MAX, "%s\r\n",
                            RN4871_COMMANDS[cmd]);
      ret = dev->uartTx(command, &commandLen);
      break;
    }
    /* Commands with arguments */
    case RN4871_CMD_REBOOT:
    case RN4871_CMD_RESET_FACTORY:
    case RN4871_CMD_SET_SERVICES:
    case RN4871_CMD_SET_DEVICE_NAME:
    case RN4871_CMD_SET_BT_NAME:
    case RN4871_CMD_CREATE_PRIVATE_SERVICE:
    case RN4871_CMD_SERVER_READ_CHARACTERISTIC:
    case RN4871_CMD_SERVER_WRITE_CHARACTERISTIC:
    case RN4871_CMD_CREATE_PRIVATE_CHARACTERISTIC: {
      char pArgs[RN4871_BUFFER_UART_LEN_MAX] = "";
      vsnprintf(pArgs, RN4871_BUFFER_UART_LEN_MAX, format, args);
      commandLen = snprintf(command, RN4871_BUFFER_UART_LEN_MAX, "%s,%s\r\n",
                            RN4871_COMMANDS[cmd], pArgs);
      ret = dev->uartTx(command, &commandLen);
      break;
    }
    /* Unknown command */
    default: {
      ret = RN4871_ERROR_UNKNOWN;
      logger(LOG_ERROR, "rn4871SendCmd: CMD_UNKNOWN\r\n");
      break;
    }
  }
  dev->_currentCmd = cmd;
  if (NULL != format) {
    va_end(args);
  }

  logger(LOG_DEBUG, "rn4871SendCmd: [%d] \"%s\"\r\n", commandLen, command);

  return ret;
}

rn4871_error_e rn4871ResponseProcess(rn4871_module_s *dev,
                                     const char *response) {
  assert((NULL != dev) || (NULL != response));

  rn4871_error_e ret = RN4871_ERROR_BAD_PARAMETER;
  rn4871_cmd_e cmd = dev->_currentCmd;

  logger(LOG_DEBUG, "rn4871ResponseProcess: [%d] \"%s\"\r\n", strlen(response),
         response);

  if ((NULL != strstr(response, "AOK")) || (NULL != strstr(response, "CMD>")) ||
      (NULL != strstr(response, "END")) ||
      (NULL != strstr(response, "Rebooting"))) {
    dev->_fsmState = RN4871_FSM_STATE_INIT;

    /* Check if error is returned */
    if (NULL != strstr(response, "Err")) {
      ret = RN4871_ERROR_BAD_PARAMETER;
    }
    /* Parse and get data from response */
    else {
      switch (cmd) {
        case RN4871_CMD_MODE_ENTER: {
          ret = RN4871_ERROR_NONE;
          dev->_currentMode = RN4871_MODE_COMMAND;
          break;
        }
        case RN4871_CMD_MODE_QUIT: {
          ret = RN4871_ERROR_NONE;
          dev->_currentMode = RN4871_MODE_DATA;
          break;
        }
        case RN4871_CMD_SET_BT_NAME:
        case RN4871_CMD_SET_DEVICE_NAME:
        case RN4871_CMD_SET_SERVICES:
        case RN4871_CMD_CLEAR_ALL_SERVICES:
        case RN4871_CMD_CREATE_PRIVATE_SERVICE:
        case RN4871_CMD_CREATE_PRIVATE_CHARACTERISTIC:
        case RN4871_CMD_SERVER_WRITE_CHARACTERISTIC:
        case RN4871_CMD_DUMP_INFOS:
        case RN4871_CMD_GET_DEVICE_NAME:
        case RN4871_CMD_GET_VERSION:
        case RN4871_CMD_SERVER_READ_CHARACTERISTIC: {
          ret = RN4871_ERROR_NONE;
          break;
        }
        case RN4871_CMD_REBOOT:
        case RN4871_CMD_RESET_FACTORY: {
          dev->_currentMode = RN4871_MODE_DATA;
          dev->_fsmState = RN4871_FSM_STATE_IDLE;
          ret = RN4871_ERROR_NONE;
          break;
        }
        default:
          ret = RN4871_ERROR_UNKNOWN;
          break;
      }
    }
  }
  return ret;
}

rn4871_error_e rn4871WaitReceivedData(rn4871_module_s *dev, char *receivedData,
                                      uint16_t *receivedDataLen) {
  assert(NULL != dev);

  if (RN4871_MODE_DATA != dev->_currentMode) {
    return RN4871_ERROR_NO_MODE_DATA;
  }

  dev->uartRx(receivedData, receivedDataLen);

  logger(LOG_DEBUG, "rn4871ReceivedDataProcess: [%d] \"%s\"\r\n",
         *receivedDataLen, receivedData);

  if (NULL != strstr(receivedData, "Rebooting")) {
    dev->_fsmState = RN4871_FSM_STATE_IDLE;
  } else if (NULL != strstr(receivedData, "DISCONNECT")) {
    dev->_fsmState = RN4871_FSM_STATE_IDLE;
  } else if (NULL != strstr(receivedData, "CONNECT")) {
    dev->_fsmState = RN4871_FSM_STATE_CONNECTED;
  } else if (NULL != strstr(receivedData, "STREAM_OPEN")) {
    dev->_fsmState = RN4871_FSM_STATE_STREAMING;
  }

  return RN4871_ERROR_NONE;
}

rn4871_error_e rn4871EnterCommandMode(rn4871_module_s *dev) {
  assert(NULL != dev);

  rn4871_error_e ret = RN4871_ERROR_BAD_PARAMETER;
  char response[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  uint16_t responseSize = 0;

  ret = rn4871SendCmd(dev, RN4871_CMD_MODE_ENTER, NULL);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = dev->uartRx(response, &responseSize);
  if (RN4871_ERROR_NONE == ret) {
    ret = rn4871ResponseProcess(dev, response);
  } else {
    /* Module may be already on command mode */
    logger(LOG_DEBUG, "\"$$$\" without reponse => Send CR+LF\r\n");
    uint16_t stringSize = CR_LF_LEN_MAX - 1;
    ret = dev->uartTx((char *)CR_LF, &stringSize);
    if (RN4871_ERROR_NONE != ret) {
      return ret;
    }
    ret = dev->uartRx(response, &responseSize);
    dev->_fsmState = RN4871_FSM_STATE_INIT;
    dev->_currentMode = RN4871_MODE_COMMAND;
  }
  return ret;
}

rn4871_error_e rn4871QuitCommandMode(rn4871_module_s *dev) {
  assert(NULL != dev);

  rn4871_error_e ret = RN4871_ERROR_BAD_PARAMETER;
  char response[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  uint16_t responseSize = 0;

  ret = rn4871SendCmd(dev, RN4871_CMD_MODE_QUIT, NULL);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = dev->uartRx(response, &responseSize);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = rn4871ResponseProcess(dev, response);
  return ret;
}

rn4871_error_e rn4871RebootModule(rn4871_module_s *dev) {
  assert(NULL != dev);

  rn4871_error_e ret = RN4871_ERROR_BAD_PARAMETER;
  char response[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  uint16_t responseSize = 0;

  ret = rn4871SendCmd(dev, RN4871_CMD_REBOOT, "1");
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = dev->uartRx(response, &responseSize);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = rn4871ResponseProcess(dev, response);
  return ret;
}

rn4871_error_e rn4871SetServices(rn4871_module_s *dev, uint16_t service) {
  assert(NULL != dev);

  rn4871_error_e ret = RN4871_ERROR_BAD_PARAMETER;
  char response[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  uint16_t responseSize = 0;

  ret = rn4871SendCmd(dev, RN4871_CMD_SET_SERVICES, "%X", service);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = dev->uartRx(response, &responseSize);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = rn4871ResponseProcess(dev, response);
  return ret;
}

rn4871_error_e rn4871SetDeviceName(rn4871_module_s *dev,
                                   const char *deviceName) {
  assert((NULL != dev) || (NULL != deviceName));

  rn4871_error_e ret = RN4871_ERROR_BAD_PARAMETER;
  char response[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  uint16_t responseSize = 0;

  uint16_t deviceNameLen = (uint16_t)strlen(deviceName);
  if ((0 >= deviceNameLen) || (DEVICE_NAME_LEN_MAX < deviceNameLen)) {
    logger(LOG_ERROR,
           "rn4871SetDeviceName: deviceNameLen is incorrect ... [%d]\r\n",
           deviceNameLen);
    return RN4871_ERROR_BAD_PARAMETER;
  }

  ret = rn4871SendCmd(dev, RN4871_CMD_SET_DEVICE_NAME, deviceName);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = dev->uartRx(response, &responseSize);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = rn4871ResponseProcess(dev, response);
  return ret;
}

rn4871_error_e rn4871SetConfig(rn4871_module_s *dev, rn4871_config_s *config) {
  assert((NULL != dev) || (NULL != config));

  rn4871_error_e ret = RN4871_ERROR_BAD_PARAMETER;
  ret = rn4871SetDeviceName(dev, config->deviceName);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }

  ret = rn4871SetServices(dev, config->services);
  return ret;
}

rn4871_error_e rn4871GetDeviceName(rn4871_module_s *dev, char *deviceName,
                                   uint16_t deviceNameMaxLen) {
  assert((NULL != dev) || (NULL != deviceName));

  char infos[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  rn4871_error_e ret = rn4871DumpInfos(dev, infos);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = rn4871ParseDumpInfos(infos, FIELD_DEVICE_NAME, deviceName,
                             deviceNameMaxLen);
  return ret;
}

rn4871_error_e rn4871ParseFirmwareVersion(const char *firmwareVersion,
                                          char *result, uint16_t resultMaxLen) {
  assert((NULL != firmwareVersion) || (NULL != result));

  rn4871_error_e ret = RN4871_ERROR_BAD_PARAMETER;
  char delimiter[] = " \r\n";
  char *token = strtok((char *)firmwareVersion, delimiter);
  do {
    if (NULL != strstr(token, "V")) {
      ret = RN4871_ERROR_NONE;
      break;
    }
    token = strtok(NULL, delimiter);
  } while (NULL != token);

  if (RN4871_ERROR_NONE == ret) {
    strncpy(result, token, resultMaxLen);
  }
  return ret;
}

rn4871_error_e rn4871GetFirmwareVersion(rn4871_module_s *dev,
                                        char *firmwareVersion,
                                        uint16_t firmwareVersionMaxLen) {
  assert((NULL != dev) || (NULL != firmwareVersion));

  rn4871_error_e ret = RN4871_ERROR_BAD_PARAMETER;
  char response[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  uint16_t responseSize = 0;

  ret = rn4871SendCmd(dev, RN4871_CMD_GET_VERSION, NULL);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = dev->uartRx(response, &responseSize);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = rn4871ResponseProcess(dev, response);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = rn4871ParseFirmwareVersion(response, firmwareVersion,
                                   firmwareVersionMaxLen);
  return ret;
}

rn4871_error_e rn4871DumpInfos(rn4871_module_s *dev, char *infos) {
  assert((NULL != dev) || (NULL != infos));

  rn4871_error_e ret = RN4871_ERROR_BAD_PARAMETER;
  char response[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  uint16_t responseSize = 0;

  ret = rn4871SendCmd(dev, RN4871_CMD_DUMP_INFOS, NULL);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = dev->uartRx(response, &responseSize);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = rn4871ResponseProcess(dev, response);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }

  strncpy(infos, response, RN4871_BUFFER_UART_LEN_MAX);
  return ret;
}

rn4871_error_e rn4871ParseDumpInfos(const char *infos,
                                    RN4871_DUMP_INFOS_FIELD field, char *result,
                                    uint16_t resultMaxLen) {
  assert((NULL != infos) || (NULL != result));

  char delimiter[] = "\r\n";
  char *token = strtok((char *)infos, delimiter);
  do {
    if (NULL != strstr(token, DUMP_INFOS_FIELD[field])) {
      break;
    }
    token = strtok(NULL, delimiter);
  } while (NULL != token);

  if (NULL == token) {
    logger(LOG_ERROR, "rn4871ParseDumpInfos: string infos is empty ...\r\n");
    return RN4871_ERROR_BAD_PARAMETER;
  }
  char *tmp;
  tmp = strtok(token, "=");
  tmp = strtok(NULL, "=");
  if (NULL == tmp) {
    return RN4871_ERROR_BAD_PARAMETER;
  }
  strncpy(result, tmp, resultMaxLen);
  return RN4871_ERROR_NONE;
}

rn4871_error_e rn4871GetMacAddress(rn4871_module_s *dev, char *macAddress,
                                   uint16_t macAddressMaxLen) {
  assert((NULL != dev) || (NULL != macAddress));

  char infos[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  rn4871_error_e ret = rn4871DumpInfos(dev, infos);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = rn4871ParseDumpInfos(infos, FIELD_MAC_ADDRESS, macAddress,
                             macAddressMaxLen);
  return ret;
}

rn4871_error_e rn4871GetServices(rn4871_module_s *dev, uint16_t *services) {
  assert((NULL != dev) || (NULL != services));

  char infos[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  rn4871_error_e ret = rn4871DumpInfos(dev, infos);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  char tmp[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  ret = rn4871ParseDumpInfos(infos, FIELD_SERVICES, tmp,
                             RN4871_BUFFER_UART_LEN_MAX);
  *services = (uint16_t)strtol(tmp, NULL, BASE_HEXADECIMAL);
  return ret;
}

rn4871_error_e rn4871GetConfig(rn4871_module_s *dev, rn4871_config_s *config) {
  assert((NULL != dev) || (NULL != config));

  rn4871_error_e ret = RN4871_ERROR_BAD_PARAMETER;
  char infos[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  ret = rn4871DumpInfos(dev, infos);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  char save[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  strncpy(save, infos, RN4871_BUFFER_UART_LEN_MAX);

  char tmp[RN4871_BUFFER_UART_LEN_MAX + 1] = "";
  ret = rn4871ParseDumpInfos(save, FIELD_SERVICES, tmp,
                             RN4871_BUFFER_UART_LEN_MAX);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  strncpy(save, infos, RN4871_BUFFER_UART_LEN_MAX);
  config->services = (uint16_t)strtol(tmp, NULL, BASE_HEXADECIMAL);
  ret = rn4871ParseDumpInfos(save, FIELD_MAC_ADDRESS, config->macAddress,
                             MAC_ADDRESS_LEN_MAX);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  strncpy(save, infos, RN4871_BUFFER_UART_LEN_MAX);
  ret = rn4871ParseDumpInfos(save, FIELD_DEVICE_NAME, config->deviceName,
                             DEVICE_NAME_LEN_MAX);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }
  ret = rn4871GetFirmwareVersion(dev, config->firmwareVersion,
                                 FW_VERSION_LEN_MAX);
  return ret;
}

rn4871_error_e rn4871TransparentUartSendData(rn4871_module_s *dev,
                                             const char *dataToSend,
                                             uint16_t dataToSendLen) {
  assert((NULL != dev) || (NULL != dataToSend));

  /* Must on data mode */
  if (RN4871_MODE_DATA != dev->_currentMode) {
    logger(LOG_ERROR,
           "rn4871TransparentUartSendData: module is not on DATA mode ...\r\n");
    return RN4871_ERROR_NO_MODE_DATA;
  }

  /* Must on streaming state */
  if (RN4871_FSM_STATE_STREAMING != rn4871GetFsmState(dev)) {
    logger(
        LOG_ERROR,
        "rn4871TransparentUartSendData: fsm is not on STREAMING mode ...\r\n");
    return RN4871_ERROR_NO_STREAMING;
  }

  logger(LOG_DEBUG, "rn4871TransparentUartSendData: [%d] \"%s\"\r\n",
         dataToSendLen, dataToSend);
  return dev->uartTx((char *)dataToSend, &dataToSendLen);
}

rn4871_fsm_e rn4871GetFsmState(rn4871_module_s *dev) {
  assert(NULL != dev);
  return dev->_fsmState;
}

void rn4871SetForceFsmState(rn4871_module_s *dev, rn4871_fsm_e fsmForceState) {
  assert(NULL != dev);

  switch (fsmForceState) {
    case RN4871_FSM_STATE_INIT:
    case RN4871_FSM_STATE_IDLE:
    case RN4871_FSM_STATE_CONNECTED:
    case RN4871_FSM_STATE_STREAMING:
    case RN4871_FSM_STATE_HALT:
      dev->_fsmState = fsmForceState;
      break;

    default:
      dev->_fsmState = RN4871_FSM_STATE_NONE;
      break;
  }
}

void rn4871SetForceDataMode(rn4871_module_s *dev) {
  assert(NULL != dev);
  dev->_currentMode = RN4871_MODE_DATA;
}

rn4871_error_e rn4871IsOnTransparentUart(rn4871_module_s *dev, bool *result) {
  assert((NULL != dev) || (NULL != result));
  rn4871_error_e ret = RN4871_ERROR_BAD_PARAMETER;

  *result = false;
  uint16_t services = 0x00;
  ret = rn4871GetServices(dev, &services);
  if (RN4871_ERROR_NONE != ret) {
    return ret;
  }

  if (RN4871_UART_TRANSPARENT & services) {
    *result = true;
  }

  return RN4871_ERROR_NONE;
}

char *rn4871GetErrorCodeStr(rn4871_error_e errorCode) {
  int id = RN4871_ERROR_UNKNOWN;
  switch (errorCode) {
    case RN4871_ERROR_NONE:
    case RN4871_ERROR_BAD_PARAMETER:
    case RN4871_ERROR_UART_FAIL:
    case RN4871_ERROR_CMD_UNKNOWN:
    case RN4871_ERROR_NO_MODE_COMMAND:
    case RN4871_ERROR_NO_MODE_DATA:
    case RN4871_ERROR_NO_TRANSPARENT_UART:
    case RN4871_ERROR_NO_CONNECTED:
    case RN4871_ERROR_NO_STREAMING:
      id = errorCode;
      break;
    default:
      id = RN4871_ERROR_UNKNOWN;
      break;
  }
  return ((char *)RN4871_ERRORS[id]);
}