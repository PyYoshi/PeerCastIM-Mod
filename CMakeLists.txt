cmake_minimum_required(VERSION 2.8.4)
project(peercast)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

include_directories(core/ core/common/)
set(SOURCE_FILES
    core/common/channel.cpp
    core/common/gnutella.cpp
    core/common/html.cpp
    core/common/http.cpp
    core/common/icy.cpp
    core/common/identify_encoding.c
    core/common/inifile.cpp
    core/common/jis.cpp
    core/common/mms.cpp
    core/common/mp3.cpp
    core/common/nsv.cpp
    core/common/ogg.cpp
    core/common/pcp.cpp
    core/common/peercast.cpp
    core/common/rtsp.cpp
    core/common/servent.cpp
    core/common/servhs.cpp
    core/common/servmgr.cpp
    core/common/socket.cpp
    core/common/stats.cpp
    core/common/stream.cpp
    core/common/sys.cpp
    core/common/url.cpp
    core/common/xml.cpp
    core/unix/compat.cpp
    core/unix/usocket.cpp
    core/unix/usys.cpp)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(SOURCE_FILES ${SOURCE_FILES} ui/osx/main.cpp)
endif (CMAKE_SYSTEM_NAME MATCHES "Darwin")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(SOURCE_FILES ${SOURCE_FILES} ui/linux/main.cpp)
endif (CMAKE_SYSTEM_NAME MATCHES "Linux")

add_executable(peercast ${SOURCE_FILES})

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__cplusplus -D_UNIX -D_APPLE -D_REENTRANT -DOS_MACOSX")
    find_library(corefoundation_lib CoreFoundation)
    set(frameworks ${corefoundation_lib})
    target_link_libraries(peercast ${frameworks})
endif (CMAKE_SYSTEM_NAME MATCHES "Darwin")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_BINRELOC -pthread  -D__cplusplus -D_UNIX -D_REENTRANT -DOS_LINUX")
endif (CMAKE_SYSTEM_NAME MATCHES "Linux")
