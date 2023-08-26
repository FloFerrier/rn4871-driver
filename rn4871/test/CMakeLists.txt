set(CMAKE_C_EXTENSIONS ON)
include(cmake/cmocka.cmake)

add_subdirectory(unit_test)
add_subdirectory(interface_test)