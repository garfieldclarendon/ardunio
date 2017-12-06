#ifndef LABELCONTROLLER_H
#define LABELCONTROLLER_H

#include "LabelPainter.h"

class LabelController : public LabelPainter
{
    Q_OBJECT

public:
    LabelController(EntityModel *model, QObject *parent = nullptr);

    // LabelPainter interface
    void paintHeader(QRectF &rect, QPainter *painter) override;
    void paintBody(QRectF &rect, QPainter *painter) override;
};

#endif // LABELCONTROLLER_H
