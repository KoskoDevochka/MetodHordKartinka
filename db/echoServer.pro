QT -= gui
QT += network sql

CONFIG += c++11 console
CONFIG -= app_bundle

# Определяем ОС для путей
win32 {
    DEFINES += Q_OS_WIN
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mytcpserver.cpp \
    database.cpp

HEADERS += \
    mytcpserver.h \
    database.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target