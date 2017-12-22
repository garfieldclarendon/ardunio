#Header files
HEADERS += $$PWD/AppService.h \
            $$PWD/Database.h \
            $$PWD/WebServer.h \
            $$PWD/MessageBroadcaster.h \
    $$PWD/ControllerHandler.h \
    $$PWD/TurnoutHandler.h \
    $$PWD/DeviceManager.h \
    $$PWD/ControllerManager.h \
    $$PWD/APIHandler.h \
    $$PWD/DeviceHandler.h \
    $$PWD/RouteHandler.h \
    $$PWD/PanelHandler.h \
    $$PWD/WebServerThread.h \
    $$PWD/SemaphoreHandler.h \
    $$PWD/SignalHandler.h \
    $$PWD/BlockHandler.h \
    $$PWD/NotificationServer.h \
    $$PWD/NCEInterface.h \
    $$PWD/NCEMessage.h \
    $$PWD/controllermessage.h \
    $$PWD/EntityMetadata.h \
    $$PWD/Simulator.h \
    $$PWD/../libraries/shared/UDPMessage.h

#Source files
SOURCES += $$PWD/main.cpp \
           $$PWD/AppService.cpp \
           $$PWD/Database.cpp \
           $$PWD/WebServer.cpp \
           $$PWD/MessageBroadcaster.cpp \
    $$PWD/ControllerHandler.cpp \
    $$PWD/TurnoutHandler.cpp \
    $$PWD/DeviceManager.cpp \
    $$PWD/ControllerManager.cpp \
    $$PWD/APIHandler.cpp \
    $$PWD/DeviceHandler.cpp \
    $$PWD/RouteHandler.cpp \
    $$PWD/PanelHandler.cpp \
    $$PWD/WebServerThread.cpp \
    $$PWD/SemaphoreHandler.cpp \
    $$PWD/SignalHandler.cpp \
    $$PWD/BlockHandler.cpp \
    $$PWD/NotificationServer.cpp \
    $$PWD/NCEInterface.cpp \
    $$PWD/NCEMessage.cpp \
    $$PWD/controllermessage.cpp \
    $$PWD/EntityMetadata.cpp \
    $$PWD/Simulator.cpp \
    $$PWD/../libraries/shared/UDPMessage.cpp

win32 {
HEADERS += $$PWD/StatusDialog.h
SOURCES += $$PWD/StatusDialog.cpp
}

linux {
HEADERS += $$PWD/PIGPIO.h
SOURCES += $$PWD/PIGPIO.cpp
}
FORMS += \
    $$PWD/StatusDialog.ui

DISTFILES +=
