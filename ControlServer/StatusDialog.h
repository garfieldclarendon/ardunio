#ifndef STATUSDIALOG_H
#define STATUSDIALOG_H

#include <QDialog>

namespace Ui {
class StatusDialog;
}

class StatusDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatusDialog(QWidget *parent = 0);
    ~StatusDialog();

protected slots:
    void onControllerConnected(int index);
    void onControllerDisconnected(int index);
    void onControllerPing(int index, quint64 length);

    void onDeviceStatusChanged(int deviceID, int status);
    void onPinStateChanged(int moduleIndex, int pinNumber, int pinMode);
    void onNCEDataChanged(quint8 data, int blockIndex, int byteIndex);

private:
    void setupControllerList(void);
    void setupDeviceList(void);
    void setupPanelList(void);
    void setupNCEStatusList(void);

    Ui::StatusDialog *ui;
};

#endif // STATUSDIALOG_H
