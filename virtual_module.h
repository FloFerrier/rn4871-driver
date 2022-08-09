/**
 * \file    virtual_module.h
 * \defgroup Virtual_Module Virtual Module
 * \copyright nothing
 *
 * \brief   Header that contains APIs for using virtual module of RN4871.
 * \details To use virtual module APIs, the following code shows you
 * the integration on the driver RN4871 module.
 *
 * 
 * @{
 */

#ifndef VIRTUAL_MODULE__H
#define VIRTUAL_MODULE__H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "rn4871.h"

/*!
 * \details This API allows to emulate UART Rx on the virtual module.
 *
 * \param[in] pInput    : Buffer that contains elements to receive by the module.
 * \param[in] inputSize : Size of elements on the buffer pInput.
 *
 */
void virtualModuleReceiveData(const uint8_t *pInput, const uint16_t inputSize);

/*!
 * \details This API allows to emulate UART Tx on the virtual module.
 *
 * \param[out] pOutput    : Buffer that contains elements to transmit by the module.
 * \param[out] outputSize : Size of elements on the buffer pOutput.
 *
 */
void virtualModuleSendData(uint8_t *pOutput, uint16_t *outputSize);

void virtualModuleConnect(struct rn4871_dev_s *dev);
void virtualModuleStream(struct rn4871_dev_s *dev);
void virtualModuleDisconnect(struct rn4871_dev_s *dev);
void virtualModuleSetForceDataMode(void);

#endif /* VIRTUAL_MODULE__H */
 /**@}*/