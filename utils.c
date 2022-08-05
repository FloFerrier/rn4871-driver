#include <assert.h>
#include <ctype.h>

#include "utils.h"

bool checkHexaIsCorrect(const char *hexa, size_t size) {
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