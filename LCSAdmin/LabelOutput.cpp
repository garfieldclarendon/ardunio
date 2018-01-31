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
    font.setPointSize(12);
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
    QPen pen = painter->pen();
    QFont font = painter->font();
    font.setFamily("Courier New");
    font.setPointSize(10);

    font.setBold(false);
    painter->setFont(font);

    QString list[16];

    for(int x = 0; x < m_model->getRowCount(); x++)
    {
        int port = m_model->data(x, "port").toInt();
        QString text = m_model->data(x, "labelName").toString();
        if(text.trimmed().length() == 0)
            text = m_model->data(x, "deviceName").toString();
        if(m_model->data(x, "deviceClass").toInt() == DeviceSignal)
        {
            list[port] = text + "(red)";
            if(port + 1 < 16)
                list[port + 1] = text + "(green)";
            if(port + 2 < 16)
                list[port + 2] = text + "(yellow)";
        }
        else
        {
            list[port] = text;
        }
    }

    for(int x = 7; x >= 0; x--)
    {
        paintOutput(rect, painter, list[x], x);
    }

    pen.setWidth(3);
    painter->setPen(pen);
    painter->drawLine(rect.topLeft(), rect.topRight());
    pen.setWidth(1);
    painter->setPen(pen);
    rect.setTop(rect.top() + 3);

    for(int x = 8; x < 16; x++)
    {
        paintOutput(rect, painter, list[x], x);
    }
}

void LabelOutput::paintSignal(QRectF &rect, QPainter *painter, int row, int port)
{
    QRectF size;
    QString text = QString("%1 - %2 ").arg(row, 2, 10, QChar(' ')).arg(m_model->data(row, "deviceName").toString());

    if(row == port)
        text += "(red)";
    else if(row == port + 1)
        text += "(green)";
    else if(row == port + 2)
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

void LabelOutput::paintOutput(QRectF &rect, QPainter *painter, const QString &text, int port)
{
    QRectF size;
    if(port > 7)
        port = port - 8;
    QString t = QString("%1 - %2").arg(port).arg(text);
    painter->drawText(rect, 0, t, &size);
    rect.setTop(rect.top() + size.height());

    if(port != m_model->rowCount() - 1)
    {
        QPen pen = painter->pen();
        pen.setWidth(3);
        painter->setPen(pen);
        painter->drawLine(rect.topLeft(), rect.topRight());
        pen.setWidth(1);
        painter->setPen(pen);
        rect.setTop(rect.top() + m_padding);
    }
}
