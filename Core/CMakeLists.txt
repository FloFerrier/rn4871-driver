set(TARGET_LIB rn4871-driver)

include_directories(inc)

set(SOURCE_FILES
	src/rn4871_core.c
	src/rn4871_logger.c
	src/virtual_module.c
)

add_library(${TARGET_LIB} STATIC ${SOURCE_FILES})