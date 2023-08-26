set(TARGET_LIB rn4871-driver)

include_directories(inc)

set(SOURCE_FILES
    src/rn4871_core.c
    src/rn4871_defs.c
    src/rn4871_logger.c
    src/virtual_module.c
)

add_library(${TARGET_LIB} STATIC ${SOURCE_FILES})

if (${RN4871_UTEST})
    target_compile_options(${TARGET_LIB} PRIVATE --coverage)
    target_compile_definitions(${TARGET_LIB} PRIVATE -DRN4871_UTEST)
endif()