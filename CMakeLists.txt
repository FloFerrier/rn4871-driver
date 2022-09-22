set(TARGET_LIB rn4871-driver)

set(SOURCE_FILES
	rn4871_core.c
	rn4871_logger.c
	virtual_module.c
)

add_library(${TARGET_LIB} STATIC ${SOURCE_FILES})