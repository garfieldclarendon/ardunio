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
    MessageMonitorWidget.cpp \
    TrackSchematic.cpp \
    UDPMessage.cpp \
    TrackTrunout.cpp \
    MessageBroadcaster.cpp \
    Database.cpp \
    TcpClientHandler.cpp \
    SystemMessageHandler.cpp \
    ControllerTab.cpp \
    DeviceTab.cpp \
    WebServer.cpp \
    UpdateServer.cpp \
    ControllerWidget.cpp \
    ControllerModel.cpp \
    TurnoutModel.cpp \
    PanelModuleModel.cpp \
    PanelInputModel.cpp \
    PanelOutputModel.cpp \
    UI.cpp \
    TcpServer.cpp \
    bonjourserviceregister.cpp \
    ControllerModuleModel.cpp

HEADERS  += MainWindow.h \
    MessageMonitorWidget.h \
    TrackSchematic.h \
    UDPMessage.h \
    TrackTrunout.h \
    MessageBroadcaster.h \
    Database.h \
    TcpClientHandler.h \
    SystemMessageHandler.h \
    ControllerTab.h \
    DeviceTab.h \
    WebServer.h \
    UpdateServer.h \
    ControllerWidget.h \
    ControllerModel.h \
    TurnoutModel.h \
    PanelModuleModel.h \
    PanelInputModel.h \
    PanelOutputModel.h \
    UI.h \
    TcpServer.h \
    BonjourRecord.h \
    bonjourserviceregister.h \
    ControllerModuleModel.h

RESOURCES += \
    Resources.qrc

win32 {
    LIBS+=-ldnssd
    # Add your path to bonjour here.
    LIBPATH = "C:/Program Files/Bonjour SDK/Lib/Win32"
    INCLUDEPATH += "C:/Program Files/Bonjour SDK/Include"
}
