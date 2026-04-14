QT -= gui
QT += core network sql

CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= windows

TARGET = db
DESTDIR = build

SOURCES += \
    main.cpp \
    mytcpserver.cpp \
    database.cpp

HEADERS += \
    mytcpserver.h \
    database.h

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target