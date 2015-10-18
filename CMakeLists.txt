CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(labor)

# setup the source
file(GLOB labor_source ${PROJECT_SOURCE_DIR}/src/*.cc)

set(PROJECT_BINARY_DIR ${PROJECT_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

if(WIN32)
    include_directories("${PROJECT_SOURCE_DIR}/depends/include" "${PROJECT_SOURCE_DIR}/depends/include/pthread_win" "$ENV{PYTHON_HOME}/Include")
    link_directories("${PROJECT_SOURCE_DIR}/depends/win32_lib" "$ENV{PYTHON_HOME}/libs")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

    # set debug's options
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLABOR_DEBUG /MTd")
    # set release options
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX /wd\"4100\" /wd\"4505\" /wd\"4127\" /wd\"4819\" /wd\"4189\"")

    # Copy the DLL file to Path
    message("labor >> Copy the Windows dependece")
    file(GLOB labor_DEPS_DLL ${PROJECT_SOURCE_DIR}/depends/win32_lib/*.dll)
    file(COPY ${labor_DEPS_DLL} DESTINATION ${PROJECT_BINARY_DIR})

    # Copy the service
    message("labor >> Copy the Serivce file to destination directory")
    file(COPY ${PROJECT_SOURCE_DIR}/files/labor.conf DESTINATION ${PROJECT_BINARY_DIR})
    file(COPY ${PROJECT_SOURCE_DIR}/files/services DESTINATION ${PROJECT_BINARY_DIR})

    # Set output path
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR})

    add_executable(labor ${labor_source})
    set_target_properties(labor PROPERTIES DEBUG_OUTPUT_NAME labor_d RELEASE_OUTPUT_NAME labor)
    target_link_libraries(labor debug "libzmq_d" debug "pthread_d" debug "python27_d" debug "lua_d")
    target_link_libraries(labor optimized "libzmq" optimized "pthread" optimized "python27" optimized "lua")

else()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLABOR_DEBUG -Wall -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -s")

    include_directories("${PROJECT_SOURCE_DIR}/depends/include" "/usr/include/python2.7/")

    # Copy the service
    message("labor >> Copy the Serivce file to destination directory")
    file(COPY ${PROJECT_SOURCE_DIR}/files/labor.conf DESTINATION ${PROJECT_BINARY_DIR})
    file(COPY ${PROJECT_SOURCE_DIR}/files/services DESTINATION ${PROJECT_BINARY_DIR})

    add_executable(labor ${labor_source})
    target_link_libraries(labor zmq pthread python2.7 lua5.1 dl util)

endif(WIN32)

# copy the test files
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/log)
file(COPY ${PROJECT_SOURCE_DIR}/test DESTINATION ${PROJECT_BINARY_DIR})
