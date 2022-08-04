/**
 * \file    rn4871.h
 * \defgroup RN4871_Driver RN4871 Driver APIs
 * \copyright nothing
 *
 * \brief   Header that contains APIs for driver RN4871 module.
 * \details To use driver RN4871 module APIs, the following code shows you
 * the integration on an exemple.
 *
 * @{
 */

#ifndef RN4871_H
#define RN4871_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#include "rn4871_defs.h"

struct rn4871_dev_s {
    rn4871_com_fptr_t uartTx;
    rn4871_com_fptr_t uartRx;
    rn4871_delay_fptr_t delayMs;
    enum rn4871_cmd_e _current_cmd;
    enum rn4871_fsm_e fsm_state;
};

uint8_t rn4871SendCmd(struct rn4871_dev_s *dev, enum rn4871_cmd_e cmd, const char *format, ...);
uint8_t rn4871ResponseProcess(struct rn4871_dev_s *dev, const char *input, char *output);

/*!
 * \details This API allows to enter on command mode for the module.
 * It is the entry point, call this API before using other APIs for configuring
 * the module.
 *
 * \param[in,out] dev : Structure instance of rn4871_dev_s
 *
 * \return Result of API execution status
 * \retval CODE_RETURN_ERROR
 * \retval CODE_RETURN_SUCCESS
 */
uint8_t rn4871EnterCommandMode(struct rn4871_dev_s *dev);

/*!
 * \details This API allows to reboot the module.
 * It is the end point, call this API after using other APIs for configuring
 * the module. The FSM state of module is on IDLE after.
 *
 * \param[in,out] dev : Structure instance of rn4871_dev_s
 *
 * \return Result of API execution status
 * \retval CODE_RETURN_ERROR
 * \retval CODE_RETURN_SUCCESS
 */
uint8_t rn4871RebootModule(struct rn4871_dev_s *dev);

/*!
 * \details This API allows to set services on the module.
 * The module can be set on GATT mode or on Transparent UART mode.
 *
 * \param[in,out] dev : Structure instance of rn4871_dev_s
 * \param[in]     service : bitmap on 16 bit
 *
 * \return Result of API execution status
 * \retval CODE_RETURN_ERROR
 * \retval CODE_RETURN_SUCCESS
 */
uint8_t rn4871SetServices(struct rn4871_dev_s *dev, uint16_t service);

/*!
 * \details This API allows to set a new device name on the module.
 *
 * \param[in,out] dev : Structure instance of rn4871_dev_s
 * \param[in]     deviceName : Buffer that contains device name
 * \param[in]     sizeDeviceName : Size of previous buffer
 *
 * \return Result of API execution status
 * \retval CODE_RETURN_ERROR
 * \retval CODE_RETURN_SUCCESS
 */
uint8_t rn4871SetDeviceName(struct rn4871_dev_s *dev, const char *deviceName, size_t sizeDeviceName);

/*!
 * \details This API allows to erase all GATT services on the module.
 * Need to reboot the module for making effect.
 *
 * \param[in,out] dev : Structure instance of rn4871_dev_s
 *
 * \return Result of API execution status
 * \retval CODE_RETURN_ERROR
 * \retval CODE_RETURN_SUCCESS
 */
uint8_t rn4871EraseAllGattServices(struct rn4871_dev_s *dev);

/*!
 * \details This API allows to get the device name for the module.
 *
 * \param[in,out] dev : Structure instance of rn4871_dev_s
 * \param[out]    deviceName : Buffer that contains device name string
 *
 * \return Result of API execution status
 * \retval CODE_RETURN_ERROR
 * \retval CODE_RETURN_SUCCESS
 */
uint8_t rn4871GetDeviceName(struct rn4871_dev_s *dev, char *deviceName);

/*!
 * \details This API allows to get the firmware version for the module.
 *
 * \param[in,out] dev : Structure instance of rn4871_dev_s
 * \param[out]    firmwareVersion : Buffer that contains firmware version string
 *
 * \return Result of API execution status
 * \retval CODE_RETURN_ERROR
 * \retval CODE_RETURN_SUCCESS
 */
uint8_t rn4871GetFirmwareVersion(struct rn4871_dev_s *dev, char *firmwareVersion);

uint8_t rn4871TransparentUartSendData(struct rn4871_dev_s *dev, uint8_t *pBuffer, uint16_t bufferSize);

#endif /* RN4871_H */

/**@}*/