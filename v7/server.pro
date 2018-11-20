#TEMPLATE = app
#CONFIG += console c++11
#CONFIG -= app_bundle
#CONFIG -= qt


HEADERS += \
    tool.h \
    configmanager.h \
    jsonpacket.h \
    test.h

SOURCES += \
    tool.cpp \
    configmanager.cpp \
    main.cpp \
    jsonpacket.cpp
DEFINES+=IS_UNIX
CONFIG+=c++11
include(common.pri)
install_files.files+=res/
install_files.path=$$OUT_PWD/

INSTALLS+=install_files
