QT -= gui
QT += core sql network gui

CONFIG += c++17 console
CONFIG -= app_bundle

SOURCES = main.cpp database.cpp mytcpserver.cpp
HEADERS = database.h mytcpserver.h

TARGET = echoServer
