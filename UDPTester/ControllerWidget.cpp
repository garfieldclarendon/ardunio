#include <QQmlEngine>
#include <QQmlContext>

#include "ControllerWidget.h"
#include "MessageBroadcaster.h"
#include "UI.h"

ControllerWidget::ControllerWidget(QWidget *parent)
    : QQuickWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    engine()->rootContext()->setContextProperty("broadcaster", MessageBroadcaster::instance());
    UI *ui = new UI(this);
    engine()->rootContext()->setContextProperty("ui", ui);

    setSource(QUrl("qrc:/ManageControllers.qml"));
}

