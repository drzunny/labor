CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(labor)

# setup the source
set(labor_source "src/labor_connect.cc" "src/labor_event.cc" "src/labor_log.cc"
                 "src/labor_lvm.cc" "src/labor_pvm.cc" "src/labor_opt.cc"
                 "src/labor_request.cc" "src/labor_response.cc" "src/labor_service.cc"
                 "src/labor_utils.cc" "src/labor_main.cc")


if(WIN32)
    include_directories("${PROJECT_SOURCE_DIR}/depends/include" "${PROJECT_SOURCE_DIR}/depends/include/pthread_win" "$ENV{PYTHON_HOME}/Include")
    link_directories("${PROJECT_SOURCE_DIR}/depends/win32_lib" "$ENV{PYTHON_HOME}/libs")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
    set(labor_dbg_depends libzmq_d.lib pthread_d.lib python27_d.lib lua_d.lib)
    set(labor_rel_depends libzmq.lib pthread.lib python27.lib lua.lib)

    # set debug's options
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLABOR_DEBUG /MTd")
    # set release options
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

else()
    include_directories("${PROJECT_SOURCE_DIR}/depends/include")
    set(labor_dbg_depends zmq pthread python27 lua)
    set(labor_rel_depends ${labor_dbg_depends})
endif(WIN32)

# output
add_executable(labor ${labor_source})
target_link_libraries(labor debug ${labor_dbg_depends})
target_link_libraries(labor optimized ${labor_rel_depends})
