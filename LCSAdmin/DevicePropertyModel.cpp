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
    roleNames[Qt::UserRole + 1] = QByteArray("ID");
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
            checkKeys();
        }
    }
}

void DevicePropertyModel::setClass(int value)
{
    if(m_class != (DeviceClassEnum)value)
    {
        m_class = (DeviceClassEnum)value;
        emit classChanged();
        checkKeys();
    }
}

void DevicePropertyModel::apiReady()
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

void DevicePropertyModel::checkKeys()
{
    if(m_class == DeviceTurnout)
        checkTurnoutKeys();
    else if(m_class == DevicePanelInput)
        checkPanelInputKeys();
    else if(m_class == DevicePanelOutput)
        checkPanelOutputKeys();
}

void DevicePropertyModel::checkTurnoutKeys()
{
    bool motorFound, inputFound = false;

    for(int x = 0; x < rowCount(); x++)
    {
        if(data(x, "key").toString().toUpper() == "MOTORPIN")
        {
            motorFound = true;
        }
        else if(data(x, "key").toString().toUpper() == "INPUTPIN")
        {
            inputFound = true;
        }
    }

    if(motorFound == false)
    {
       Entity e = getEntity(-1);
       e.setValue("key", "MOTORPIN");
       e.setValue("value", "0");
       setEntity(-1, e);
    }

    if(inputFound == false)
    {
       Entity e = getEntity(-1);
       e.setValue("key", "INPUTPIN");
       e.setValue("value", "0");
       setEntity(-1, e);
    }
}

void DevicePropertyModel::checkPanelInputKeys()
{
    bool routeIDFound = false;

    for(int x = 0; x < rowCount(); x++)
    {
        if(data(x, "key").toString().toUpper() == "ROUTEID")
        {
            routeIDFound = true;
        }
    }

    if(routeIDFound == false)
    {
       Entity e = getEntity(-1);
       e.setValue("key", "ROUTEID");
       e.setValue("value", "0");
       setEntity(-1, e);
    }
}

void DevicePropertyModel::checkPanelOutputKeys()
{
    bool itemFound, onFound, flashingFound = false;

    for(int x = 0; x < rowCount(); x++)
    {
        if(data(x, "key").toString().toUpper() == "ITEMID")
        {
            itemFound = true;
        }
        else if(data(x, "key").toString().toUpper() == "ONVALUE")
        {
            onFound = true;
        }
        else if(data(x, "key").toString().toUpper() == "FLASHINGVALUE")
        {
            flashingFound = true;
        }
    }

    if(itemFound == false)
    {
       Entity e = getEntity(-1);
       e.setValue("key", "ITEMID");
       e.setValue("value", "0");
       setEntity(-1, e);
    }

    if(onFound == false)
    {
       Entity e = getEntity(-1);
       e.setValue("key", "ONVALUE");
       e.setValue("value", "0");
       setEntity(-1, e);
    }

    if(onFound == false)
    {
       Entity e = getEntity(-1);
       e.setValue("key", "FLASHINGVALUE");
       e.setValue("value", "0");
       setEntity(-1, e);
    }
}
