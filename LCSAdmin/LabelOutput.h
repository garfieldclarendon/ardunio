#ifndef LABELOUTPUT_H
#define LABELOUTPUT_H

#include "LabelPainter.h"

class LabelOutput : public LabelPainter
{
    Q_OBJECT

public:
    LabelOutput(EntityModel *model, QObject *parent = nullptr);

    // LabelPainter interface
    void paintHeader(QRectF &rect, QPainter *painter) override;
    void paintBody(QRectF &rect, QPainter *painter) override;
    void paintSignal(QRectF &rect, QPainter *painter, int row);

};

#endif // LABELOUTPUT_H
