#include <QFont>
#include <QPainter>

#include "LabelInput.h"
#include "EntityModel.h"

LabelInput::LabelInput(EntityModel *model, QObject *parent)
    : LabelPainter(model, parent)
{

}

void LabelInput::paintHeader(QRectF &rect, QPainter *painter)
{
    QRectF size;

    QPen pen = painter->pen();

    QFont font = painter->font();
    font.setPointSize(12);
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(rect, 0, "Input Module", &size);
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
    rect.setTop(rect.top() + m_padding);
}

void LabelInput::paintBody(QRectF &rect, QPainter *painter)
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
        list[port] = text;
    }

    for(int x = 7; x >= 0; x--)
    {
        paintInput(rect, painter, list[x], x);
    }

    pen.setWidth(3);
    painter->setPen(pen);
    painter->drawLine(rect.topLeft(), rect.topRight());
    pen.setWidth(1);
    painter->setPen(pen);
    rect.setTop(rect.top() + 3);

    for(int x = 8; x < 16; x++)
    {
        paintInput(rect, painter, list[x], x);
    }
}

void LabelInput::paintInput(QRectF &rect, QPainter *painter, const QString &text, int port)
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
