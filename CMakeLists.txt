set(SOURCE_FILES
	rn4871_defs.h
	rn4871.c
	rn4871.h
	virtual_module.c
	virtual_module.h
  logs.c
  logs.h)

set(RN4871_LOG_ENABLE 0x7)

add_library(librn4871-driver STATIC ${SOURCE_FILES})

set_target_properties(librn4871-driver PROPERTIES LINK_FLAGS "--coverage")

target_compile_definitions(librn4871-driver PRIVATE RN4871_LOG_ENABLE=${RN4871_LOG_ENABLE})