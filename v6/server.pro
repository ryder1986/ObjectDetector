#DEFINES+=IS_CLIENT
DEFINES+=IS_SERVER


HEADERS += \
    tool.h \
    jsonpacket.h \
    configmanager.h \
    app.h

SOURCES += \
    tool.cpp \
    jsonpacket.cpp \
    main.cpp \
    configmanager.cpp \
    app.cpp
CONFIG+=c++11
unix{
    DEFINES+=IS_UNIX
}
win32{
    DEFINES+=IS_WIN32
}

DISTFILES += \
    res/config.json



include(common.pri)
