set(RN4871_CORE_INC "${CMAKE_CURRENT_SOURCE_DIR}/Core/inc")

add_subdirectory(Core)
if(${RN4871_UTEST})
    add_subdirectory(Test)
endif()