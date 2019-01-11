#ifndef LABELPAINTER_H
#define LABELPAINTER_H

#include <QObject>
#include <QFont>

class EntityModel;
class QPrinter;
class QPainter;

class LabelPainter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QFont font READ getFont WRITE setFont NOTIFY fontChanged)

public:
    explicit LabelPainter(EntityModel *model, QObject *parent = nullptr);

    void setFont(const QFont &font);
    QFont getFont(void) const { return m_font; }

signals:
    void fontChanged();

public slots:

protected slots:
    virtual void printerPaintRequested(QPrinter *printer);

    virtual void paintHeader(QRectF &rect, QPainter *painter, const QFont &font) = 0;
    virtual void paintBody(QRectF &rect, QPainter *painter, const QFont &font) = 0;

protected:
    QString getDeviceTypeName(int deviceType);

    EntityModel *m_model;
    int m_padding;
    QFont m_font;
};

#endif // LABELPAINTER_H
