#ifndef RN4871_API_H
#define RN4871_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DEVICE_NAME_LEN_MAX (20)
#define MAC_ADDRESS_LEN_MAX (20)
#define FW_VERSION_LEN_MAX (5)

typedef enum {
  RN4871_ERROR_NONE = 0,
  RN4871_ERROR_BAD_PARAMETER = 1,
  RN4871_ERROR_UART_FAIL = 2,
  RN4871_ERROR_CMD_UNKNOWN = 3,
  RN4871_ERROR_NO_MODE_COMMAND = 4,
  RN4871_ERROR_NO_MODE_DATA = 5,
  RN4871_ERROR_NO_TRANSPARENT_UART = 6,
  RN4871_ERROR_NO_CONNECTED = 7,
  RN4871_ERROR_NO_STREAMING = 8,
  RN4871_ERROR_UNKNOWN = 9,
} rn4871_error_e;

typedef enum {
  RN4871_CMD_MODE_ENTER = 0,
  RN4871_CMD_MODE_QUIT = 1,
  RN4871_CMD_REBOOT = 2,
  RN4871_CMD_RESET_FACTORY = 3,
  RN4871_CMD_SET_BT_NAME = 4,
  RN4871_CMD_SET_DEVICE_NAME = 5,
  RN4871_CMD_GET_DEVICE_NAME = 6,
  RN4871_CMD_SET_SERVICES = 7,
  RN4871_CMD_DUMP_INFOS = 8,
  RN4871_CMD_GET_VERSION = 9,
  RN4871_CMD_CLEAR_ALL_SERVICES = 10,
  RN4871_CMD_CREATE_PRIVATE_SERVICE = 11,
  RN4871_CMD_CREATE_PRIVATE_CHARACTERISTIC = 12,
  RN4871_CMD_SERVER_WRITE_CHARACTERISTIC = 13,
  RN4871_CMD_SERVER_READ_CHARACTERISTIC = 15,
  RN4871_CMD_NONE = 16,
} rn4871_cmd_e;

typedef enum {
  RN4871_MODE_DATA,
  RN4871_MODE_COMMAND,
} rn4871_mode_e;

typedef enum {
  RN4871_FSM_STATE_NONE,
  RN4871_FSM_STATE_INIT,
  RN4871_FSM_STATE_IDLE,
  RN4871_FSM_STATE_CONNECTED,
  RN4871_FSM_STATE_STREAMING,
  RN4871_FSM_STATE_HALT,
} rn4871_fsm_e;

typedef enum {
  RN4871_DEVICE_INFORMATION = 0x80,
  RN4871_UART_TRANSPARENT = 0x40,
  RN4871_BEACON = 0x20,
  RN4871_RESERVED = 0x10,
} rn4871_service_e;

typedef rn4871_error_e (*rn4871_com_fptr_t)(char *buffer, uint16_t *bufferLen);

typedef void (*rn4871_delay_fptr_t)(uint32_t delay);

typedef struct {
  rn4871_com_fptr_t uartTx;
  rn4871_com_fptr_t uartRx;
  rn4871_delay_fptr_t delayMs;
  rn4871_mode_e _currentMode;
  rn4871_cmd_e _currentCmd;
  rn4871_fsm_e _fsmState;
} rn4871_module_s;

rn4871_error_e rn4871Init(rn4871_module_s *dev);

rn4871_error_e rn4871WaitReceivedData(rn4871_module_s *dev, char *receivedData,
                                      uint16_t *receivedDataLen);

rn4871_error_e rn4871EnterCommandMode(rn4871_module_s *dev);
rn4871_error_e rn4871QuitCommandMode(rn4871_module_s *dev);

rn4871_error_e rn4871RebootModule(rn4871_module_s *dev);

rn4871_error_e rn4871IsOnTransparentUart(rn4871_module_s *dev, bool *result);

rn4871_error_e rn4871TransparentUartSendData(rn4871_module_s *dev,
                                             const char *dataToSend,
                                             uint16_t dataToSendLen);

rn4871_error_e rn4871DumpInfos(rn4871_module_s *dev, char *infos);

rn4871_error_e rn4871GetFirmwareVersion(rn4871_module_s *dev,
                                        char *firmwareVersion,
                                        uint16_t deviceNameMaxLen);

rn4871_error_e rn4871GetMacAddress(rn4871_module_s *dev, char *macAddress,
                                   uint16_t deviceNameMaxLen);

rn4871_error_e rn4871SetServices(rn4871_module_s *dev, uint16_t service);
rn4871_error_e rn4871GetServices(rn4871_module_s *dev, uint16_t *services);

rn4871_error_e rn4871SetDeviceName(rn4871_module_s *dev,
                                   const char *deviceName);
rn4871_error_e rn4871GetDeviceName(rn4871_module_s *dev, char *deviceName,
                                   uint16_t deviceNameMaxLen);

typedef struct {
  char deviceName[DEVICE_NAME_LEN_MAX + 1];
  char macAddress[MAC_ADDRESS_LEN_MAX + 1];
  char firmwareVersion[FW_VERSION_LEN_MAX + 1];
  uint16_t services;
} rn4871_config_s;

rn4871_error_e rn4871SetConfig(rn4871_module_s *dev, rn4871_config_s *config);
rn4871_error_e rn4871GetConfig(rn4871_module_s *dev, rn4871_config_s *config);

void rn4871SetForceFsmState(rn4871_module_s *dev, rn4871_fsm_e fsmForceState);
void rn4871SetForceDataMode(rn4871_module_s *dev);

rn4871_fsm_e rn4871GetFsmState(rn4871_module_s *dev);

char *rn4871GetErrorCodeStr(rn4871_error_e errorCode);

#ifdef __cplusplus
}
#endif

#endif /* RN4871_API_H */