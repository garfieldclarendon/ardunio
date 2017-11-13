#ifndef ENTITY_H
#define ENTITY_H

#include <QQmlPropertyMap>
#include <QJsonObject>

class Entity
{
    Q_GADGET
    Q_PROPERTY(QQmlPropertyMap *data READ getData)
    Q_PROPERTY(QString entityName READ getEntityName)

public:
    Entity(void);
    Entity(const QJsonObject &obj);
    Entity(const Entity &other);
    ~Entity(void);

    QJsonObject getObject(void) const;
    void setObject(const QJsonObject &value)  { m_object = value; }
    Entity& operator = (const Entity &other)
    {
        copy(other);
        return *this;
    }

    QQmlPropertyMap *getData(void);
    QString getEntityName(void) const { return m_entityName; }
    void setEntityName(const QString &value) { m_entityName = value; }

signals:

public slots:
    void setValue(const QString &key, const QVariant &value);
    QVariant getValue(const QString &key) const;
    bool hasError(void) const;
    QString errorText(void) const;

private:
    void copy(const Entity &other);
    void addProperties(void);
    void removeProperties(void);

    QJsonObject m_object;
    QQmlPropertyMap *m_data;
    QString m_entityName;
};
Q_DECLARE_METATYPE(Entity)
Q_DECLARE_OPAQUE_POINTER(Entity)

#endif // ENTITY_H
