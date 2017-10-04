#ifndef CONTROLLERTAB_H
#define CONTROLLERTAB_H

#include <QWidget>

class QSqlRelationalTableModel;
class QTableView;

class ControllerTab : public QWidget
{
    Q_OBJECT
public:
    explicit ControllerTab(QWidget *parent = 0);

signals:
    void currentControllerIDChanged(int controllerID);

public slots:
    void selectionChanged(void);

private:
    void setupUI(void);
    void setupModel(void);

    QTableView *tableView;
    QSqlRelationalTableModel *tableModel;
};

#endif // CONTROLLERTAB_H
