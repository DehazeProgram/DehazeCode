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


win32:CONFIG(release, debug|release): LIBS += -LC:/OSGeo4W64/lib/ -lopencv_core248 -lopencv_highgui248 -lopencv_imgproc248
else:win32:CONFIG(debug, debug|release): LIBS += -L/C:/OSGeo4W64/lib/ -lopencv_core248d -lopencv_highgui248d
else:unix: LIBS += -LC:/OSGeo4W64/lib/ -lopencv_core -lopencv_highgui

INCLUDEPATH += C:/OSGeo4W64/include
DEPENDPATH += C:/OSGeo4W64/include
