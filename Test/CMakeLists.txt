set(TARGET "Rn4871")

include(CTest)

set(TEST_TARGET "${TARGET}_test" CACHE INTERNAL "")

include_directories(${RN4871_CORE_INC})

add_executable(${TEST_TARGET}
    main_test.c
    test_rn4871.c
    test_virtual_module.c
    mock_rn4871.c
)

target_link_libraries(${TEST_TARGET} PRIVATE rn4871-driver)

target_compile_options(${TEST_TARGET} PRIVATE
    --coverage
)

target_link_options(${TEST_TARGET} PRIVATE
    --coverage
)

target_link_libraries(${TEST_TARGET} PRIVATE rn4871-driver cmocka-static)

enable_testing()
add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})