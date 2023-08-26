#ifndef RN4871_CORE_H
#define RN4871_CORE_H

#include "rn4871_api.h"

typedef enum {
  FIELD_MAC_ADDRESS,
  FIELD_DEVICE_NAME,
  FIELD_CONNECTION,
  FIELD_AUTHENTIFICATION,
  FIELD_FEATURES,
  FIELD_SERVICES,
} RN4871_DUMP_INFOS_FIELD;

#ifdef RN4871_UTEST
RN4871_CODE_RETURN rn4871SendCmd(RN4871_MODULE *dev, RN4871_CMD cmd,
                                 const char *format, ...);
RN4871_CODE_RETURN rn4871ResponseProcess(RN4871_MODULE *dev,
                                         const char *response);
RN4871_CODE_RETURN rn4871ParseDumpInfos(const char *infos,
                                        RN4871_DUMP_INFOS_FIELD field,
                                        char *result, uint16_t resultMaxLen);
RN4871_CODE_RETURN rn4871ParseFirmwareVersion(const char *firmwareVersion,
                                              char *result,
                                              uint16_t resultMaxLen);
#endif

#endif /* RN4871_API_H */