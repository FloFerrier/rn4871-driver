set(TARGET_NAME "Driver_Rn4871_Test")

include_directories(${RN4871_CORE_INC})

add_executable(${TARGET_NAME}
    main_test.c
    test_rn4871.c
    mock_rn4871.c
)

target_link_libraries(${TARGET_NAME} PRIVATE rn4871-driver)

target_compile_options(${TARGET_NAME} PRIVATE
    --coverage
)

target_link_options(${TARGET_NAME} PRIVATE
    --coverage
)

target_link_libraries(${TARGET_NAME} PRIVATE rn4871-driver cmocka-static)

enable_testing()
add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})