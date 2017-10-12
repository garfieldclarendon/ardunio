#ifndef CONTROLLERWIDGET_H
#define CONTROLLERWIDGET_H

#include <QQuickWidget>

class ControllerWidget : public QQuickWidget
{
    Q_OBJECT
public:
    ControllerWidget(const QString &urlText, QWidget *parent = NULL);

signals:

public slots:
};

#endif // CONTROLLERWIDGET_H
