set(TARGET_NAME "rn4871_unit_test")

include_directories(
    ${CMAKE_SOURCE_DIR}/rn4871/include
    ${CMAKE_SOURCE_DIR}/rn4871/src
)

add_executable(${TARGET_NAME}
    ${CMAKE_SOURCE_DIR}/rn4871/src/rn4871_core.c
    ${CMAKE_SOURCE_DIR}/rn4871/src/rn4871_defs.c
    ${CMAKE_SOURCE_DIR}/rn4871/src/rn4871_logger.c

    main_test.c
    test_rn4871.c
    mock_rn4871.c
)

target_compile_definitions(${TARGET_NAME} PRIVATE
    -DRN4871_UTEST
)

target_compile_options(${TARGET_NAME} PRIVATE
    --coverage
)

target_link_options(${TARGET_NAME} PRIVATE
    --coverage
)

target_link_libraries(${TARGET_NAME} PRIVATE cmocka-static)

enable_testing()
add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})