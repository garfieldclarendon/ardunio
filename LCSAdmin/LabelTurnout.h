#ifndef LABELTURNOUT_H
#define LABELTURNOUT_H

#include "LabelPainter.h"

class LabelTurnout : public LabelPainter
{
    Q_OBJECT

public:
    LabelTurnout(EntityModel *model, QObject *parent = nullptr);

    // LabelPainter interface
    void paintHeader(QRectF &rect, QPainter *painter) override;
    void paintBody(QRectF &rect, QPainter *painter) override;
};

#endif // LABELTURNOUT_H
