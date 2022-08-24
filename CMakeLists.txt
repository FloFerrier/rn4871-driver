set(SOURCE_FILES
	rn4871.c
	virtual_module.c
	logs.c)

add_library(librn4871-driver STATIC ${SOURCE_FILES})