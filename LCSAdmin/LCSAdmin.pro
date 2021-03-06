QT += quick websockets gui printsupport
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += "../libraries/shared"

HEADERS += API.h \
           DeviceModel.h \
           JSonModel.h \
           UI.h \
    ../libraries/shared/UDPMessage.h \
    ../LCSServer/MessageBroadcaster.h \
    Entity.h \
    SignalAspectModel.h \
    EntityModel.h \
    SignalConditionModel.h \
    ControllerModel.h \
    ControllerModuleModel.h \
    RouteModel.h \
    RouteEntryModel.h \
    DevicePropertyModel.h \
    TrackSegment.h \
    TrackSingle.h \
    TrackSiding.h \
    TrackTurnout.h \
    LabelPainter.h \
    LabelTurnout.h \
    LabelInput.h \
    LabelOutput.h \
    LabelController.h \
    DeviceModuleModel.h \
    ReportController.h

SOURCES += main.cpp \
           DeviceModel.cpp \
           API.cpp \
           JSonModel.cpp \
           UI.cpp \
    ../libraries/shared/UDPMessage.cpp \
            ../LCSServer/MessageBroadcaster.cpp \
    Entity.cpp \
    SignalAspectModel.cpp \
    EntityModel.cpp \
    SignalConditionModel.cpp \
    ControllerModel.cpp \
    ControllerModuleModel.cpp \
    RouteModel.cpp \
    RouteEntryModel.cpp \
    DevicePropertyModel.cpp \
    TrackSegment.cpp \
    TrackSingle.cpp \
    TrackSiding.cpp \
    TrackTurnout.cpp \
    LabelPainter.cpp \
    LabelTurnout.cpp \
    LabelInput.cpp \
    LabelOutput.cpp \
    LabelController.cpp \
    DeviceModuleModel.cpp \
    ReportController.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

RC_ICONS = LCSAdmin.ico
