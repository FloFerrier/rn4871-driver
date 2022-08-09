option(BUILD_DOC "Build documentation" ON)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target(doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
else(DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif(DOXYGEN_FOUND)

set(SOURCE_FILES
	rn4871_defs.h
	rn4871.c
	rn4871.h
	virtual_module.c
	virtual_module.h
  utils.c
  utils.h
  logs.c
  logs.h)

set(RN4871_LOG_ENABLE 0x7)

add_library(librn4871-driver STATIC ${SOURCE_FILES})

set_target_properties(librn4871-driver PROPERTIES LINK_FLAGS "--coverage")

target_compile_definitions(librn4871-driver PRIVATE RN4871_LOG_ENABLE=${RN4871_LOG_ENABLE})