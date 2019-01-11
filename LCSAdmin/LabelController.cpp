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

void LabelController::paintHeader(QRectF &rect, QPainter *painter, const QFont &font)
{
    QRectF size;

    QPen pen = painter->pen();

    QFont f(font);
    f.setPointSize(font.pointSize() + 3);
    f.setBold(true);
    painter->setFont(f);
    painter->drawText(rect, 0, "LCS Controller", &size);
    rect.setTop(rect.top() + size.height() + (m_padding * 10));
    f.setPointSize(font.pointSize());
    f.setBold(false);
    painter->setFont(f);
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

void LabelController::paintBody(QRectF &rect, QPainter *painter, const QFont &font)
{
    ControllerClassEnum c = (ControllerClassEnum)m_model->data(0, "controllerClass").toInt();

    if(c == ControllerTurnout)
    {
        int controllerID = m_model->data(0, "controllerID").toInt();
        DeviceModel m;
        m.setControllerID(controllerID);
        LabelTurnout tp(&m);
        tp.paintBody(rect, painter, font);
    }
}
