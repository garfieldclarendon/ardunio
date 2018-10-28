#ifndef LABELPAINTER_H
#define LABELPAINTER_H

#include <QObject>
class EntityModel;
class QPrinter;
class QPainter;

class LabelPainter : public QObject
{
    Q_OBJECT
public:
    explicit LabelPainter(EntityModel *model, QObject *parent = nullptr);

signals:

public slots:

protected slots:
    virtual void printerPaintRequested(QPrinter *printer);

    virtual void paintHeader(QRectF &rect, QPainter *painter) = 0;
    virtual void paintBody(QRectF &rect, QPainter *painter) = 0;

protected:
    QString getDeviceTypeName(int deviceType);

    EntityModel *m_model;
    int m_padding;
};

#endif // LABELPAINTER_H
