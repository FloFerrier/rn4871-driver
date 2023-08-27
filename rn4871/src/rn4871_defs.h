#ifndef RN4871_DEFS_H
#define RN4871_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define RN4871_BUFFER_UART_LEN_MAX (255)
#define DEVICE_NAME_LEN_MAX (20)
#define MAC_ADDRESS_LEN_MAX (20)
#define FW_VERSION_LEN_MAX (5)
#define ERROR_CODE_LEN_MAX (20)
#define COMMAND_LEN_MAX (10)
#define RN4871_ERROR_MAX (10)
#define RN4871_COMMAND_MAX (17)

extern const char RN4871_ERRORS[RN4871_ERROR_MAX][ERROR_CODE_LEN_MAX];
extern const char RN4871_COMMANDS[RN4871_COMMAND_MAX][COMMAND_LEN_MAX];

#ifdef __cplusplus
}
#endif

#endif /* RN4871_DEFS_H */