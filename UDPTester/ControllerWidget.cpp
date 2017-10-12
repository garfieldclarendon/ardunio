#include <QQmlEngine>
#include <QQmlContext>

#include "ControllerWidget.h"
#include "UI.h"

ControllerWidget::ControllerWidget(const QString &urlText, QWidget *parent)
    : QQuickWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    UI *ui = new UI(this);
    engine()->rootContext()->setContextProperty("ui", ui);

    setSource(QUrl(urlText));
}

