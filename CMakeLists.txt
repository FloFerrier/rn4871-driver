set(SOURCE_FILES
	rn4871_defs.h
	rn4871.c
	rn4871.h
	virtual_module.c
	virtual_module.h)

add_library(librn4871-driver STATIC ${SOURCE_FILES})

set_target_properties(librn4871-driver PROPERTIES LINK_FLAGS "--coverage")