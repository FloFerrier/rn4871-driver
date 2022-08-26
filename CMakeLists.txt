set(SOURCE_FILES
	rn4871_core.c
	rn4871_logger.c
	virtual_module.c
)

add_library(librn4871-driver STATIC ${SOURCE_FILES})