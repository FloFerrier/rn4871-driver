#ifndef VIRTUAL_MODULE__H
#define VIRTUAL_MODULE__H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

void uartRxVirtualModule(const uint8_t *pInput, const uint16_t inputSize);
void uartTxVirtualModule(uint8_t *pOutput, uint16_t *outputSize);

#endif /* VIRTUAL_MODULE__H */