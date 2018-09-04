#-------------------------------------------------
#
# Project created by QtCreator 2018-09-04T10:36:15
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtXiangqi
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    serverdialog.cpp \
    clientdialog.cpp

HEADERS  += mainwindow.h \
    serverdialog.h \
    clientdialog.h

FORMS    += mainwindow.ui \
    serverdialog.ui \
    clientdialog.ui
