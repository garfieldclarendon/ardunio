#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Database.h"

class JMRIWebSocket;

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
    Database db;
    JMRIWebSocket *m_jmriSocket;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *) override;
};

#endif // MAINWINDOW_H
