# RN4871 module API
## Introduction
This package contains a **portable library** for abstracting RN4871 module API.
RN4871 is a hardware module that supports **BLE** (Bluetooth Low Energy) and distributed by **Microship** [link to Website](https://www.microchip.com/en-us/product/RN4871).
The module is designed to allow an easily interfacing with a device like a microcontroller via a **standard UART**. So, developers can used a module with a completely integrated Bluetooth software stack and add a wireless connectivity to their products.
The module supports **GATT** (Generic ATTribute Profile), this acronym defines the way that two BLE devices transfer data. The data protocol uses concepts named **Services** and **Characteristics**. On this structure, the module contains a specific service for simulating an UART communication named **Transparent UART**.
## Getting Started
The project uses CMake as generator of toolchain.
### Build source code as a library
The RN4871 driver is designed to be compile as a c library and integrate on your embedded system.
```bash
mkdir build_target && cd build_target
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target all
```
### Build and Run Test Suite
The RN4871 driver contains standalone test suite to validate the running. CMocka is used as a test suite framework.
```bash
mkdir build_test && cd build_test
cmake .. -DCMAKE_BUILD_TYPE=Test
cmake --build . --target all
ctest -V
```
All test binaries are stored on the bin folder.
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
### Interfacing with callback
The driver uses callback for abstracting embedded hardware. So, developers
must define callback functions for using UART communication and delay.
These callbacks are specific to your hardware !
You can reuse this template for declarating callbacks :
> **Notes:** The function **rn4871LogSender** is not a callback but a simple extern function declaration.
```c
void rn4871LogSender(char *log, int len)
{
	/* Check if argument is NULL or not */
	assert(NULL != log);

	/* Typically, logs are sending to serial port (like printf functions).
	 * But, be careful to define this function because microcontroller
	 * can not support this natively.
	 */
}

RN4871_CODE_RETURN rn4871UartTxCb(char *buf, uint16_t *len)
{
	/* Check if arguments are NULL or not */
	assert((NULL != buf) || (NULL != len));

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

RN4871_CODE_RETURN rn4871UartRxCb(char *buf, uint16_t *len)
{
	/* Check if arguments are NULL or not */
	assert((NULL != buf) || (NULL != len));

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

void rn4871DelayMsCb(uint32_t delay)
{
	/* Wait a delay in milliseconds */
}
```
### Structure instantiation and init function
The driver supports multi-instantiate.
```c
RN4871_CONFIG config =
{
	.deviceName = "RN4871-standalone",
	.services = DEVICE_INFORMATION | UART_TRANSPARENT,
};

RN4871_MODULE module =
{
	.uartTx = rn4871UartTxCb,
	.uartRx = rn4871UartRxCb,
	.delayMs = rn4871DelayMsCb,
};

uint8_t ret = rn4871Init(&module);
```
### Code return for function APIs
A set of code return is describing on the file **rn4871_defs.h**.
An enumerator variable allows to simplify the comprehension.
```c
typedef enum
{
	CODE_RETURN_SUCCESS,
	CODE_RETURN_ERROR,
  	CODE_RETURN_UART_FAIL,
  	CODE_RETURN_CMD_UNKNOWN,
  	CODE_RETURN_NO_COMMAND_MODE,
 	CODE_RETURN_NO_DATA_MODE,
  	CODE_RETURN_NO_TRANSPARENT_UART,
	CODE_RETURN_NO_CONNECTED,
	CODE_RETURN_NO_STREAMING,
} RN4871_CODE_RETURN;
```
### Functions APIs
#### Functions for configuring the module
The module can be set by different functions. For using
these functions, the module must to be on command mode.
To enter on command mode, we have this function:
```c
RN4871_CODE_RETURN rn4871EnterCommandMode(RN4871_MODULE *dev);
```
To quit the command mode, we have two possibilities:
> **Notes:** For activating some settings, the module must to be reboot.
```c
RN4871_CODE_RETURN rn4871QuitCommandMode(RN4871_MODULE *dev);
RN4871_CODE_RETURN rn4871RebootModule(RN4871_MODULE *dev);
```
After entering on command mode, you can use theses setter/getter functions :
```c
RN4871_CODE_RETURN rn4871EraseAllGattServices(RN4871_MODULE *dev);
RN4871_CODE_RETURN rn4871SetServices(RN4871_MODULE *dev, uint16_t service);
RN4871_CODE_RETURN rn4871SetDeviceName(RN4871_MODULE *dev, const char *deviceName, uint16_t deviceNameLen);
RN4871_CODE_RETURN rn4871SetConfig(RN4871_MODULE *dev, RN4871_CONFIG *config);
RN4871_CODE_RETURN rn4871DumpInfos(RN4871_MODULE *dev, char *infos);
RN4871_CODE_RETURN rn4871GetServices(RN4871_MODULE *dev, uint16_t *services);
RN4871_CODE_RETURN rn4871GetDeviceName(RN4871_MODULE *dev, char *deviceName);
RN4871_CODE_RETURN rn4871GetMacAddress(RN4871_MODULE *dev, char *macAddress);
RN4871_CODE_RETURN rn4871GetFirmwareVersion(RN4871_MODULE *dev, char *firmwareVersion);
RN4871_CODE_RETURN rn4871GetConfig(RN4871_MODULE *dev,RN4871_CONFIG *config);
RN4871_CODE_RETURN rn4871IsOnTransparentUart(RN4871_MODULE *dev, bool *result);
char* rn4871GetErrorCodeStr(RN4871_CODE_RETURN errorCode);
```
On data mode, you can use these functions :
```c
RN4871_CODE_RETURN rn4871WaitReceivedData(RN4871_MODULE *dev, char *receivedData, uint16_t *receivedDataLen);
RN4871_CODE_RETURN rn4871TransparentUartSendData(RN4871_MODULE *dev, const char *dataToSend, uint16_t dataToSendLen);
```
To help us, a software **FSM** (Finite-State Machine)
is implemented for having the state of the module. 
```c
RN4871_FSM rn4871GetFsmState(void);
```
### Internal Driver Log
The driver implements an internal logger, you can enable log with the variable **RN4871_LOG_ENABLE**.
```c
#define RN4871_LOG_ENABLE (LOG_ERROR | LOG_DEBUG | LOG_INFO)
```
You find the bitmap on the file **rn4871_logger.h**. 
```c
typedef enum
{
    LOG_ERROR = 0x1,
    LOG_DEBUG = 0x2,
    LOG_INFO  = 0x4,
} RN4871_LOG_LEVEL;
```
### Virtual Module
If you do not have the hardware module, the driver contains a virtual module for emulating it.
```c
void rn4871SetForceFsmState(RN4871_FSM fsmForceState);
void rn4871SetForceDataMode(void);
void virtualModuleInit(VIRTUAL_MODULE *virtualModule);
void virtualModuleReceiveData(char *dataReceived, uint16_t dataReceivedLen);
void virtualModuleSendData(char *dataToSend, uint16_t *dataToSendLen);
void virtualModuleConnect(RN4871_MODULE *dev);
void virtualModuleStream(RN4871_MODULE *dev);
void virtualModuleDisconnect(RN4871_MODULE *dev);
void virtualModuleSetForceDataMode(void);
```
### Usecase
This exemple allows to describe sequence for using Transparent UART mode.
> **Notes:** All specific hardware configuration for your microcontroller are not describe it.
```c
/* Instantiate structures */
RN4871_CONFIG config =
{
	.deviceName = "RN4871-standalone",
	.services = DEVICE_INFORMATION | UART_TRANSPARENT,
};

RN4871_MODULE dev =
{
	.uartTx = rn4871UartTxCb,
	.uartRx = rn4871UartRxCb,
	.delayMs = rn4871DelayMsCb,
};

/* Init the module */
rn4871Init(&dev);

/* Set the module and reboot for taking effect */
rn4871EnterCommandMode(&dev);
rn4871SetConfig(&dev, &config);
rn4871RebootModule(&dev);

/* Get config and display this */
rn4871EnterCommandMode(&dev);
rn4871GetConfig(&dev, &config);
printf("------ Configuration ------\r\n");
printf("Device name: %s\r\n", config.deviceName);
printf("Firmware version: %s\r\n", config.firmwareVersion);
printf("Mac address: %s\r\n", config.macAddress);
printf("Services: 0x%02X\r\n", config.services);
printf("---------------------------\r\n");
rn4871QuitCommandMode(&dev);

/* Wait a Streaming connection by external Bluetooth */
char receivedData[256] = "";
uint16_t receivedDataLen = 0;
while(FSM_STATE_STREAMING != rn4871GetFsmState(&dev))
{
	/* Blocking state here */
	rn4871WaitReceivedData(&module, receivedData, receivedDataLen);
}

/* Send amount of data */
char dataToSend[] = "Hello World";
uint16_t dataToSendLen = strlen(dataToSend);
while(FSM_STATE_STREAMING == rn4871GetFsmState(&dev))
{
	rn4871TransparentUartSendData(&module, dataToSend, dataToSendLen);
	rn4871WaitReceivedData(&module, receivedData, receivedDataLen);
	sleep(10); // in second
}
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

## License
MIT 2022