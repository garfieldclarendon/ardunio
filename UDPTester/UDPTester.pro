#-------------------------------------------------
#
# Project created by QtCreator 2016-08-09T19:20:30
#
#-------------------------------------------------

QT       += core gui network sql websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets quickwidgets

TARGET = UDPTester
TEMPLATE = app

INCLUDEPATH += "../libraries/shared"

SOURCES += main.cpp\
        MainWindow.cpp \
    TrackSchematic.cpp \
    TrackTrunout.cpp \
    ControllerTab.cpp \
    DeviceTab.cpp \
    ControllerWidget.cpp \
    ControllerModel.cpp \
    TurnoutModel.cpp \
    PanelModuleModel.cpp \
    PanelInputModel.cpp \
    PanelOutputModel.cpp \
    UI.cpp \
    ControllerModuleModel.cpp \
    ControllerModuleWidget.cpp \
    JSonModel.cpp \
    API.cpp \
    ../ControlServer/UDPMessage.cpp \
    ../ControlServer/MessageBroadcaster.cpp

HEADERS  += MainWindow.h \
    TrackSchematic.h \
    TrackTrunout.h \
    ControllerTab.h \
    DeviceTab.h \
    ControllerWidget.h \
    ControllerModel.h \
    TurnoutModel.h \
    PanelModuleModel.h \
    PanelInputModel.h \
    PanelOutputModel.h \
    UI.h \
    ControllerModuleModel.h \
    ControllerModuleWidget.h \
    JSonModel.h \
    API.h \
    ../ControlServer/UDPMessage.h \
    ../ControlServer/MessageBroadcaster.h

RESOURCES += \
    Resources.qrc

win32 {
    LIBS+=-ldnssd
    # Add your path to bonjour here.
    LIBPATH = "C:/Program Files/Bonjour SDK/Lib/Win32"
    INCLUDEPATH += "C:/Program Files/Bonjour SDK/Include"
}
