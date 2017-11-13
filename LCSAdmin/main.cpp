#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFontMetrics>

#include "DeviceModel.h"
#include "SignalAspectModel.h"
#include "SignalConditionModel.h"
#include "ControllerModel.h"
#include "ControllerModuleModel.h"
#include "RouteEntryModel.h"
#include "RouteModel.h"
#include "UI.h"
#include "API.h"
#include "Entity.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    qRegisterMetaType<Entity>();

    qmlRegisterType<DeviceModel>("Utils", 1, 0, "DeviceModel");
    qmlRegisterType<SignalAspectModel>("Utils", 1, 0, "SignalAspectModel");
    qmlRegisterType<SignalConditionModel>("Utils", 1, 0, "SignalConditionModel");
    qmlRegisterType<ControllerModel>("Utils", 1, 0, "ControllerModel");
    qmlRegisterType<ControllerModuleModel>("Utils", 1, 0, "ControllerModuleModel");
    qmlRegisterType<RouteModel>("Utils", 1, 0, "RouteModel");
    qmlRegisterType<RouteEntryModel>("Utils", 1, 0, "RouteEntryModel");
//    qmlRegisterType<PanelModuleModel>("Utils", 1, 0, "PanelModuleModel");
//    qmlRegisterType<PanelInputModel>("Utils", 1, 0, "PanelInputModel");
//    qmlRegisterType<PanelOutputModel>("Utils", 1, 0, "PanelOutputModel");

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
