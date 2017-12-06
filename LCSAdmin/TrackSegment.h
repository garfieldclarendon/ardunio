#ifndef TRACKSEGMENT_H
#define TRACKSEGMENT_H

#include <QQuickPaintedItem>

class TrackSegment : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(int penWidth READ penWidth WRITE setPenWidth NOTIFY penWidthChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(Alignment textAlignment READ textAlignment WRITE setTextAlignment NOTIFY textAlignmentChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor activeColor READ activeColor WRITE setActiveColor NOTIFY activeColorChanged)
    Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor NOTIFY lineColorChanged)
    Q_PROPERTY(QColor occupiedColor READ occupiedColor WRITE setOccupiedColor NOTIFY occupiedColorChanged)

public:
    enum Alignment {
        AlignTop,
        AlignBottom
    };
    Q_ENUM(Alignment)

    TrackSegment(QQuickItem *parent = 0);

    int penWidth(void) const { return m_penWidth; }
    void setPenWidth(int value) { m_penWidth = value; emit penWidthChanged(); }
    QString text(void) const { return m_text; }
    void setText(const QString &value);
    Alignment textAlignment(void) const { return m_textAlignment; }
    void setTextAlignment(Alignment value) { m_textAlignment = value; emit textAlignmentChanged(); update(); }
    QColor backgroundColor(void) const {return m_backgroundColor; }
    void setBackgroundColor(const QColor &value) { m_backgroundColor = value; backgroundColorChanged(); update(); }
    QColor activeColor(void) const {return m_activeColor; }
    void setActiveColor(const QColor &value) { m_activeColor = value; activeColorChanged(); update(); }
    QColor lineColor(void) const {return m_lineColor; }
    void setLineColor(const QColor &value) { m_lineColor = value; lineColorChanged(); update(); }
    QColor occupiedColor(void) const {return m_occupiedColor; }
    void setOccupiedColor(const QColor &value) { m_occupiedColor = value; occupiedColorChanged(); update(); }

    void paint(QPainter *painter) override;
    virtual void paintSegment(QPainter *painter) = 0;

signals:
    void penWidthChanged(void);
    void textChanged(void);
    void textAlignmentChanged(void);
    void backgroundColorChanged(void);
    void activeColorChanged(void);
    void lineColorChanged(void);
    void occupiedColorChanged(void);

public slots:

protected:
    virtual void paintText(QPainter *painter);
    void updateHeight(void);
    int getTrackY(int track);
    void trim(QPainter *painter);

    void paintDoubleTrack(QPainter *painter);
    void paintCrossOverRight(QPainter *painter);
    void paintCrossOverLeft(QPainter *painter);
    void paintTurnoutRightNorth(QPainter *painter);

    const int m_lineHeight = 40;
    const int m_margin = 5;

    QString m_text;
    int m_top;
    int m_penWidth;
    Alignment m_textAlignment;
    Alignment m_segmentAlignment;
    QColor m_backgroundColor;
    QColor m_activeColor;
    QColor m_lineColor;
    QColor m_occupiedColor;

    // QQuickItem interface
protected:
    void setTotalTracks(int value) {m_totalTracks = value; updateHeight(); }
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    int m_totalTracks;
    QPoint m_lastMousePress;
};

#endif // TRACKSEGMENT_H
