#ifndef UI_H
#define UI_H

#include <QObject>
#include <QGuiApplication>
#include <QFont>
#include <QFontInfo>
#include <QFontMetrics>

class UI : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int margin READ getMargin WRITE setMargin NOTIFY marginChanged)
    Q_PROPERTY(int baseFontSize READ getBaseFontSize CONSTANT)
    Q_PROPERTY(QString baseFontFamily READ getBaseFontFamily CONSTANT)
    Q_PROPERTY(QFont baseFont READ getBaseFont CONSTANT)

public:
    UI(QObject *parent = NULL);

    int getMargin(void) { applyRatio(m_margin); return m_margin; }
    void setMargin(int value) { m_margin = value; emit marginChanged(); }
    int getBaseFontSize(void) { return m_baseFontSize; }
    QString getBaseFontFamily(void) { return getBaseFont().family(); }
    QFont getBaseFont(void) { return QGuiApplication::font(); }
 //  int getBaseFontSize(void) { QWidget w; return  w.font().pixelSize(); }
    void init(void);

signals:
    void marginChanged(void);

public slots:
    int applyFontRatio(const int value);
    int applyRatio(const int value);

private:

    qreal m_ratio;
    qreal m_ratioFont;

    // standard definitions
    int m_margin;
    int m_baseFontSize;
};

#endif // UI_H
