#include <QFont>
#include <QPainter>

#include "LabelTurnout.h"
#include "EntityModel.h"

LabelTurnout::LabelTurnout(EntityModel *model, QObject *parent)
    : LabelPainter(model, parent)
{

}

void LabelTurnout::paintHeader(QRectF &rect, QPainter *painter)
{
    QRectF size;

    QPen pen = painter->pen();

    QFont font = painter->font();
    font.setPointSize(16);
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(rect, 0, "Turnout Module", &size);
    rect.setTop(rect.top() + size.height() + m_padding);
    font.setBold(false);
    painter->setFont(font);
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

void LabelTurnout::paintBody(QRectF &rect, QPainter *painter)
{
    QRectF size;

    QPen pen = painter->pen();

    for(int x = m_model->rowCount() - 1; x >= 0; x--)
    {
        QFont font = painter->font();
        font.setPointSize(16);
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(rect, 0, m_model->data(x, "deviceName").toString(), &size);
        rect.setTop(rect.top() + size.height() + m_padding);

        font.setBold(false);
        painter->setFont(font);
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
