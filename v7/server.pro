#TEMPLATE = app
#CONFIG += console c++11
#CONFIG -= app_bundle
#CONFIG -= qt
    QMAKE_LFLAGS+=-Wl,--rpath=/root/source/opencv-3.2.0/build/__install/lib
    INCLUDEPATH+=/root/source/opencv-3.2.0/build/__install/include
    LIBS+=-L/root/source/opencv-3.2.0/build/__install/lib

HEADERS += \
    tool.h \
    configmanager.h \
    jsonpacket.h \
    test.h \
    videosource.h

SOURCES += \
    tool.cpp \
    configmanager.cpp \
    main.cpp \
    jsonpacket.cpp \
    videosource.cpp
DEFINES+=IS_UNIX
CONFIG+=c++11
include(common.pri)
install_files.files+=res/
install_files.path=$$OUT_PWD/

INSTALLS+=install_files
