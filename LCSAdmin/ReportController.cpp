#include <QFont>
#include <QPrinter>
#include <QPainter>

#include "ReportController.h"
#include "EntityModel.h"
#include "DeviceModel.h"
#include "DeviceModuleModel.h"
#include "ControllerModuleModel.h"
#include "LabelTurnout.h"
#include "LabelInput.h"
#include "LabelOutput.h"

ReportController::ReportController(EntityModel *model, QObject *parent)
    : LabelPainter(model, parent)
{

}

void ReportController::paintHeader(QRectF &rect, QPainter *painter)
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

void ReportController::paintBody(QRectF &rect, QPainter *painter)
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
    else if(c == ControllerMulti)
    {
        int controllerID = m_model->data(0, "controllerID").toInt();
        ControllerModuleModel moduleModel;
        moduleModel.setControllerID(controllerID);

        for(int x = 0; x < moduleModel.getRowCount(); x++)
        {
            int moduleID = moduleModel.data(x, "controllerModuleID").toInt();
            DeviceModuleModel model;
            model.setModuleID(moduleID);
            ModuleClassEnum moduleClass = (ModuleClassEnum)model.data(0, "moduleClass").toInt();
            if(moduleClass == ModuleInput)
            {
                LabelInput label(&model);
                label.paintHeader(rect, painter);
                label.paintBody(rect, painter);
            }
            else if(moduleClass == ModuleOutput)
            {
                LabelOutput label(&model);
                label.paintHeader(rect, painter);
                label.paintBody(rect, painter);
            }
            else if(moduleClass == ModuleTurnout)
            {
                LabelTurnout label(&model);
                label.paintHeader(rect, painter);
                label.paintBody(rect, painter);
            }
        }
    }
}

void ReportController::printerPaintRequested(QPrinter *printer)
{
    QPainter painter(printer);
    QRectF r = printer->pageRect();
    int moduleHeight = -1;

    QPen pen;
    pen.setColor(Qt::black);
    painter.setPen(pen);

    paintHeader(r, &painter);
    if(r.y() + 20 >= printer->height())
    {
        printer->newPage();
        r = printer->pageRect();
    }
    ControllerClassEnum c = (ControllerClassEnum)m_model->data(0, "controllerClass").toInt();

    if(c == ControllerTurnout)
    {
        int controllerID = m_model->data(0, "controllerID").toInt();
        DeviceModel m;
        m.setControllerID(controllerID);
        LabelTurnout tp(&m);
        tp.paintBody(r, &painter);
    }
    else if(c == ControllerMulti)
    {
        int controllerID = m_model->data(0, "controllerID").toInt();
        ControllerModuleModel moduleModel;
        moduleModel.setControllerID(controllerID);

        for(int x = 0; x < moduleModel.getRowCount(); x++)
        {
            int moduleID = moduleModel.data(x, "controllerModuleID").toInt();
            ModuleClassEnum moduleClass = (ModuleClassEnum)moduleModel.data(x, "moduleClass").toInt();
            int h = r.y();
            paintModule(r, &painter, moduleClass, moduleID);
            if(moduleHeight == -1)
                moduleHeight = r.y() - h;
            if(r.y() + moduleHeight >= printer->height())
            {
                printer->newPage();
                r = printer->pageRect();
            }
        }
    }
}

void ReportController::paintModule(QRectF &rect, QPainter *painter, ModuleClassEnum moduleClass, int moduleID)
{
    DeviceModuleModel model;
    model.setModuleID(moduleID);
    if(moduleClass == ModuleInput)
    {
        LabelInput label(&model);
        label.paintHeader(rect, painter);
        label.paintBody(rect, painter);
    }
    else if(moduleClass == ModuleOutput)
    {
        LabelOutput label(&model);
        label.paintHeader(rect, painter);
        label.paintBody(rect, painter);
    }
    else if(moduleClass == ModuleTurnout)
    {
        LabelTurnout label(&model);
        label.paintHeader(rect, painter);
        label.paintBody(rect, painter);
    }
}
