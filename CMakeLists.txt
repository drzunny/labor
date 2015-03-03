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

    # try to link python27_d if Debug. otherwise will link python27
    # if python27_d does not found in your environment, copy python27.lib to python27_d.lib is OK
    set(labor_depends libzmq.lib pthread.lib python27_d.lib python27.lib lua.lib)

    # set debug's options
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLABOR_DEBUG /MTd")
    # set release options
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

    # Copy the DLL file to Path
    message("labor >> Copy the Windows dependece")
    file(GLOB labor_DEPS_DLL ${PROJECT_SOURCE_DIR}/depends/win32_lib/*.dll)
    file(COPY ${labor_DEPS_DLL} DESTINATION ${PROJECT_BINARY_DIR}/Release)
    file(GLOB labor_RM_DLL ${PROJECT_BINARY_DIR}/Release/*_d.*)
    file(REMOVE ${labor_RM_DLL})

    # Copy the service
    message("labor >> Copy the Serivce file to destination directory")
    file(COPY ${PROJECT_SOURCE_DIR}/files/labor.conf DESTINATION ${PROJECT_BINARY_DIR}/Debug)
    file(COPY ${PROJECT_SOURCE_DIR}/files/services DESTINATION ${PROJECT_BINARY_DIR}/Debug)
    file(COPY ${PROJECT_SOURCE_DIR}/files/labor.conf DESTINATION ${PROJECT_BINARY_DIR}/Release)
    file(COPY ${PROJECT_SOURCE_DIR}/files/services DESTINATION ${PROJECT_BINARY_DIR}/Release)

else()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLABOR_DEBUG -Wall -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -s")

    include_directories("${PROJECT_SOURCE_DIR}/depends/include" "/usr/include/python2.7/")
    set(labor_depends zmq pthread python2.7 lua5.1)

    # Copy the service
    message("labor >> Copy the Serivce file to destination directory")
    file(COPY ${PROJECT_SOURCE_DIR}/files/labor.conf DESTINATION ${PROJECT_BINARY_DIR})
    file(COPY ${PROJECT_SOURCE_DIR}/files/services DESTINATION ${PROJECT_BINARY_DIR})
endif(WIN32)

# output
add_executable(labor ${labor_source})
target_link_libraries(labor ${labor_depends})

# copy the test files
file(COPY ${PROJECT_SOURCE_DIR}/test DESTINATION ${PROJECT_BINARY_DIR})

