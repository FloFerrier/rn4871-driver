set(TARGET_NAME "rn4871_interface_test")

add_executable(${TARGET_NAME}
    main_test.c
    test_integration.c
)

target_link_libraries(${TARGET_NAME} PRIVATE rn4871-driver)

target_compile_definitions(${TARGET_NAME} PRIVATE
    -DRN4871_UTEST
)

target_compile_options(${TARGET_NAME} PRIVATE
    --coverage
)

target_link_options(${TARGET_NAME} PRIVATE
    --coverage
)

target_link_libraries(${TARGET_NAME} PRIVATE rn4871-driver cmocka-static)

enable_testing()
add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})