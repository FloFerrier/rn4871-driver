#ifndef RN4871_DEFS_H
#define RN4871_DEFS_H

#include <stdint.h>

#define RN4871_BUFFER_UART_LEN_MAX (255)

#define DEVICE_NAME_LEN_MAX (20)
#define MAC_ADDRESS_LEN_MAX (20)
#define FW_VERSION_LEN_MAX (5)
#define ERROR_CODE_LEN_MAX (20)
#define COMMAND_LEN_MAX (10)

enum rn4871_code_return_e
{
	CODE_RETURN_SUCCESS = 0,
	CODE_RETURN_ERROR = 1,
	CODE_RETURN_UART_FAIL = 2,
	CODE_RETURN_CMD_UNKNOWN = 3,
	CODE_RETURN_NO_COMMAND_MODE = 4,
	CODE_RETURN_NO_DATA_MODE = 5,
	CODE_RETURN_NO_TRANSPARENT_UART = 6,
	CODE_RETURN_NO_CONNECTED = 7,
	CODE_RETURN_NO_STREAMING = 8,
};

static const char ERROR_CODE[][ERROR_CODE_LEN_MAX] =
{
	"SUCCESS",
	"ERROR",
	"UART_FAIL",
	"COMMAND_UNKNOWN",
	"NO_COMMAND_MODE",
	"NO_DATA_MODE",
	"NO_TRANSPARENT_UART",
	"NO_CONNECTED",
	"NO_STREAMING",
};

enum rn4871_cmd_e
{
	CMD_MODE_ENTER = 0, /* $$$ */
	CMD_MODE_QUIT = 1, /* --- */
	CMD_REBOOT = 2, /* R */
	CMD_RESET_FACTORY = 3, /* SF */
	CMD_SET_BT_NAME = 4, /* S- */
	CMD_SET_DEVICE_NAME = 5, /* SN */
	CMD_GET_DEVICE_NAME = 6, /* GN */
	CMD_SET_SERVICES = 7, /* SS */
	CMD_DUMP_INFOS = 8, /* D */
	CMD_GET_VERSION = 9, /* V */
	CMD_CLEAR_ALL_SERVICES = 10, /* PZ */
	CMD_CREATE_PRIVATE_SERVICE = 11, /* PS */
	CMD_CREATE_PRIVATE_CHARACTERISTIC = 12, /* PC */
	CMD_SERVER_WRITE_CHARACTERISTIC = 13, /* SHW */
	CMD_SERVER_READ_CHARACTERISTIC = 15, /* SHR */
	CMD_NONE = 16,
};

static const char TABLE_COMMAND[][COMMAND_LEN_MAX] =
{
    "$$$",
    "---",
    "R",
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
    "",
};

enum rn4871_mode_e
{
    DATA_MODE,
    COMMAND_MODE,
};

enum rn4871_fsm_e
{
	FSM_STATE_NONE,
	FSM_STATE_INIT,
	FSM_STATE_IDLE,
	FSM_STATE_CONNECTED,
	FSM_STATE_STREAMING,
	FSM_STATE_HALT,
};

enum rn4871_service_e
{
  	DEVICE_INFORMATION = 0x80,
  	UART_TRANSPARENT = 0x40,
  	BEACON = 0x20,
 	RESERVED = 0x10,
};

typedef enum rn4871_code_return_e RN4871_CODE_RETURN;
typedef enum rn4871_cmd_e RN4871_CMD;
typedef enum rn4871_mode_e RN4871_MODE;
typedef enum rn4871_fsm_e RN4871_FSM;
typedef enum rn4871_service_e RN4871_SERVICE;

typedef RN4871_CODE_RETURN (*rn4871_com_fptr_t)(char *buffer, uint16_t *bufferLen);

typedef void (*rn4871_delay_fptr_t)(uint32_t delay);

#endif /* RN4871_DEFS_H */