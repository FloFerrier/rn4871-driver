#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "test_virtual_module.h"

int setup(void **state) {
  VIRTUAL_MODULE *module = malloc(sizeof(VIRTUAL_MODULE));
  virtualModuleInit(module);
  *state = module;
  return 0;
}

int teardown(void **state) {
  VIRTUAL_MODULE *module = *state;
  free(module);
  return 0;
}

int main() {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(tryInitModule_Success),

      cmocka_unit_test_setup_teardown(receiveData_CmdEnter, setup, teardown),
      cmocka_unit_test_setup_teardown(receiveData_CmdQuit, setup, teardown),
      cmocka_unit_test_setup_teardown(receiveData_CmdIncorrect, setup,
                                      teardown),
      cmocka_unit_test_setup_teardown(receiveData_CmdDumpInfos, setup,
                                      teardown),
      cmocka_unit_test_setup_teardown(receiveData_CmdFirmwareVersion, setup,
                                      teardown),
      cmocka_unit_test_setup_teardown(receiveData_CmdReboot, setup, teardown),
      cmocka_unit_test_setup_teardown(receiveData_CmdResetFactory, setup,
                                      teardown),
      cmocka_unit_test_setup_teardown(receiveData_CmdGetDeviceName, setup,
                                      teardown),
      cmocka_unit_test_setup_teardown(receiveData_CmdSetDeviceName, setup,
                                      teardown),
      cmocka_unit_test_setup_teardown(receiveData_CmdSetServices, setup,
                                      teardown),
      cmocka_unit_test_setup_teardown(receiveData_AmountOfData, setup,
                                      teardown),

      cmocka_unit_test_setup_teardown(sendData_EmptyString, setup, teardown),
      cmocka_unit_test_setup_teardown(sendData_AmountOfData, setup, teardown),

      cmocka_unit_test_setup_teardown(simulateBleExternal_Connection, setup,
                                      teardown),
      cmocka_unit_test_setup_teardown(simulateBleExternal_Streaming, setup,
                                      teardown),
      cmocka_unit_test_setup_teardown(simulateBleExternal_Disconnection, setup,
                                      teardown),

      cmocka_unit_test_setup_teardown(setForceDataMode_Success, setup,
                                      teardown),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}