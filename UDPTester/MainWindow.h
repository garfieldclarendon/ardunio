#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Database.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected slots:
    void sendConfigData(int controllerID);
    void sendResetCommand(int controllerID);
    void sendFirmware(int controllerID);

protected:
    void setupUI(void);

private:
    void createControllerTab(QTabWidget *tab);
    Database db;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *) override;
};

#endif // MAINWINDOW_H
