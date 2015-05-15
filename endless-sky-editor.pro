#-------------------------------------------------
#
# Project created by QtCreator 2015-04-30T20:16:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = endless-sky-editor
TEMPLATE = app
CONFIG += c++11

target.path = /usr/games/
INSTALLS += target

desktop.path = /usr/share/applications/
desktop.files = endless-sky-editor.desktop
INSTALLS += desktop

manual.path = /usr/share/man/man6/
manual.files = endless-sky-editor.6
INSTALLS += manual

icon.path = /usr/share/icons/hicolor/48x48/apps/
icon.files = endless-sky-editor.png
INSTALLS += icon


SOURCES += main.cpp\
		DataFile.cpp\
		DataNode.cpp\
		DataWriter.cpp\
        MainWindow.cpp\
        Planet.cpp\
        StellarObject.cpp\
        System.cpp \
    SystemView.cpp \
    Map.cpp \
    SpriteSet.cpp \
    GalaxyView.cpp \
    Galaxy.cpp \
    DetailView.cpp \
    AsteroidField.cpp \
    PlanetView.cpp \
    LandscapeView.cpp \
    LandscapeLoader.cpp

HEADERS  += DataFile.h\
		DataNode.h\
		DataWriter.h\
        MainWindow.h\
        Planet.h\
        StellarObject.h\
        System.h \
    SystemView.h \
    Map.h \
    SpriteSet.h \
    GalaxyView.h \
    Galaxy.h \
    DetailView.h \
    AsteroidField.h \
    PlanetView.h \
    LandscapeView.h \
    LandscapeLoader.h
