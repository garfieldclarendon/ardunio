#include "DeviceModuleModel.h"
#include "GlobalDefs.h"
#include "JSonModel.h"
#include "API.h"

DeviceModuleModel::DeviceModuleModel(QObject *parent)
    : EntityModel("moduleDevicePort", parent), m_deviceID(0), m_moduleID(0)
{
    connect(API::instance(), SIGNAL(apiReady()), this, SLOT(apiReady()));
    apiReady();
}

QHash<int, QByteArray> DeviceModuleModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("id");
    roleNames[Qt::UserRole + 1] = QByteArray("deviceID");
    roleNames[Qt::UserRole + 2] = QByteArray("controllerModuleID");
    roleNames[Qt::UserRole + 3] = QByteArray("moduleName");
    roleNames[Qt::UserRole + 4] = QByteArray("moduleClass");
    roleNames[Qt::UserRole + 5] = QByteArray("deviceName");
    roleNames[Qt::UserRole + 6] = QByteArray("labelName");
    roleNames[Qt::UserRole + 7] = QByteArray("deivceClass");
    roleNames[Qt::UserRole + 8] = QByteArray("port");

    return roleNames;
}


void DeviceModuleModel::setDeviceID(int value)
{
    if(m_deviceID != value)
    {
        m_deviceID = value;
        emit deviceIDChanged();
        QJsonDocument jsonDoc;
        if(API::instance()->getApiReady())
        {
            QString json = API::instance()->getModuleDevicePortList(m_deviceID, m_moduleID);
            jsonDoc = QJsonDocument::fromJson(json.toLatin1());
            beginResetModel();
            m_jsonModel->setJson(jsonDoc, false);
            endResetModel();
            emit rowCountChanged();
        }
    }
}

void DeviceModuleModel::setModuleID(int value)
{
    if(m_moduleID != value)
    {
        m_moduleID = value;
        emit moduleIDChanged();
        QJsonDocument jsonDoc;
        if(API::instance()->getApiReady())
        {
            QString json = API::instance()->getModuleDevicePortList(m_deviceID, m_moduleID);
            jsonDoc = QJsonDocument::fromJson(json.toLatin1());
            beginResetModel();
            m_jsonModel->setJson(jsonDoc, false);
            endResetModel();
            emit rowCountChanged();
        }
    }
}

void DeviceModuleModel::apiReady()
{
    if(m_jsonModel && API::instance()->getApiReady() && (m_deviceID > 0 || m_moduleID > 0))
    {
        QJsonDocument jsonDoc;
        QString json = API::instance()->getModuleDevicePortList(m_deviceID, m_moduleID);
        jsonDoc = QJsonDocument::fromJson(json.toLatin1());
        beginResetModel();
        m_jsonModel->setJson(jsonDoc, false);
        endResetModel();
    }
}

void DeviceModuleModel::createEmptyObject(QJsonObject &obj)
{
    QHash<int, QByteArray> names(roleNames());

    for(int x = 0; x < names.values().count(); x++)
    {
        QString key(names.values().value(x));
        QVariant v;
        if(key == "deviceID")
            v = m_deviceID;
        else if(key == "moduleID")
            v = m_moduleID;
        else if(key == "port")
            v = 0;
        obj[key] = QJsonValue::fromVariant(v);
    }
}

