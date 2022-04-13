QT       += core gui opengl openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

CONFIG += c++17

QMAKE_CXXFLAGS += /MT

DEFINES += GOOGLE_GLOG_DLL_DECL GLOG_NO_ABBREVIATED_SEVERITIES

SOURCES += \
    file.cpp \
    imageview.cpp \
    main.cpp \
    mainwindow.cpp \
    mat2qimage.cpp \
    zoom.cpp

HEADERS += \
    error.h \
    file.h \
    imageview.h \
    mainwindow.h \
    mat2qimage.h \
    zoom.h

FORMS += \
    mainwindow.ui

LIBS += -L$$PWD/libs/ -lopencv_img_hash455
LIBS += -L$$PWD/libs/ -lopencv_world455


INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
