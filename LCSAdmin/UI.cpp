#include <QGuiApplication>
#include <QScreen>
#include <QFont>
#include <QFontInfo>

#include "UI.h"

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
