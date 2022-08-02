#include "virtual_module.h"

#define BUFFER_MAX_LEN (255)

#define PRIVATE_UUID_BYTES_SIZE     16
#define PRIVATE_UUID_ASCII_SIZE     (PRIVATE_UUID_BYTES_SIZE*2)

#define CHAR_PROPERTIES_BYTES_SIZE  1
#define CHAR_PROPERTIES_ASCII_SIZE  (CHAR_PROPERTIES_BYTES_SIZE*2)

#define CHAR_HANDLE_BYTES_SIZE      2
#define CHAR_HANDLE_ASCII_SIZE      (CHAR_HANDLE_BYTES_SIZE*2)

#define CHAR_VALUES_BYTES_SIZE      1
#define CHAR_VALUES_ASCII_SIZE      (CHAR_VALUES_BYTES_SIZE*2)

struct virtual_module_s {
    bool _commandMode;
    bool _transparentUart;
    char _bt_name[256];
    char _module_name[25];
};

struct virtual_module_s virtualModule = {
    ._commandMode = false,
    ._transparentUart = false,
    ._bt_name = "",
    ._module_name = "",
};

char pGlobalBuffer[BUFFER_MAX_LEN+1] = "";

static bool _checkHexaIsCorrect(const char *hexa, size_t size);

bool _checkHexaIsCorrect(const char *hexa, size_t size) {
    assert(NULL != hexa);

    if(size != strlen(hexa)) {
        return false;
    }

    /* Must contain only [A-F] or [0-9] characters */
    for(int idx=0; idx < size; idx++) {
        if(0 == isxdigit(hexa[idx]))
            return false;
    }
    return true;
}

void uartRxVirtualModule(const uint8_t *pInput, const uint16_t inputSize) {
    assert(NULL != pInput);
    memset(pGlobalBuffer, '\0', BUFFER_MAX_LEN);
    static uint8_t cnt_cmd_mode = 0;
    if(0 == strcmp(pInput, "$")) {
        ++cnt_cmd_mode;
        if (3 == cnt_cmd_mode) {
            strncpy(pGlobalBuffer, "CMD>", BUFFER_MAX_LEN);
            cnt_cmd_mode = 0;
        }
    }
    else if (0 == strcmp(pInput, "R,1\r\n")){
        strncpy(pGlobalBuffer, "Rebooting\r\n%REBOOT%", BUFFER_MAX_LEN);
    }
    else if (0 == strcmp(pInput, "SF,1\r\n")){
        strncpy(pGlobalBuffer, "Rebooting\r\n%REBOOT%", BUFFER_MAX_LEN);
    }
    else if (0 == strcmp(pInput, "SF,2\r\n")){
        strncpy(pGlobalBuffer, "Rebooting\r\n%REBOOT%", BUFFER_MAX_LEN);
    }
    else if (0 != strstr(pInput, "S-")){
        char delimiter[] = ",\r";
        char *saveptr;
        char *token = strtok_r((char*)pInput, delimiter, &saveptr);
        token = strtok_r(NULL, delimiter, &saveptr);
        strcpy(virtualModule._bt_name, token);
        strncpy(pGlobalBuffer, "AOK\r\nCMD>", BUFFER_MAX_LEN);
    }
    else if (0 != strstr(pInput, "SN")){
        char delimiter[] = ",\r";
        char *saveptr;
        char *token = strtok_r((char*)pInput, delimiter, &saveptr);
        token = strtok_r(NULL, delimiter, &saveptr);
        strcpy(virtualModule._module_name, token);
        strncpy(pGlobalBuffer, "AOK\r\nCMD>", BUFFER_MAX_LEN);
    }
    else if (0 == strcmp(pInput, "GN\r\n")){
        snprintf(pGlobalBuffer, BUFFER_MAX_LEN, "%s\r\nCMD>", virtualModule._module_name);
    }
    else if (0 != strstr(pInput, "SS")){
        char delimiter[] = ",\r";
        char *saveptr;
        char *token = strtok_r((char*)pInput, delimiter, &saveptr);
        token = strtok_r(NULL, delimiter, &saveptr);
        if (0 == strcmp("C0", token))
            virtualModule._transparentUart = true;
        strncpy(pGlobalBuffer, "AOK\r\nCMD>", BUFFER_MAX_LEN);
    }
    else if (0 == strcmp(pInput, "D\r\n")){
        /* Send Dump infos */
    }
    else if (0 == strcmp(pInput, "V\r\n")){
        /* Send firmware version */
    }
    else if (0 == strcmp(pInput, "PZ\r\n")){
        strncpy(pGlobalBuffer, "AOK\r\nCMD>", BUFFER_MAX_LEN);
    }
    else if (0 != strstr(pInput, "PS")){
        /* Get services UUID */
        char delimiter[] = ",\r";
        char *saveptr;
        char *token = strtok_r((char*)pInput, delimiter, &saveptr);
        token = strtok_r(NULL, delimiter, &saveptr);
        if (!_checkHexaIsCorrect(token, PRIVATE_UUID_ASCII_SIZE))
            strncpy(pGlobalBuffer, "Err\r\nCMD>", BUFFER_MAX_LEN);
        else
            strncpy(pGlobalBuffer, "AOK\r\nCMD>", BUFFER_MAX_LEN);
    }
    else if (0 != strstr(pInput, "PC")){
        char delimiter[] = ",\r";
        char *saveptr;
        char *token = strtok_r((char*)pInput, delimiter, &saveptr);
        /* Get characteristic UUID */
        token = strtok_r(NULL, delimiter, &saveptr);
        if (!_checkHexaIsCorrect(token, PRIVATE_UUID_ASCII_SIZE)) {
            strncpy(pGlobalBuffer, "Err\r\nCMD>", BUFFER_MAX_LEN);
            return;
        }
        /* Get characteristic properties */
        token = strtok_r(NULL, delimiter, &saveptr);
        if (!_checkHexaIsCorrect(token, CHAR_PROPERTIES_ASCII_SIZE)) {
            strncpy(pGlobalBuffer, "Err\r\nCMD>", BUFFER_MAX_LEN);
            return;
        }
        /* Get characteristic value size */
        token = strtok_r(NULL, delimiter, &saveptr);
        if (!_checkHexaIsCorrect(token, CHAR_VALUES_ASCII_SIZE))
            strncpy(pGlobalBuffer, "Err\r\nCMD>", BUFFER_MAX_LEN);
        else
            strncpy(pGlobalBuffer, "AOK\r\nCMD>", BUFFER_MAX_LEN);
    }
    else if (0 != strstr(pInput, "SHW")){
        char delimiter[] = ",\r";
        char *saveptr;
        char *token = strtok_r((char*)pInput, delimiter, &saveptr);
        /* Get characteristic handle */
        token = strtok_r(NULL, delimiter, &saveptr);
        if (!_checkHexaIsCorrect(token, CHAR_HANDLE_ASCII_SIZE)) {
            strncpy(pGlobalBuffer, "Err\r\nCMD>", BUFFER_MAX_LEN);
            return;
        }
        /* Get characteristic value */
        token = strtok_r(NULL, delimiter, &saveptr);
        if (!_checkHexaIsCorrect(token, CHAR_VALUES_ASCII_SIZE)) {
            strncpy(pGlobalBuffer, "Err\r\nCMD>", BUFFER_MAX_LEN);
            return;
        }
        else
            strncpy(pGlobalBuffer, "AOK\r\nCMD>", BUFFER_MAX_LEN);
    }
    else if (0 != strstr(pInput, "SHR")){
        char delimiter[] = ",\r";
        char *saveptr;
        char *token = strtok_r((char*)pInput, delimiter, &saveptr);
        /* Get characteristic handle */
        token = strtok_r(NULL, delimiter, &saveptr);
        if (!_checkHexaIsCorrect(token, CHAR_HANDLE_ASCII_SIZE))
            strncpy(pGlobalBuffer, "Err\r\nCMD>", BUFFER_MAX_LEN);
        else
            strncpy(pGlobalBuffer, "12\r\nCMD>", BUFFER_MAX_LEN);
    }
    else {
        strncpy(pGlobalBuffer, "Err\r\nCMD>", BUFFER_MAX_LEN);
    }
}

void uartTxVirtualModule(uint8_t *pOutput, uint16_t *outputSize) {
    assert((NULL != pOutput) || (NULL != outputSize));
    strncpy(pOutput, pGlobalBuffer, BUFFER_MAX_LEN);
    *outputSize = strnlen(pGlobalBuffer, BUFFER_MAX_LEN);
}