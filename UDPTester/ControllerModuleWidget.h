#ifndef CONTROLLERMODULEWIDGET_H
#define CONTROLLERMODULEWIDGET_H

#include <QWidget>

class QSqlRelationalTableModel;
class QTableView;

class ControllerModuleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControllerModuleWidget(QWidget *parent = 0);

signals:
    void currentControllerModuleIDChanged(int controllerModuleID, int controllerID);

public slots:
    void selectionChanged(void);
    void setCurrentControllerID(int controllerID);

private:
    void setupUI(void);
    void setupModel(void);

    QTableView *tableView;
    QSqlRelationalTableModel *tableModel;
    int m_controllerID;
};

#endif // CONTROLLERMODULEWIDGET_H
