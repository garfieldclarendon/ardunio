#ifndef CONTROLLERTAB_H
#define CONTROLLERTAB_H

#include <QWidget>

#include "Database.h"

class QSqlRelationalTableModel;
class QTableView;

class ControllerTab : public QWidget
{
    Q_OBJECT
public:
    explicit ControllerTab(QWidget *parent = 0);

signals:

public slots:

private:
    void setupUI(void);
    void setupModel(void);

    Database db;
    QTableView *tableView;
    QSqlRelationalTableModel *tableModel;
};

#endif // CONTROLLERTAB_H
