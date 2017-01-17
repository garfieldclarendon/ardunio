#include <QGuiApplication>
#include <QScreen>
#include <QFont>

#include "UI.h"

const qreal refDpi = 162.;
const qreal refHeight = 962.;
const qreal refWidth = 1080.;

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
    m_baseFontSize = QGuiApplication::font().pixelSize() > 0 ? QGuiApplication::font().pixelSize() : 8;

    qDebug("-----------------------------------------------------------------");
    qDebug(QString("init  DPI: %1 Width %2  Height %3  Font: %4").arg(dpi).arg(width).arg(height).arg(m_baseFontSize).toLatin1());
    qDebug("-----------------------------------------------------------------");
}

int UI::applyFontRatio(const int value)
{
    if(m_ratio == 0.0)
        init();
    int s = QGuiApplication::font().pointSize();
    qDebug(QString("VALUE IS %1 ratio is %2 = %3").arg(value).arg(m_ratioFont).arg(value * m_ratioFont).toLatin1());
    return value;
}
/*
int UI::applyRatio(const int value)
{
    if(m_ratio == 0.0)
        init();

    int ret = qMax(2, int(value * m_ratio));
//    qDebug(QString("applyRation: %1 to %2").arg(value).arg(ret).toLatin1());
    return ret;
}
*/

int UI::applyRatio(const int value)
{
    int maxSize = 0;
        qreal dpr = qApp->primaryScreen()->devicePixelRatio();
        qreal dpi = qApp->primaryScreen()->physicalDotsPerInch();
//        qreal dpi = qApp->primaryScreen()->logicalDotsPerInch();

        qreal density = dpi * dpr;
        qreal r = density / 160;
    //    qreal r = dpi / 160;

        qreal newMaxSize = 0.0;
        if(maxSize > 0)
        {
            newMaxSize = maxSize * r;
        }
        qreal newSize = value * r;

    #if defined(Q_OS_WIN) || defined(Q_OS_OSX)
        // for some reason, the dvicePixelRatio() function on Windows and Mac always returns 1.  Use this hack
        // to get a reasonable size
//        newSize = newSize * 2;
//        newMaxSize = newMaxSize * 2;
    #endif

        if(newSize > newMaxSize && maxSize > 0)
            newSize = newMaxSize;

//        qDebug(QString("DP SIZE %1  NEW SIZE %2").arg(value).arg(qRound(newSize)).toLatin1());

        return qRound(newSize);
}
