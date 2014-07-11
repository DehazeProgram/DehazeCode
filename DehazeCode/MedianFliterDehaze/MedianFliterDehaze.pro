QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE =lib

CONFIG +=staticlib

TARGET = medianfliterdehazor

HEADERS += medianfliterdehazor.h
		 

SOURCES += medianfliterdehazor.cpp
		
DESTDIR = ../../lib/Dehazor

INCLUDEPATH += ../../BasicProcessing


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/BasicProcessing/ -lbasicprocessing
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/BasicProcessing/ -lbasicprocessing
else:unix: LIBS += -L$$OUT_PWD/../lib/BasicProcessing/ -lbasicprocessing

INCLUDEPATH += $$PWD/../../BasicProcessing
DEPENDPATH += $$PWD/../../BasicProcessing


win32:CONFIG(release, debug|release):LIBS += -LC:/OSGeo4W64/lib/ -lopencv_core248 -lopencv_highgui248 -lopencv_gpu248 -lopencv_imgproc248 -lopencv_features2d248 -lopencv_photo248
else:win32:CONFIG(debug, debug|release):LIBS += -L/C:/OSGeo4W64/lib/ -lopencv_core248d -lopencv_highgui248d
else:unix: LIBS += -LC:/OSGeo4W64/lib/ -lopencv_core -lopencv_highgui