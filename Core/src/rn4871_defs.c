#include "rn4871_defs.h"

const char ERROR_CODE_STR[ERROR_CODE_NB_MAX][ERROR_CODE_LEN_MAX] = {
    "SUCCESS",
    "ERROR",
    "UART_FAIL",
    "COMMAND_UNKNOWN",
    "NO_COMMAND_MODE",
    "NO_DATA_MODE",
    "NO_TRANSPARENT_UART",
    "NO_CONNECTED",
    "NO_STREAMING",
    "",
};

const char TABLE_COMMAND_STR[COMMAND_NB_MAX][COMMAND_LEN_MAX] = {
    "$$$", "---", "R",  "SF", "S-", "SN",  "GN",  "SS",
    "D",   "V",   "PZ", "PS", "PC", "SHW", "SHR", "",
};