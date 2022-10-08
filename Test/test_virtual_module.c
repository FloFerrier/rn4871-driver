#include "test_virtual_module.h"

void test_virtualModuleInit(void **state)
{
    struct virtual_module_s vm;
    virtualModuleInit(&vm);
    assert_string_equal(vm.firmwareVersion, "V1.40");
    assert_string_equal(vm.moduleName, "RN4871-VM");
    assert_string_equal(vm.macAddress, "001122334455");
    assert_int_equal(vm.services, DEVICE_INFORMATION);
}

void test_virtualModuleReceiveData(void **state)
{
    struct virtual_module_s vm;
    virtualModuleInit(&vm);
    assert_false(vm._command_mode);

    virtualModuleReceiveData(&vm, "$");
    virtualModuleReceiveData(&vm, "$");
    virtualModuleReceiveData(&vm, "$");
    assert_string_equal(vm._global_buffer, "CMD> ");
    assert_true(vm._command_mode);

    assert_true(vm._command_mode);
    virtualModuleReceiveData(&vm, "Fake\r\n");
    assert_string_equal(vm._global_buffer, "Err\r\nCMD> ");

    assert_true(vm._command_mode);
    virtualModuleReceiveData(&vm, "D\r\n");
    assert_string_equal(vm._global_buffer, "BTA=001122334455\r\nName=RN4871-VM\r\nConnected=no\r\nAuthen=0\r\nFeatures=0000\r\nServices=80\r\nCMD> ");

    assert_true(vm._command_mode);
    virtualModuleReceiveData(&vm, "V\r\n");
    assert_string_equal(vm._global_buffer, "RN4871 V1.40 7/9/2019 (c)Microship Technology Inc\r\nCMD> ");

    assert_true(vm._command_mode);
    virtualModuleReceiveData(&vm, "PZ\r\n");
    assert_string_equal(vm._global_buffer, "AOK\r\nCMD> ");

    assert_true(vm._command_mode);
    virtualModuleReceiveData(&vm, "R,1\r\n");
    assert_string_equal(vm._global_buffer, "Rebooting\r\nCMD> ");
    assert_false(vm._command_mode);
    vm._command_mode = true;

    assert_true(vm._command_mode);
    virtualModuleReceiveData(&vm, "SF,1\r\n");
    assert_string_equal(vm._global_buffer, "Rebooting\r\nCMD> ");
    assert_false(vm._command_mode);
    vm._command_mode = true;

    assert_true(vm._command_mode);
    virtualModuleReceiveData(&vm, "GN\r\n");
    assert_string_equal(vm._global_buffer, "RN4871-VM\r\nCMD> ");

    assert_true(vm._command_mode);
    virtualModuleReceiveData(&vm, "SN,test\r\n");
    assert_string_equal(vm._global_buffer, "AOK\r\nCMD> ");
    assert_string_equal(vm.moduleName, "test");

    assert_true(vm._command_mode);
    virtualModuleReceiveData(&vm, "SS,C0\r\n");
    assert_string_equal(vm._global_buffer, "AOK\r\nCMD> ");
    assert_int_equal(vm.services, 0xC0);

    assert_true(vm._command_mode);
    virtualModuleReceiveData(&vm, "---\r\n");
    assert_string_equal(vm._global_buffer, "END\r\n");
    assert_false(vm._command_mode);

    assert_false(vm._command_mode);
    virtualModuleReceiveData(&vm, "Test to send some data !\r\n");
    assert_string_equal(vm._global_buffer, "Test to send some data !\r\n");
}

void test_virtualModuleSendData(void **state)
{
    struct virtual_module_s vm;
    char dataToSend[256] = "";
    uint16_t dataToSendLen = 0;
    virtualModuleSendData(&vm, dataToSend, &dataToSendLen);
    assert_string_equal(dataToSend, "");
    assert_int_equal(dataToSendLen, 0);

    strncpy(vm._global_buffer, "Test to send some data", 255);
    virtualModuleSendData(&vm, dataToSend, &dataToSendLen);
    assert_string_equal(dataToSend, "Test to send some data");
    assert_int_equal(dataToSendLen, 22);
}