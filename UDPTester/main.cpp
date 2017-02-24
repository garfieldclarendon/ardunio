#include "MainWindow.h"
#include <QApplication>
#include <QtQml>

#include "ControllerModel.h"
#include "TurnoutModel.h"
#include "PanelModuleModel.h"
#include "PanelInputModel.h"
#include "PanelOutputModel.h"
#include "ControllerModuleModel.h"

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication a(argc, argv);

    if(QApplication::font().pointSize() < 1)
    {
        QFont f = QApplication::font();
        f.setPointSize(12);
        QApplication::setFont(f);
    }
    qRegisterMetaType<UDPMessage>("UDPMessage");

    qmlRegisterType<ControllerModel>("Utils", 1, 0, "ControllerModel");
    qmlRegisterType<TurnoutModel>("Utils", 1, 0, "TurnoutModel");
    qmlRegisterType<PanelModuleModel>("Utils", 1, 0, "PanelModuleModel");
    qmlRegisterType<PanelInputModel>("Utils", 1, 0, "PanelInputModel");
    qmlRegisterType<PanelOutputModel>("Utils", 1, 0, "PanelOutputModel");
    qmlRegisterType<ControllerModuleModel>("Utils", 1, 0, "ControllerModuleModel");

    MainWindow w;
    w.show();

    return a.exec();
}
