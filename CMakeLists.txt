include(cmake/cmocka.cmake)

set(RN4871_CORE_INC "${CMAKE_CURRENT_SOURCE_DIR}/Core/inc")

add_subdirectory(Core)
add_subdirectory(Test)