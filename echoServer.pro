QT -= gui
QT += core network sql widgets

CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= windows

TARGET = db
DESTDIR = build

SOURCES += \
    clientwindow.cpp \
    main.cpp \
    mytcpserver.cpp \
    database.cpp
    clientwindow.cpp

HEADERS += \
    clientwindow.h \
    mytcpserver.h \
    database.h
    clientwindow.h

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
