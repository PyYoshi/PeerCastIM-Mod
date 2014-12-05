cmake_minimum_required(VERSION 2.8.4)
project(peercast)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__cplusplus -D_UNIX -D_APPLE -D_REENTRANT -DOS_MACOSX")
endif (CMAKE_SYSTEM_NAME MATCHES "Darwin")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_BINRELOC -pthread  -D__cplusplus -D_UNIX -D_REENTRANT -DOS_LINUX")
endif (CMAKE_SYSTEM_NAME MATCHES "Linux")

include_directories(core/ core/common/)

set(SOURCE_FILES
    core/common/addrCont.h
    core/common/asf.h
    core/common/atom.h
    core/common/channel.cpp
    core/common/channel.h
    core/common/common.h
    core/common/cstream.h
    core/common/gnutella.cpp
    core/common/gnutella.h
    core/common/html-xml.h
    core/common/html.cpp
    core/common/html.h
    core/common/http.cpp
    core/common/http.h
    core/common/icy.cpp
    core/common/icy.h
    core/common/id.h
    core/common/identify_encoding.c
    core/common/identify_encoding.h
    core/common/inifile.cpp
    core/common/inifile.h
    core/common/jis.cpp
    core/common/jis.h
    core/common/mms.cpp
    core/common/mms.h
    core/common/mp3.cpp
    core/common/mp3.h
    core/common/nsv.cpp
    core/common/nsv.h
    core/common/ogg.cpp
    core/common/ogg.h
    core/common/pcp.cpp
    core/common/pcp.h
    core/common/peercast.cpp
    core/common/peercast.h
    core/common/rtsp.cpp
    core/common/rtsp.h
    core/common/servent.cpp
    core/common/servent.h
    core/common/servhs.cpp
    core/common/servmgr.cpp
    core/common/servmgr.h
    core/common/socket.cpp
    core/common/socket.h
    core/common/stats.cpp
    core/common/stats.h
    core/common/stream.cpp
    core/common/stream.h
    core/common/sys.cpp
    core/common/sys.h
    core/common/ts_vector.h
    core/common/url.cpp
    core/common/url.h
    core/common/utf8.c
    core/common/utf8.h
    core/common/version2.h
    core/common/xml.cpp
    core/common/xml.h
    core/unix/compat.cpp
    core/unix/compat.h
    core/unix/ts_vector.h
    core/unix/usocket.cpp
    core/unix/usocket.h
    core/unix/usys.cpp
    core/unix/usys.h
    ui/osx/main.cpp
    ui/linux/main.cpp)

add_executable(peercast ${SOURCE_FILES})