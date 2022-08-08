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
#include <stdlib.h>

#include "rn4871_defs.h"

struct rn4871_dev_s {
    rn4871_com_fptr_t uartTx;
    rn4871_com_fptr_t uartRx;
    rn4871_delay_fptr_t delayMs;
};

uint8_t rn4871ResponseProcess(struct rn4871_dev_s *dev, const char *input);

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
 * \details This API allows to get the services for the module.
 *
 * \param[in,out] dev : Structure instance of rn4871_dev_s
 * \param[out]    services : Refer to enum rn4871_service_e for having mask bits
 *
 * \return Result of API execution status
 * \retval CODE_RETURN_ERROR
 * \retval CODE_RETURN_SUCCESS
 */
uint8_t rn4871GetServices(struct rn4871_dev_s *dev, uint16_t *services);

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

/*!
 * \details This API allows to dump infos as :
 *           - BTA    = mac address
 *           - Name   = device name
 *           - Connected
 *           - Authen = authentification
 *           - Features
 *           - Services
 *
 * \param[in,out] dev : Structure instance of rn4871_dev_s
 * \param[out]    macAddress : Buffer that contains infos string
 *
 * \return Result of API execution status
 * \retval CODE_RETURN_ERROR
 * \retval CODE_RETURN_SUCCESS
 */
uint8_t rn4871DumpInfos(struct rn4871_dev_s *dev, char *infos);

/*!
 * \details This API allows to get the mac address for the module.
 *
 * \param[in,out] dev : Structure instance of rn4871_dev_s
 * \param[out]    macAddress : Buffer that contains mac address string
 *
 * \return Result of API execution status
 * \retval CODE_RETURN_ERROR
 * \retval CODE_RETURN_SUCCESS
 */
uint8_t rn4871GetMacAddress(struct rn4871_dev_s *dev, char *macAddress);

/*!
 * \details This API allows to get if services on the module contain the transparent Uart mode.
 *
 * \param[in,out] dev : Structure instance of rn4871_dev_s
 * \param[out]    result : True or False, depending the setting of services.
 *
 * \return Result of API execution status
 * \retval CODE_RETURN_ERROR
 * \retval CODE_RETURN_SUCCESS
 */
uint8_t rn4871IsOnTransparentUart(struct rn4871_dev_s *dev, bool *result);

uint8_t rn4871TransparentUartSendData(struct rn4871_dev_s *dev, uint8_t *pBuffer, uint16_t bufferSize);

/*!
 * \details This API allows to get the FSM state for the module (software FSM).
 * \note FSM = Finite State Machine
 *
 * \return Result of FSM state for the module RN4871
 * \retval FSM_STATE_NONE
 * \retval FSM_STATE_INIT
 * \retval FSM_STATE_IDLE
 * \retval FSM_STATE_CONNECTED
 * \retval FSM_STATE_STREAMING
 * \retval FSM_STATE_HALT
 */
enum rn4871_fsm_e rn4871GetFsmState(void);

/*!
 * \details This API allows to force the FSM state.
 * \warning Use this API only for testing the module
 * \note FSM = Finite State Machine
 *
 * \param[in] fsmForceState State to force on the FSM module.
 */
void rn4871SetForceFsmState(enum rn4871_fsm_e fsmForceState);

/*!
 * \details This API allows to force the data mode.
 * \warning Use this API only for testing the module
 *
 */
void rn4871SetForceDataMode(void);

#endif /* RN4871_H */

/**@}*/