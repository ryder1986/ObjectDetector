unix{
    DEFINES+=IS_UNIX
    LIBS+=-lpthread
    QMAKE_LFLAGS+=-Wl,--rpath=/root/source/opencv-3.2.0/build/__install/lib
    INCLUDEPATH+=/root/source/opencv-3.2.0/build/__install/include
    LIBS+=-L/root/source/opencv-3.2.0/build/__install/lib
    LIBS+=-L/root/source/ffmpeg-3.4.4/__install/lib
    QMAKE_CFLAGS+=-w
}else{
    DEFINES+=IS_WIN
}
INCLUDEPATH+=cppjson/include


TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    app.cpp configmanager.cpp \
    camera.cpp tool.cpp server.cpp \
    socket.cpp videosource.cpp \
    detectregion.cpp videoprocessor.cpp \
    mvdprocessor.cpp \
    dummyprocessor.cpp jsonpacket.cpp

HEADERS +=\
    cppjson/include/json/reader.h\
    cppjson/include/json/writer.h\
    cppjson/include/json/value.h \
    app.h configmanager.h camera.h tool.h server.h socket.h \
    videosource.h detectregion.h videoprocessor.h \
    mvdprocessor.h \
    dummyprocessor.h

SOURCES +=\
    cppjson/json_reader.cpp \
    cppjson/json_writer.cpp \
    cppjson/json_value.cpp

LIBS+=-lopencv_core
LIBS+=-lopencv_imgcodecs
LIBS+=-lopencv_highgui
LIBS+=-lopencv_ml
LIBS+=-lopencv_videoio
LIBS+=-lopencv_imgproc

LIBS+=-lswresample

install_files.files+=res/
install_files.path=$$OUT_PWD/
INSTALLS+=install_files
