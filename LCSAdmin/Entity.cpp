#include <QVariant>

#include "Entity.h"

Entity::Entity(void)
    : m_data(NULL)
{

}

Entity::Entity(const QJsonObject &obj)
    : m_data(NULL)
{
    m_object = obj;
}

Entity::Entity(const Entity &other)
    : m_data(NULL)
{
    copy(other);
}

Entity::~Entity()
{
    if(m_data)
        m_data->deleteLater();
}

QJsonObject Entity::getObject() const
{
    return m_object;
}

QQmlPropertyMap *Entity::getData()
{
    if(m_data == NULL)
        addProperties();

    return m_data;
}

void Entity::setValue(const QString &key, const QVariant &value)
{
    m_object[key] = QJsonValue::fromVariant(value);
}

QVariant Entity::getValue(const QString &key) const
{
    return m_object[key].toVariant();
}

bool Entity::hasError() const
{
    return m_object.contains("dbError");
}

QString Entity::errorText() const
{
    return m_object["dbError"].toString();
}

void Entity::copy(const Entity &other)
{
    removeProperties();
    m_object = other.m_object;
    m_entityName = other.m_entityName;
    addProperties();
}

void Entity::addProperties()
{
    m_data = new QQmlPropertyMap;
    QString blank("");
    QVariant var;
    QStringList keys = m_object.keys();
    foreach(QString key, keys)
    {
        var = m_object.value(key).toVariant();
        if(var.isValid() == false)
            var = blank;
        m_data->insert(key, var);
    }
}

void Entity::removeProperties()
{
    if(m_data)
        delete m_data;
    m_data = NULL;
}
