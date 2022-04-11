QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

QMAKE_CXXFLAGS_RELEASE += /MT
QMAKE_CXXFLAGS_DEBUG += /MT

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES -= QT_NO_DEBUG_OUTPUT

SOURCES += \
    file.cpp \
    main.cpp \
    mainwindow.cpp \
    zoom.cpp

HEADERS += \
    error.h \
    file.h \
    mainwindow.h \
    zoom.h

FORMS += \
    mainwindow.ui

LIBS += -LD:/OpenCV4/x64/vc17/lib/ -lopencv_img_hash455
LIBS += -LD:/OpenCV4/x64/vc17/lib/ -lopencv_world455

INCLUDEPATH += D:/OpenCV4/include
DEPENDPATH += D:/OpenCV4/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
