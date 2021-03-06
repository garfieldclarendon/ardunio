#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

#include "EntityMetadata.h"

EntityMetadata *EntityMetadata::m_instance = NULL;

EntityMetadata::EntityMetadata(QObject *parent) : QObject(parent)
{
    load();
}

EntityMetadata *EntityMetadata::instance()
{
    if(m_instance == NULL)
        m_instance = new EntityMetadata(NULL);
    return m_instance;
}

QString EntityMetadata::getTableName(const QString &entityName) const
{
    return m_map.value(entityName)["tableName"].toString();
}

QString EntityMetadata::getKeyField(const QString &entityName) const
{
    return m_map.value(entityName)["keyField"].toString();
}

QString EntityMetadata::getTableField(const QString &entityName) const
{
    return m_map.value(entityName)["tableKeyField"].toString();
}

void EntityMetadata::load()
{
    QFile file(":/metadata/EntityMetadata.txt");
    if(file.open(QIODevice::ReadOnly))
    {
        QString text = file.readAll();
        text = text.simplified();
        QJsonDocument doc = QJsonDocument::fromJson(text.toLatin1());

        QJsonArray array = doc.array();
        QJsonObject obj;
        for(int x = 0; x < array.count(); x++)
        {
            obj = array[x].toObject();
            m_map[obj["name"].toString()] = obj;
        }
    }
}
