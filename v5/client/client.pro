#-------------------------------------------------
#
# Project created by QtCreator 2018-09-29T07:58:15
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    playerwidget.cpp

HEADERS += \
        mainwindow.h \
    misc.h \
    playerwidget.h

FORMS += \
        mainwindow.ui
SERVER_PATH=../server

INCLUDEPATH+=$$SERVER_PATH
HEADERS+=$$SERVER_PATH/tool.h
SOURCES+=$$SERVER_PATH/tool.cpp
HEADERS+=$$SERVER_PATH/videosource.h
SOURCES+=$$SERVER_PATH/videosource.cpp
SOURCES+=$$SERVER_PATH/jsonpacket.cpp
HEADERS+=$$SERVER_PATH/jsonpacket.h
INCLUDEPATH+=$$SERVER_PATH/cppjson/include
SOURCES += $$SERVER_PATH/cppjson/json_reader.cpp \
$$SERVER_PATH/cppjson/json_writer.cpp \
$$SERVER_PATH/cppjson/json_value.cpp


win32{
    DEFINES+=IS_WIN
}
unix{
CONFIG+=c++11
    INCLUDEPATH+=/root/source/opencv-3.2.0/build/__install/include
    LIBS+=-L/root/source/opencv-3.2.0/build/__install/lib
    LIBS+=-L/root/source/ffmpeg-3.4.4/__install/lib
    QMAKE_LFLAGS+=-Wl,--rpath=/root/source/opencv-3.2.0/build/__install/lib
    QMAKE_LFLAGS+=-Wl,--rpath=/root/source/ffmpeg-3.4.4/__install/lib
    DEFINES+=IS_UNIX
    LIBS+=-lopencv_core
    LIBS+=-lopencv_video
    LIBS+=-lopencv_imgcodecs
    LIBS+=-lopencv_highgui
    LIBS+=-lopencv_ml
    LIBS+=-lopencv_videoio
    LIBS+=-lopencv_imgproc
    LIBS+=-lswresample
}else{
    message(build win32)
    INCLUDEPATH+=C:\opencv3.2\opencv\build\include
    debug{
    LIBS+=-LC:\opencv3.2\opencv\build\x64\vc14\lib -lopencv_world320d
    }
    release{
    LIBS+=-LC:\opencv3.2\opencv\build\x64\vc14\lib -lopencv_world320
    }

}



install_files.files+=res/
install_files.path=$$OUT_PWD/
INSTALLS+=install_files
