TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle

CONFIG += qt QtCore

INCLUDEPATH += /usr/local/include
LIBS += -L/usr/local/lib/ -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_viz

HEADERS += \
    dsvlprocessor.h \
    samplegenerator.h \
    types.h

SOURCES += \
    dsvlprocessor.cpp \
    main.cpp \
    samplegenerator.cpp
