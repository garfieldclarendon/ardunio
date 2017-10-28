#ifndef ENTITYMETADATA_H
#define ENTITYMETADATA_H

#include <QObject>
#include <QJsonObject>
#include <QMap>

class EntityMetadata : public QObject
{
    Q_OBJECT
public:
    explicit EntityMetadata(QObject *parent = nullptr);

    static EntityMetadata *instance(void);

signals:

public slots:

private:
    void load(void);
    static EntityMetadata *m_instance;
    QMap<QString, QJsonObject> m_map;
};

#endif // ENTITYMETADATA_H
