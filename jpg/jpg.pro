TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    markers.cpp \
    reader.cpp \
    huffmancoding.cpp

HEADERS += \
    markers.h \
    reader.h \
    huffmancoding.h

