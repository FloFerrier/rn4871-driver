# librn4871-driver
Portable library for abstracting RN4871 module functions.
## API
```c
uint8_t rn4871UartTxAPI(uint8_t *pBuffer, uint16_t *bufferSize) {
    /* Your own function to send data through UART */
}

uint8_t rn4871UartRxAPI(uint8_t *pBuffer, uint16_t *bufferSize) {
	/* Your own function to receive data through UART */
}

void rn4871DelayMsAPI(uint32_t delay) {
	/* Your own function to wait a delay in millisecond */
}

struct rn4871_dev_s device = {
    .uartTx = rn4871UartTxAPI,
    .uartRx = rn4871UartRxAPI,
    .delayMs = rn4871DelayMsAPI,
    ._current_cmd = CMD_NONE,
	.fsm_state = FSM_STATE_INIT,
};

/* Must be on command mode for setting things */
rn4871EnterCommandMode(&dev);

rn4871SetServices(&dev, DEVICE_INFORMATION | UART_TRANSPARENT);
char deviceName[21] = "RN4871-Test";
rn4871SetDeviceName(&dev, deviceName, strlen(deviceName));
rn4871EraseAllGattServices(&dev);
rn4871GetDeviceName(&dev, deviceName);
char firmwareVersion[21] = "";
rn4871GetFirmwareVersion(&dev, firmwareVersion);

/* Need to reboot for allowing to set things */
rn4871RebootModule(&dev);

rn4871TransparentUartSendData(&dev, uint8_t *pBuffer, uint16_t bufferSize);
```