# RN4871 module API
## Introduction
This package contains a **portable library** for abstracting RN4871 module API.
RN4871 is a hardware module that supports **BLE** (Bluetooth Low Energy) and distributed by **Microship** [link to Website](https://www.microchip.com/en-us/product/RN4871).
The module is designed to allow an easily interfacing with a device like a microcontroller via a **standard UART**. So, developers can used a module with a completely integrated Bluetooth software stack and add a wireless connectivity to their products.
The module supports **GATT** (Generic ATTribute Profile), this acronym defines the way that two BLE devices transfer data. The data protocol uses concepts named **Services** and **Characteristics**. On this structure, the module contains a specific service for simulating an UART communication named **Transparent UART**.
## User guide
### Default UART settings
The driver do not allow to change UART settings on the module.
So, we consider to use the default settings :
- Baudrate: 115200
- Databits: 8
- Parity: None
- Stopbits: 1
- Flowcontrol: Disabled

> **Notes:** If you want change this, you must to adapt your embedded UART setting but also send good commands to the module (independatly to the driver).

### Software FSM (Finite-State Machine)
Todo
### Interfacing with callback
The driver uses callback for abstracting embedded hardware. So, developers
must define callback functions for using UART communication and delay.
These callbacks are specific to your hardware !
You can reuse this template for declarating callbacks :
> **Notes:** The function **rn4871LogSender** is not a callback but a simple extern function declaration.
```c
void rn4871LogSender(char *log, int len) {
	/* Check if argument is NULL or not */
	assert(NULL != log);

	/* Typically, logs are sending to serial port (like printf functions).
	 * But, be careful to define this function because microcontroller
	 * can not support this natively.
	 */
}

uint8_t rn4871UartTxCb(char *buf, uint16_t *len) {
	/* Check if arguments are NULL or not */
	assert((NULL != buf) || (NULL != len);

	/* Add function to allow send amount of data
	 * through UART port.
	 * The pointer buf must contain all the data and len must 
	 * contain the amount of data.
	 * Hardware module sends only ASCII character (on 8 bits).
	 */

	/* Return code
	 * CODE_RETURN_SUCCESS
	 * CODE_RETURN_UART_FAIL
	 * See rn4871_defs.h for other code return
	 */
    return CODE_RETURN_UART_FAIL;
}

uint8_t rn4871UartRxCb(char *buf, uint16_t *len) {
	/* Check if arguments are NULL or not */
	assert((NULL != buf) || (NULL != len);

	/* Add function to allow receive amount of data
	 * through UART port.
	 * The pointer buf must return all the data and len must return
	 * the amount of data.
	 * Hardware module sends only ASCII character (on 8 bits).
	 */

	/* Return code
	 * CODE_RETURN_SUCCESS
	 * CODE_RETURN_UART_FAIL
	 * See rn4871_defs.h for other code return
	 */
    return CODE_RETURN_UART_FAIL;
}

void rn4871DelayMsCb(uint32_t delay) {
	/* Wait a delay in milliseconds */
}
```
### Structure instantiation
The driver supports only one instantiation of RN4871 structure because
we consider that one microcontroller will control only one RN4871 module.
```c
struct rn4871_dev_s dev = {
	.uartTx = rn4871UartTxCb,
	.uartRx = rn4871UartRxCb,
	.delayMs = rn4871DelayMsCb,
};
```
### Code return for function APIs
A set of code return is describing on the file **rn4871_defs.h**.
An enumerator variable allows to simplify the comprehension.
```c
enum rn4871_code_return_e {
  CODE_RETURN_SUCCESS,
  CODE_RETURN_ERROR,
  CODE_RETURN_UART_FAIL,
  CODE_RETURN_CMD_UNKNOWN,
  CODE_RETURN_NO_COMMAND_MODE,
  CODE_RETURN_NO_DATA_MODE,
  CODE_RETURN_NO_TRANSPARENT_UART,
  CODE_RETURN_NO_CONNECTED,
  CODE_RETURN_NO_STREAMING,
};
```
### Functions APIs
#### Functions for configuring the module
The module can be set by different functions. For using
these functions, the module must to be on command mode.
To enter on command mode, we have this function:
```c
uint8_t rn4871EnterCommandMode(struct rn4871_dev_s *dev);
```
To quit the command mode, we have two possibilities:
> **Notes:** For activating some settings, the module must to be reboot.
```c
uint8_t rn4871QuitCommandMode(struct rn4871_dev_s *dev);
uint8_t rn4871RebootModule(struct rn4871_dev_s *dev);
```
After entering on command mode, you can use theses functions :
```c
uint8_t rn4871SetServices(struct rn4871_dev_s *dev, uint16_t service);
uint8_t rn4871SetDeviceName(struct rn4871_dev_s *dev, const char *deviceName, uint16_t deviceNameLen);
uint8_t rn4871GetServices(struct rn4871_dev_s *dev, uint16_t *services);
uint8_t rn4871GetDeviceName(struct rn4871_dev_s *dev, char *deviceName);
uint8_t rn4871GetFirmwareVersion(struct rn4871_dev_s *dev, char *firmwareVersion);
uint8_t rn4871DumpInfos(struct rn4871_dev_s *dev, char *infos);
uint8_t rn4871GetMacAddress(struct rn4871_dev_s *dev, char *macAddress);
uint8_t rn4871IsOnTransparentUart(struct rn4871_dev_s *dev, bool *result);
```
On data mode, you can use these functions :
```c
uint8_t rn4871ReceivedDataProcess(struct rn4871_dev_s *dev);
uint8_t rn4871TransparentUartSendData(struct rn4871_dev_s *dev, const char *dataToSend, uint16_t dataToSendLen);
```
To help us, a software **FSM** (Finite-State Machine)
is implemented for having the state of the module. 
```c
enum rn4871_fsm_e rn4871GetFsmState(void);
```
### Exemple
This exemple allows to describe sequence for using Transparent UART mode. 
```c
```
## Release Note
- **v0.1.0**
	- Support only gatt **server mode**
	- Support **Transparent UART**

## Roadmap
- **v0.2.0**
	- Add Gatt support with **custom services and characteristics**

> **Note:** vX.Y.Z release notation :
> X : Major version without API compatibily
> Y : Minor version with continuous compatibility
> Z : Patch for bug fixes