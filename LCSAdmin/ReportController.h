#ifndef REPORTCONTROLLER_H
#define REPORTCONTROLLER_H

#include "LabelPainter.h"
#include "GlobalDefs.h"

class ReportController : public LabelPainter
{
public:
    ReportController(EntityModel *model, QObject *parent = nullptr);
    // LabelPainter interface
    void paintHeader(QRectF &rect, QPainter *painter) override;
    void paintBody(QRectF &rect, QPainter *painter) override;

protected slots:
    virtual void printerPaintRequested(QPrinter *printer);

private:
    void paintModule(QRectF &rect, QPainter *painter, ModuleClassEnum moduleClass, int moduleID);
};

#endif // REPORTCONTROLLER_H
