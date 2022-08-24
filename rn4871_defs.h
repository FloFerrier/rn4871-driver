#ifndef RN4871_DEFS_H
#define RN4871_DEFS_H

#define BUFFER_UART_LEN_MAX (255)

#define DEVICE_NAME_CHARACTER_MAX (20)

enum rn4871_code_return_e {
  CODE_RETURN_SUCCESS,
  CODE_RETURN_ERROR,
  CODE_RETURN_UART_FAIL,
  CODE_RETURN_CMD_UNKNOWN,
  CODE_RETURN_NO_COMMAND_MODE,
  CODE_RETURN_NO_DATA_MODE,
  CODE_RETURN_NO_TRANSPARENT_UART,
  CODE_RETURN_NO_CONNECTED,
  CODE_RETURN_NO_STREAMING,
  CODE_RETURN_UUID_INCORRECT,
  CODE_RETURN_PROP_INCORRECT,
  CODE_RETURN_SIZE_INCORRECT,
};

enum rn4871_cmd_e {
  CMD_MODE_ENTER, /* $$$ */
  CMD_MODE_QUIT, /* --- */
  CMD_REBOOT, /* R */
  CMD_RESET_FACTORY, /* SF */
  CMD_SET_BT_NAME, /* S- */
  CMD_SET_DEVICE_NAME, /* SN */
  CMD_GET_DEVICE_NAME, /* GN */
  CMD_SET_SERVICES, /* SS */
  CMD_DUMP_INFOS, /* D */
  CMD_GET_VERSION, /* V */
  CMD_CLEAR_ALL_SERVICES, /* PZ */
  CMD_CREATE_PRIVATE_SERVICE, /* PS */
  CMD_CREATE_PRIVATE_CHARACTERISTIC, /* PC */
  CMD_SERVER_WRITE_CHARACTERISTIC, /* SHW */
  CMD_SERVER_READ_CHARACTERISTIC, /* SHR */
  CMD_NONE,
};

enum rn4871_fsm_e {
  FSM_STATE_NONE,
  FSM_STATE_INIT,
  FSM_STATE_IDLE,
  FSM_STATE_CONNECTED,
  FSM_STATE_STREAMING,
  FSM_STATE_HALT,
};

enum rn4871_service_e {
  DEVICE_INFORMATION = 0x80,
  UART_TRANSPARENT   = 0x40,
  BEACON             = 0x20,
  RESERVED           = 0x10,
};

static const char TABLE_COMMAND[][10] = {
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

typedef uint8_t (*rn4871_com_fptr_t)(char *buffer, uint16_t *bufferLen);

typedef void (*rn4871_delay_fptr_t)(uint32_t delay);

#endif /* RN4871_DEFS_H */