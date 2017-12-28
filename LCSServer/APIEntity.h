#ifndef APIENTITY_H
#define APIENTITY_H

#include <QObject>
#include <QTcpSocket>

class APIEntity : public QObject
{
    Q_OBJECT
public:
    explicit APIEntity(QObject *parent = nullptr);

    void handleClient(QTcpSocket *socket, const QString &path, const QString &actionText, const QString &payload);

    QString fetchEntity(const QString &name);
    QString saveEntity(const QString &name, const QString &jsonText);
    QString addEntity(const QString &name, const QString &jsonText);
    void deleteEntity(const QString &name, const QString &jsonText);

signals:

public slots:

private:
};

#endif // APIENTITY_H
