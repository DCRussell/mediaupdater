
QT          += core
QT          -= gui
QT          += network

TEMPLATE = app
TARGET = lmmediaupdater

CONFIG   += console
CONFIG   -= app_bundle
CONFIG  += c++11


CONFIG(release, debug|release) {
    DEFINES += RELEASE_MODE
}

CONFIG(debug, debug|release) {
    DEFINES += DEBUG_MODE SEND_TO_DEBUG_LOG_MESSAGE
}


SOURCES += main.cpp \
    lmmediaupdaterservice.cpp \
    updatecontroller.cpp \
    downloadmanager.cpp \
    content.cpp \
    contentmanager.cpp \
    logininfo.cpp \
    checkdownloadedcontent.cpp \
    fileuploader.cpp \
    uploadmanager.cpp

HEADERS += \
    lmmediaupdaterservice.h \
    updatecontroller.h \
    downloadmanager.h \
    content.h \
    contentmanager.h \
    logininfo.h \
    checkdownloadedcontent.h \
    fileuploader.h \
    uploadmanager.h


include(../qtservice/src/qtservice.pri)
include(../qftp/qftp.pri)
include(../logger/logger.pri)
include(../eventer/eventer.pri)
include(../managetools/managetools.pri)

