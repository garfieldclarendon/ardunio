#ifndef TRACKTRUNOUT_H
#define TRACKTRUNOUT_H

#include <QWidget>
#include <QBrush>
#include <QPen>
#include <QPoint>

class TrackTrunout : public QWidget
{
    Q_OBJECT
public:
    explicit TrackTrunout(QWidget *parent = 0);

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;

    int getTurnoutID(void) const { return turnoutID; }
    void setTurnoutID(int value) { turnoutID = value; }

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    void adjustPoints(void);
    void sendMessage(bool toDiverging);

    QPen pen;
    QBrush brush;
    QPoint points[4];
    int turnoutID;
    QString currentState;
};

#endif // TRACKTRUNOUT_H
