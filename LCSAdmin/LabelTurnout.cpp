#include <QFont>
#include <QPainter>

#include "LabelTurnout.h"
#include "EntityModel.h"

LabelTurnout::LabelTurnout(EntityModel *model, QObject *parent)
    : LabelPainter(model, parent)
{

}

void LabelTurnout::paintHeader(QRectF &rect, QPainter *painter, const QFont &font)
{
    QRectF size;

    QPen pen = painter->pen();

    QFont f(font);
    f.setPointSize(font.pointSize());
    f.setBold(true);
    painter->setFont(f);
    painter->drawText(rect, 0, m_model->data(0, "moduleName").toString(), &size);
    rect.setTop(rect.top() + size.height() + m_padding);
    f.setBold(false);
    painter->setFont(f);
    QString address = QString("Address: %1").arg(m_model->data(0, "address").toInt());
    painter->drawText(rect, 0, address, &size);
    rect.setTop(rect.top() + size.height() + (m_padding * 5));
    pen.setWidth(3);
    painter->setPen(pen);
    painter->drawLine(rect.topLeft(), rect.topRight());
    pen.setWidth(1);
    painter->setPen(pen);
    rect.setTop(rect.top() + (m_padding * 10));
}

void LabelTurnout::paintBody(QRectF &rect, QPainter *painter, const QFont &font)
{
    QRectF size;

    QPen pen = painter->pen();

    QFont f(font);
    for(int x = m_model->rowCount() - 1; x >= 0; x--)
    {
        f.setPointSize(font.pointSize());
        f.setBold(true);
        painter->setFont(f);
        painter->drawText(rect, 0, m_model->data(x, "deviceName").toString(), &size);
        rect.setTop(rect.top() + size.height() + m_padding);

        f.setBold(false);
        painter->setFont(f);
        if(x == 1)
            painter->drawText(rect, 0, QString("Port: %1 (top)").arg(m_model->data(x, "port").toString()), &size);
        else
            painter->drawText(rect, 0, QString("Port: %1 (bottom)").arg(m_model->data(x, "port").toString()), &size);
        rect.setTop(rect.top() + size.height() + m_padding);

        if(x == m_model->rowCount() - 1)
        {
            rect.setTop(rect.top() + size.height() + (m_padding * 3));
            pen.setWidth(3);
            painter->setPen(pen);
            painter->drawLine(rect.topLeft(), rect.topRight());
            pen.setWidth(1);
            painter->setPen(pen);
            rect.setTop(rect.top() + (m_padding * 3));
        }
    }
}
