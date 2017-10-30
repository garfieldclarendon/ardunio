#ifndef ENTITYMETADATA_H
#define ENTITYMETADATA_H

#include <QObject>
#include <QJsonObject>
#include <QMap>

class EntityMetadata : public QObject
{
    Q_OBJECT
public:
    explicit EntityMetadata(QObject *parent = NULL);

    static EntityMetadata *instance(void);

signals:

public slots:
    QString getTableName(const QString &entityName) const;
    QString getKeyField(const QString &entityName) const;
    QString getTableField(const QString &entityName) const;

private:
    void load(void);
    static EntityMetadata *m_instance;
    QMap<QString, QJsonObject> m_map;
};

#endif // ENTITYMETADATA_H
