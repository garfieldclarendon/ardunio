#Header files
HEADERS += $$PWD/AppService.h \
            $$PWD/Database.h \
            $$PWD/WebServer.h \
            $$PWD/MessageBroadcaster.h \
    $$PWD/DeviceManager.h \
    $$PWD/ControllerManager.h \
    $$PWD/DeviceHandler.h \
    $$PWD/WebServerThread.h \
    $$PWD/NotificationServer.h \
    $$PWD/NCEInterface.h \
    $$PWD/NCEMessage.h \
    $$PWD/EntityMetadata.h \
    $$PWD/Simulator.h \
    $$PWD/../libraries/shared/UDPMessage.h \
    $$PWD/APIEntity.h \
    $$PWD/APIController.h \
    $$PWD/APIRequest.h \
    $$PWD/APIResponse.h \
    $$PWD/APITurnout.h \
    $$PWD/APIDevice.h \
    $$PWD/APIRoute.h \
    $$PWD/APISignal.h

#Source files
SOURCES += $$PWD/main.cpp \
           $$PWD/AppService.cpp \
           $$PWD/Database.cpp \
           $$PWD/WebServer.cpp \
           $$PWD/MessageBroadcaster.cpp \
    $$PWD/DeviceManager.cpp \
    $$PWD/ControllerManager.cpp \
    $$PWD/DeviceHandler.cpp \
    $$PWD/NotificationServer.cpp \
    $$PWD/NCEInterface.cpp \
    $$PWD/NCEMessage.cpp \
    $$PWD/EntityMetadata.cpp \
    $$PWD/Simulator.cpp \
    $$PWD/../libraries/shared/UDPMessage.cpp \
    $$PWD/APIEntity.cpp \
    $$PWD/APIController.cpp \
    $$PWD/APIRequest.cpp \
    $$PWD/APIResponse.cpp \
    $$PWD/WebServerThread.cpp \
    $$PWD/APITurnout.cpp \
    $$PWD/APIDevice.cpp \
    $$PWD/APIRoute.cpp \
    $$PWD/APISignal.cpp


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
