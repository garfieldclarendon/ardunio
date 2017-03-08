#ifndef DEVICETAB_H
#define DEVICETAB_H

#include <QWidget>

#include "Database.h"

class QSqlRelationalTableModel;
class QTableView;
class QPushButton;

class DeviceTab : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceTab(QWidget *parent = 0);

signals:
    void sendConfig(int controllerID);
    void resetController(int controllerID);
    void sendFirmware(int controllerID);

public slots:
    void addButtonClicked(void);
    void deleteButtonClicked(void);
    void saveModel(void);
    void sendConfigClicked(void);
    void resetClicked(void);
    void sendFirmware(void);
    void setModuleID(int moduleID, int controllerID);

private:
    void setupUI(void);
    void setupModel(void);

    Database db;
    QTableView *tableView;
    QSqlRelationalTableModel *tableModel;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *saveButton;
    int controllerModuleID;
    int controllerID;
};

#endif // DEVICETAB_H
