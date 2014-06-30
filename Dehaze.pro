TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    dehazor.cpp \
    filter.cpp \
    colorcorrect.cpp

HEADERS += \
    dehazor.h \
    filter.h \
    colorcorrect.h

unix|win32: LIBS += -lopencv_core246
unix|win32: LIBS += -lopencv_highgui246
unix|win32: LIBS += -lopencv_imgproc246
