#ifndef GATT_H
#define GATT_H

#include "stdint.h"
#include "stddef.h"
#include "rn4871_defs.h"

#define PRIVATE_CHAR_NB_MAX    (8)
#define PRIVATE_SERVICE_NB_MAX (4)

enum char_properties_e {
    CHAR_PROP_BROADCAST           = 0b00000001,
    CHAR_PROP_READ                = 0b00000010,
    CHAR_PROP_WRITE_NO_ACK        = 0b00000100,
    CHAR_PROP_WRITE               = 0b00001000,
    CHAR_PROP_NOTIFY              = 0b00010000,
    CHAR_PROP_INDICATE            = 0b00100000,
    CHAR_PROP_AUTHENTICATED_WRITE = 0b01000000,
    CHAR_PROP_EXTENDED_PROPERTY   = 0b10000000,
};

struct char_param_s {
    char uuid[32+1];
    uint8_t properties;
    uint8_t size; // in bytes
    char value[15];
};

struct service_param_s {
    char uuid[32+1];
    uint8_t char_nb;
    struct char_param_s *pChar[PRIVATE_CHAR_NB_MAX];
};

struct profile_param_s {
    uint8_t service_nb;
    struct service_param_s *pService[PRIVATE_SERVICE_NB_MAX];
};

#endif /* GATT_H */