set(TARGET_LIB rn4871-driver)

add_library(${TARGET_LIB} STATIC)

target_include_directories(${TARGET_LIB} PUBLIC
    include
)

target_sources(${TARGET_LIB} PRIVATE
    src/rn4871_core.c
    src/rn4871_defs.c
    src/rn4871_logger.c
    src/virtual_module.c
)