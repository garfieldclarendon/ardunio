#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFontMetrics>

#include "DeviceModel.h"
#include "DevicePropertyModel.h"
#include "DeviceModuleModel.h"
#include "SignalAspectModel.h"
#include "SignalConditionModel.h"
#include "ControllerModel.h"
#include "ControllerModuleModel.h"
#include "RouteEntryModel.h"
#include "RouteModel.h"
#include "UI.h"
#include "API.h"
#include "Entity.h"
#include "TrackSingle.h"
#include "TrackSiding.h"
#include "TrackTurnout.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    QApplication app(argc, argv);

    app.setOrganizationName("GCMRR");
    app.setOrganizationDomain("garfieldcentral.org");
    app.setApplicationName("LCS Admin");

    qRegisterMetaType<Entity>();
    qRegisterMetaType<ControllerStatusEnum>("ControllerStatusEnum");
    qRegisterMetaType<ControllerClassEnum>("ControllerClassEnum");
    qRegisterMetaType<DeviceClassEnum>("DeviceClassEnum");
    qRegisterMetaType<ModuleClassEnum>("ModuleClassEnum");
    qRegisterMetaType<NetActionType>("NetActionType");

    qmlRegisterType<DeviceModel>("Utils", 1, 0, "DeviceModel");
    qmlRegisterType<DevicePropertyModel>("Utils", 1, 0, "DevicePropertyModel");
    qmlRegisterType<DeviceModuleModel>("Utils", 1, 0, "DeviceModuleModel");
    qmlRegisterType<SignalAspectModel>("Utils", 1, 0, "SignalAspectModel");
    qmlRegisterType<SignalConditionModel>("Utils", 1, 0, "SignalConditionModel");
    qmlRegisterType<ControllerModel>("Utils", 1, 0, "ControllerModel");
    qmlRegisterType<ControllerModuleModel>("Utils", 1, 0, "ControllerModuleModel");
    qmlRegisterType<RouteModel>("Utils", 1, 0, "RouteModel");
    qmlRegisterType<RouteEntryModel>("Utils", 1, 0, "RouteEntryModel");

    qmlRegisterType<TrackSingle>("Track", 1, 0, "TrackSingle");
    qmlRegisterType<TrackSiding>("Track", 1, 0, "TrackSiding");
    qmlRegisterType<TrackTurnout>("Track", 1, 0, "TrackTurnout");

    QQmlApplicationEngine engine;
    UI ui;
    ui.init();

    if(ui.getBaseFontSize() <= 0)
    {
        qDebug("NO POINT SIZE!!!!!");
    }

    engine.rootContext()->setContextProperty("ui", &ui);
    engine.rootContext()->setContextProperty("api", API::instance());

    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
