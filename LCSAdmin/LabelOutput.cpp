#include <QFont>
#include <QPainter>

#include "LabelOutput.h"
#include "EntityModel.h"
#include "GlobalDefs.h"

LabelOutput::LabelOutput(EntityModel *model, QObject *parent)
    : LabelPainter(model, parent)
{

}

void LabelOutput::paintHeader(QRectF &rect, QPainter *painter)
{
    QRectF size;

    QPen pen = painter->pen();

    QFont font = painter->font();
    font.setPointSize(16);
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(rect, 0, "Output Module", &size);
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

void LabelOutput::paintBody(QRectF &rect, QPainter *painter)
{
    QRectF size;

    QPen pen = painter->pen();
    QFont font = painter->font();
    font.setFamily("Courier New");
    font.setPointSize(10);

    font.setBold(false);
    painter->setFont(font);

    for(int x = 0; x < m_model->rowCount(); x++)
    {
        if(m_model->data(x, "deviceClass").toInt() == DeviceSignal)
        {
            paintSignal(rect, painter, x);
        }
        else
        {
            QString text = QString("%1 - %2").arg(m_model->data(x, "port").toString(), 2, QChar(' ')).arg(m_model->data(x, "deviceName").toString());
            painter->drawText(rect, 0, text, &size);
            rect.setTop(rect.top() + size.height());

            if(x != m_model->rowCount() - 1)
            {
                pen.setWidth(3);
                painter->setPen(pen);
                painter->drawLine(rect.topLeft(), rect.topRight());
                pen.setWidth(1);
                painter->setPen(pen);
                rect.setTop(rect.top() + m_padding);
            }
        }
    }
}

void LabelOutput::paintSignal(QRectF &rect, QPainter *painter, int row)
{
    QRectF size;
    int port = m_model->data(row, "port").toInt();
    for(int x = port; x < port + 3; x++)
    {
        QString text = QString("%1 - %2 ").arg(x, 2, 10, QChar(' ')).arg(m_model->data(row, "deviceName").toString());
        if(x == port)
            text += "(red)";
        else if(x == port + 1)
            text += "(green)";
        else if(x == port + 2)
            text += "(yellow)";

        painter->drawText(rect, 0, text, &size);
        rect.setTop(rect.top() + size.height());
        QPen pen = painter->pen();
        pen.setWidth(3);
        painter->setPen(pen);
        painter->drawLine(rect.topLeft(), rect.topRight());
        pen.setWidth(1);
        painter->setPen(pen);
        rect.setTop(rect.top() + m_padding);
    }
}
