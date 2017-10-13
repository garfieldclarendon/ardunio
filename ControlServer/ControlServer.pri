#Header files
HEADERS += AppService.h \
            Database.h \
            WebServer.h \
            MessageBroadcaster.h \
            UDPMessage.h \
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
    $$PWD/message.h

#Source files
SOURCES += main.cpp \
           AppService.cpp \
           Database.cpp \
           WebServer.cpp \
           MessageBroadcaster.cpp \
           UDPMessage.cpp \
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
    $$PWD/message.cpp

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
