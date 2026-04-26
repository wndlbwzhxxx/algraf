QT += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_NO_OPENGL

SOURCES = \
    main.cpp \
    mainwindow.cpp \
    graphwidget.cpp

HEADERS = \
    mainwindow.h \
    graphwidget.h

unix:!macx {
    QMAKE_LIBS_OPENGL =
    LIBS -= -lGL
}
