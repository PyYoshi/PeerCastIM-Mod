#-------------------------------------------------
#
# Project created by QtCreator 2014-12-04T22:38:37
#
#-------------------------------------------------

QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PeerCast
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp

HEADERS  += mainwindow.h


FORMS    += mainwindow.ui

################# PeerCast IM Common #################
unix {
 QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter
 LIBS +=
 DEFINES += _UNIX
 SOURCES += ../../core/unix/compat.cpp ../../core/unix/usys.cpp ../../core/unix/usocket.cpp
 HEADERS += ../../core/unix/compat.h \
    ../../core/unix/ts_vector.h \
    ../../core/unix/usocket.h \
    ../../core/unix/usys.h \
}

unix:!macx { # Ubuntu or Debian or etc...
 QMAKE_CXXFLAGS += -D OS_LINUX
}

unix:macx { # OSX

 QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
 QMAKE_MAC_SDK = macosx10.9
 LIBS += -framework IOKit -framework CoreFoundation
 DEFINES += _APPLE __APPLE__
 QMAKE_CXXFLAGS += -D OS_MACOSX
 ICON = peercast.icns
 TARGET = PeerCast

 system(mkdir -p PeerCast.app/Contents/MacOS)
 system(cp -r ../html PeerCast.app/Contents/MacOS)
 system(cp peercast.xpm PeerCast.app/Contents/MacOS)
}

win32 {
 LIBS += -lwsock32
 DEFINES += WIN32 QT
 SOURCES += ../../core/win32/seh.cpp \
    ../../core/win32/wsys.cpp \
    ../../core/win32/wsocket.cpp
 HEADERS += ../../core/win32/seh.h \
    ../../core/win32/ts_vector.h \
    ../../core/win32/wsocket.h \
    ../../core/win32/wsys.h
}

#DEPENDPATH += .
HEADERS += ../../core/common/addrCont.h \
    ../../core/common/asf.h \
    ../../core/common/atom.h \
    ../../core/common/channel.h \
    ../../core/common/common.h \
    ../../core/common/cstream.h \
    ../../core/common/gnutella.h \
    ../../core/common/html-xml.h \
    ../../core/common/html.h \
    ../../core/common/http.h \
    ../../core/common/icy.h \
    ../../core/common/id.h \
    ../../core/common/identify_encoding.h \
    ../../core/common/inifile.h \
    ../../core/common/jis.h \
    ../../core/common/mms.h \
    ../../core/common/mp3.h \
    ../../core/common/nsv.h \
    ../../core/common/ogg.h \
    ../../core/common/pcp.h \
    ../../core/common/peercast.h \
    ../../core/common/rtsp.h \
    ../../core/common/servent.h \
    ../../core/common/servmgr.h \
    ../../core/common/socket.h \
    ../../core/common/stats.h \
    ../../core/common/stream.h \
    ../../core/common/sys.h \
    ../../core/common/ts_vector.h \
    ../../core/common/url.h \
    ../../core/common/utf8.h \
    ../../core/common/version2.h \
    ../../core/common/xml.h

INCLUDEPATH += . ../../core \
    ../../core/common

SOURCES += ../../core/common/socket.cpp \
    ../../core/common/servent.cpp \
    ../../core/common/servhs.cpp \
    ../../core/common/servmgr.cpp \
    ../../core/common/xml.cpp \
    ../../core/common/stream.cpp \
    ../../core/common/sys.cpp \
    ../../core/common/gnutella.cpp \
    ../../core/common/html.cpp \
    ../../core/common/channel.cpp \
    ../../core/common/http.cpp \
    ../../core/common/inifile.cpp \
    ../../core/common/peercast.cpp \
    ../../core/common/stats.cpp \
    ../../core/common/mms.cpp \
    ../../core/common/mp3.cpp \
    ../../core/common/nsv.cpp \
    ../../core/common/ogg.cpp \
    ../../core/common/url.cpp \
    ../../core/common/icy.cpp \
    ../../core/common/pcp.cpp \
    ../../core/common/jis.cpp
