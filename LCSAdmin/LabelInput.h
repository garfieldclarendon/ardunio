#ifndef LABELINPUT_H
#define LABELINPUT_H

#include "LabelPainter.h"

class LabelInput : public LabelPainter
{
    Q_OBJECT

public:
    LabelInput(EntityModel *model, QObject *parent = nullptr);

    // LabelPainter interface
    void paintHeader(QRectF &rect, QPainter *painter, const QFont &font) override;
    void paintBody(QRectF &rect, QPainter *painter, const QFont &font) override;

private:
    void paintInput(QRectF &rect, QPainter *painter, const QString &text, int port);
};

#endif // LABELINPUT_H
