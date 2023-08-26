#ifndef TEST_VIRTUAL_MODULE_H
#define TEST_VIRTUAL_MODULE_H

#include <cmocka.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "virtual_module.h"

void tryInitModule_Success(void **state);

void receiveData_CmdEnter(void **state);
void receiveData_CmdQuit(void **state);
void receiveData_CmdIncorrect(void **state);
void receiveData_CmdDumpInfos(void **state);
void receiveData_CmdFirmwareVersion(void **state);
void receiveData_CmdReboot(void **state);
void receiveData_CmdResetFactory(void **state);
void receiveData_CmdGetDeviceName(void **state);
void receiveData_CmdSetDeviceName(void **state);
void receiveData_CmdSetServices(void **state);
void receiveData_AmountOfData(void **state);

void sendData_EmptyString(void **state);
void sendData_AmountOfData(void **state);

void simulateBleExternal_Connection(void **state);
void simulateBleExternal_Streaming(void **state);
void simulateBleExternal_Disconnection(void **state);

void setForceDataMode_Success(void **state);

#endif /* TEST_VIRTUAL_MODULE_H */