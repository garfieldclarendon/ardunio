#include "DevicePropertyModel.h"
#include "GlobalDefs.h"
#include "JSonModel.h"
#include "API.h"

DevicePropertyModel::DevicePropertyModel(QObject *parent)
    : EntityModel("deviceProperty", parent), m_deviceID(0), m_class(DeviceUnknown)
{
    connect(API::instance(), SIGNAL(apiReady()), this, SLOT(apiReady()));
    apiReady();
}

QHash<int, QByteArray> DevicePropertyModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("deviceID");
    roleNames[Qt::UserRole + 1] = QByteArray("id");
    roleNames[Qt::UserRole + 2] = QByteArray("key");
    roleNames[Qt::UserRole + 3] = QByteArray("value");

    return roleNames;
}


void DevicePropertyModel::setDeviceID(int value)
{
    if(m_deviceID != value)
    {
        m_deviceID = value;
        emit deviceIDChanged();
        QJsonDocument jsonDoc;
        if(API::instance()->getApiReady())
        {
            QString json = API::instance()->getDevicePropertyList(m_deviceID);
            jsonDoc = QJsonDocument::fromJson(json.toLatin1());
            beginResetModel();
            m_jsonModel->setJson(jsonDoc, false);
            endResetModel();
            emit rowCountChanged();
        }
    }
}

void DevicePropertyModel::setClass(int value)
{
    if(m_class != (DeviceClassEnum)value)
    {
        m_class = (DeviceClassEnum)value;
        emit classChanged();
    }
}

void DevicePropertyModel::loadData()
{
    if(m_jsonModel && API::instance()->getApiReady() && m_deviceID > 0)
    {
        QJsonDocument jsonDoc;
        QString json = API::instance()->getDevicePropertyList(m_deviceID);
        jsonDoc = QJsonDocument::fromJson(json.toLatin1());
        beginResetModel();
        m_jsonModel->setJson(jsonDoc, false);
        endResetModel();
    }
}

void DevicePropertyModel::apiReady()
{
    loadData();
}

void DevicePropertyModel::createEmptyObject(QJsonObject &obj)
{
    QHash<int, QByteArray> names(roleNames());

    for(int x = 0; x < names.values().count(); x++)
    {
        QString key(names.values().value(x));
        QVariant v;
        if(key == "deviceID")
            v = m_deviceID;
        obj[key] = QJsonValue::fromVariant(v);
    }
}
