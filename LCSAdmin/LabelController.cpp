#include <QFont>
#include <QPainter>

#include "LabelController.h"
#include "LabelTurnout.h"
#include "EntityModel.h"
#include "GlobalDefs.h"
#include "DeviceModel.h"

LabelController::LabelController(EntityModel *model, QObject *parent)
    : LabelPainter(model, parent)
{

}

void LabelController::paintHeader(QRectF &rect, QPainter *painter)
{
    QRectF size;

    QPen pen = painter->pen();

    QFont font = painter->font();
    font.setPointSize(16);
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(rect, 0, "LCS Controller", &size);
    rect.setTop(rect.top() + size.height() + (m_padding * 10));
    font.setPointSize(12);
    font.setBold(false);
    painter->setFont(font);
    QString text = QString("%1").arg(m_model->data(0, "controllerName").toString());
    painter->drawText(rect, 0, text, &size);
    rect.setTop(rect.top() + size.height() + (m_padding * 5));
    text = QString("Serial #: %1").arg(m_model->data(0, "serialNumber").toInt());
    painter->drawText(rect, 0, text, &size);
    rect.setTop(rect.top() + size.height() + (m_padding * 5));
    text = QString("Controller ID #: %1").arg(m_model->data(0, "controllerID").toInt());
    painter->drawText(rect, 0, text, &size);
    rect.setTop(rect.top() + size.height() + (m_padding * 5));
    pen.setWidth(3);
    painter->setPen(pen);
    painter->drawLine(rect.topLeft(), rect.topRight());
    pen.setWidth(1);
    painter->setPen(pen);
    rect.setTop(rect.top() + (m_padding * 10));
}

void LabelController::paintBody(QRectF &rect, QPainter *painter)
{
    ControllerClassEnum c = (ControllerClassEnum)m_model->data(0, "controllerClass").toInt();

    if(c == ControllerTurnout)
    {
        int controllerID = m_model->data(0, "controllerID").toInt();
        DeviceModel m;
        m.setControllerID(controllerID);
        LabelTurnout tp(&m);
        tp.paintBody(rect, painter);
    }
}
