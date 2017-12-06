#include <QGuiApplication>
#include <QScreen>
#include <QFont>
#include <QFontInfo>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QApplication>
#include <QPainter>

#include "UI.h"
#include "DeviceModel.h"
#include "API.h"

const qreal refDpi = 162.;
const qreal refHeight = 962.;
const qreal refWidth = 1080.;
const int refHeight2 = 13;
int refDiff = 0;

UI::UI(QObject *parent)
    :QObject(parent), m_ratio(0.0), m_ratioFont(0.0), m_margin(5), m_baseFontSize(0)
{
}

void UI::init()
{
    QRect rect = QGuiApplication::primaryScreen()->geometry();
    qreal height = qMax(rect.width(), rect.height());
    qreal width = qMin(rect.width(), rect.height());
//    qreal dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();
    qreal dpi = qApp->primaryScreen()->physicalDotsPerInch();
    m_ratio = qMin(height/refHeight, width/refWidth);
    m_ratioFont = qMin(height*refDpi/(dpi*refHeight), width*refDpi/(dpi*refWidth));
    m_baseFontSize = QFontInfo(QGuiApplication::font()).pointSize();

    QFontMetrics fm(QGuiApplication::font());
    int tmpRefHeight = fm.height();
//    int tmpRefWidth = fm.width('W');

    refDiff = refHeight2 - tmpRefHeight;
    qDebug("-----------------------------------------------------------------");
    qDebug(QString("init  DPI: %1 Width %2  Height %3  refDiff %4  Font: %5").arg(dpi).arg(width).arg(tmpRefHeight).arg(refDiff).arg(m_baseFontSize).toLatin1());
    qDebug("-----------------------------------------------------------------");
}

int UI::applyFontRatio(const int value)
{
//    int s = QGuiApplication::font().pointSize();
//    qDebug(QString("VALUE IS %1 ratio is %2 = %3").arg(value).arg(m_ratioFont).arg(value * m_ratioFont).toLatin1());
    return value;
}


int UI::applyRatio(const int value)
{
    int ret = refDiff + value;
    return ret;
}

void UI::printModuleLable(int moduleID)
{
    DeviceModel model;
    m_deviceModel = &model;
    model.setControllerModuleID(moduleID);

    QPrinter printer(QPrinter::HighResolution);
    qDebug(QString("BEFORE SIZE: %1 %2").arg(printer.pageRect().height()).arg(printer.pageRect().width()).toLatin1());
    QPageSize pageSize(QSizeF(50.0, 100.0), QPageSize::Millimeter, QString(), QPageSize::ExactMatch);
    printer.setPageSize(pageSize);
//    printer.setPageOrientation(QPageLayout::Landscape);
    qDebug(QString("AFTER SIZE: %1 %2").arg(printer.pageRect().height()).arg(printer.pageRect().width()).toLatin1());

    buildModuleTextDocumentForPrint(&model);
    QPrintPreviewDialog dlg(&printer, QApplication::focusWidget());
    connect(&dlg, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printerPaintRequested(QPrinter*)));

    dlg.exec();
}

void UI::buildModuleTextDocumentForPrint(DeviceModel *model)
{
    QString text("<table \"width:100%\" style=\"border-collapse:collapse\" >");
    for(int x = 0; x < model->rowCount(); x++)
    {
        text += QString("<tr><td><b>%1</b></td></tr>").arg(model->data(x, "deviceName").toString());
        text += QString("<tr><td>%1</td>").arg(getDeviceTypeName(model->data(x, "deviceClass").toInt()));
        text += QString("<tr><td><b>Port:</b> %1</td>").arg(model->data(x, "port").toString());
        text += "<tr><td></td></tr><tr><td></td></tr>";
    }
    text += "</table>";
}

void UI::printerPaintRequested(QPrinter *printer)
{
    QPainter painter(printer);

    QRectF r = printer->pageRect();
    QRectF size;

    QPen pen;
    pen.setColor(Qt::black);
    painter.setPen(pen);

    for(int x = 0; x < m_deviceModel->rowCount(); x++)
    {
        QFont font = painter.font();
        font.setPointSize(18);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(r, 0, m_deviceModel->data(x, "deviceName").toString(), &size);

        font.setBold(false);
        painter.setFont(font);
        r.setTop(r.top() + size.height() + 5);
        painter.drawText(r, 0, getDeviceTypeName(m_deviceModel->data(x, "deviceClass").toInt()), &size);
        r.setTop(r.top() + size.height() + 5);
        painter.drawText(r, 0, QString("Port: %1").arg(m_deviceModel->data(x, "port").toString()), &size);
        if(x < m_deviceModel->rowCount() - 1)
        {
            r.setTop(r.top() + size.height() + 15);
            pen.setWidth(3);
            painter.setPen(pen);
            painter.drawLine(r.topLeft(), r.topRight());
            pen.setWidth(1);
            painter.setPen(pen);
            r.setTop(r.top() + 15);
        }
    }
}

QString UI::getDeviceTypeName(int deviceType)
{
    QString ret;
    /*
    DeviceUnknown,
    DeviceTurnout,
    DevicePanelInput,
    DevicePanelOutput,
    DeviceSignal = 4,
    DeviceSemaphore = 5,
    DeviceBlock = 6, */

    switch (deviceType) {
    case DeviceTurnout:
        ret = "Turnout";
        break;
    case DevicePanelInput:
        ret = "Panel Input";
        break;
    case DevicePanelOutput:
        ret = "Panel Output";
        break;
    case DeviceSignal:
        ret = "Signal";
        break;
    case DeviceSemaphore:
        ret = "Semaphore";
        break;
    case DeviceBlock:
        ret = "Block";
        break;
    default:
        break;
    }
    return ret;
}
