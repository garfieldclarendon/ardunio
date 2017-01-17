#ifndef UI_H
#define UI_H

#include <QObject>
#include <QGuiApplication>
#include <QFont>
#include <QWidget>

class UI : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int margin READ getMargin WRITE setMargin NOTIFY marginChanged)
    Q_PROPERTY(int baseFontSize READ getBaseFontSize CONSTANT)

public:
    UI(QObject *parent = NULL);

    int getMargin(void) { applyRatio(m_margin); return m_margin; }
    void setMargin(int value) { m_margin = value; emit marginChanged(); }
    int getBaseFontSize(void) { return QGuiApplication::font().pointSize() > 0 ? QGuiApplication::font().pointSize() : 8; }
 //  int getBaseFontSize(void) { QWidget w; return  w.font().pixelSize(); }

signals:
    void marginChanged(void);

public slots:
    int applyFontRatio(const int value);
    int applyRatio(const int value);

private:
    void init(void);

    qreal m_ratio;
    qreal m_ratioFont;

    // standard definitions
    int m_margin;
    int m_baseFontSize;
};

#endif // UI_H
