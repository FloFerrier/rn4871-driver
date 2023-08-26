cmake_minimum_required(VERSION 3.20)
project(RN4871-driver)

set(CMAKE_C_STANDARD 99)
enable_language(C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(LOGS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/logs)

if(CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL Release)
    add_subdirectory(rn4871)
elseif(CMAKE_BUILD_TYPE STREQUAL Test)
    include(CTest)
    add_subdirectory(rn4871/test)
else()
	message(ERROR " Specify CMAKE_BUILD_TYPE Release or Test")
endif()