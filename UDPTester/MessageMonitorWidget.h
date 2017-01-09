#ifndef MESSAGEMONITORWIDGET_H
#define MESSAGEMONITORWIDGET_H

#include <QWidget>

class QLabel;
class QPushButton;
class QTextEdit;
class QListView;
class QStringListModel;

class MessageMonitorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MessageMonitorWidget(QWidget *parent = 0);

signals:

public slots:
    void newRawMessage(const QString &message);
    void clearModel(void);
    void logError(int category, int code, const QString &errorText);

protected:
    void setupSocket(void);
    void setupUI(void);

private:
    QLabel *m_statusLabel;
    QListView *m_messages;
    QStringListModel *m_model;
};

#endif // MESSAGEMONITORWIDGET_H
