set(CMAKE_C_EXTENSIONS ON)
include(CTest)
include(cmake/cmocka.cmake)

add_subdirectory(Driver_Rn4871)
add_subdirectory(Virtual_Module)
add_subdirectory(Integration)