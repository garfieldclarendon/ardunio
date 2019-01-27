#include <QMessageBox>

#include "DeviceModel.h"
#include "GlobalDefs.h"
#include "JSonModel.h"
#include "API.h"

DeviceModel::DeviceModel(QObject *parent)
    : EntityModel("device", parent), m_controllerModuleID(0), m_controllerID(0), m_class(DeviceUnknown)
{
    connect(API::instance(), SIGNAL(apiReady()), this, SLOT(apiReady()));
    connect(API::instance(), SIGNAL(deviceChanged(int,int)), this, SLOT(deviceChanged(int,int)));
    apiReady();
}

QHash<int, QByteArray> DeviceModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("deviceID");
    roleNames[Qt::UserRole + 1] = QByteArray("deviceName");
    roleNames[Qt::UserRole + 2] = QByteArray("deviceDescription");
    roleNames[Qt::UserRole + 3] = QByteArray("port");
    roleNames[Qt::UserRole + 4] = QByteArray("address");
    roleNames[Qt::UserRole + 5] = QByteArray("deviceClass");
    roleNames[Qt::UserRole + 6] = QByteArray("serialNumber");
    roleNames[Qt::UserRole + 7] = QByteArray("deviceState");
    roleNames[Qt::UserRole + 8] = QByteArray("controllerID");
    roleNames[Qt::UserRole + 9] = QByteArray("labelName");

    return roleNames;
}


void DeviceModel::setControllerModuleID(int value)
{
    if(m_controllerModuleID != value)
    {
        m_controllerModuleID = value;
        emit controllerIDChanged();
        if(m_jsonModel && API::instance()->getApiReady())
        {
            QJsonDocument jsonDoc;
            QString json = API::instance()->getDeviceList(-1, m_controllerModuleID, DeviceUnknown);
            jsonDoc = QJsonDocument::fromJson(json.toLatin1());
            beginResetModel();
            m_jsonModel->setJson(jsonDoc, false);
            endResetModel();
        }
    }
}

void DeviceModel::setControllerID(int value)
{
    if(m_controllerID != value)
    {
        m_controllerID = value;
        emit controllerIDChanged();
        if(m_jsonModel && API::instance()->getApiReady())
        {
            QJsonDocument jsonDoc;
            QString json = API::instance()->getDeviceList(m_controllerID, -1, DeviceUnknown);
            jsonDoc = QJsonDocument::fromJson(json.toLatin1());
            beginResetModel();
            m_jsonModel->setJson(jsonDoc, false);
            endResetModel();
        }
    }
}

void DeviceModel::setClass(int value)
{
    if(m_class != value)
    {
        m_class = value;
        if(m_class != 0)
            setFilterText(QString());
        emit classChanged();
        invalidateFilter();
    }
}

bool DeviceModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
    bool ret = true;

    if(m_class != DeviceUnknown)
    {
        if(m_class == 99)
        {
            int c = m_jsonModel->data(source_row, "deviceClass", Qt::EditRole).toInt();
            if(c != DeviceBlock && c!= DeviceTurnout)
                ret = false;
        }
        else
        {
            if(m_jsonModel->data(source_row, "deviceClass", Qt::EditRole).toInt() != m_class)
                ret = false;
        }
    }

    if(ret && m_filterText.length() > 0)
    {
        QString name = m_jsonModel->data(source_row, "deviceName", Qt::EditRole).toString();
        if(name.startsWith(m_filterText, Qt::CaseInsensitive) == false)
            ret = false;
    }

    return ret;
}

void DeviceModel::apiReady()
{
    loadData();
}

void DeviceModel::deviceChanged(int deviceID, int status)
{
    for(int x = 0; x < m_jsonModel->rowCount(); x++)
    {
        int d = m_jsonModel->data(x, "deviceID", Qt::EditRole).toInt();
        if(d == deviceID)
        {
            m_jsonModel->setData(x, "deviceState", status);
            break;
        }
    }
}

int DeviceModel::getDeviceRow(int deviceID)
{
    int row = -1;
    for(int x = 0; x < m_jsonModel->rowCount(); x++)
    {
        if(m_jsonModel->data(x, "deviceID").toInt() == deviceID)
        {
            QModelIndex i = m_jsonModel->index(x, 0);
            i = mapFromSource(i);
            row = i.row();
            break;
        }
    }
    return row;
}

int DeviceModel::getDeviceClass(int deviceID)
{
    int retClass = 0;
    for(int x = 0; x < m_jsonModel->rowCount(); x++)
    {
        if(m_jsonModel->data(x, "deviceID").toInt() == deviceID)
        {
            retClass = m_jsonModel->data(x, "deviceClass").toInt();
            break;
        }
    }
    return retClass;
}

Entity DeviceModel::createNewDevice(const QString &name, const QString &description, DeviceClassEnum deviceClass, bool createExtra)
{
    QString jsonText = API::instance()->createNewDevice(name, description, deviceClass, createExtra);
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toLatin1());
    QJsonObject newDevice = doc.object();
    int deviceID = newDevice["id"].toVariant().toInt();
    if(deviceID > 0)
    {
        jsonText = API::instance()->getDeviceList(-1, -1, DeviceUnknown, deviceID);
        doc = QJsonDocument::fromJson(jsonText.toLatin1());
        newDevice = doc.array().at(0).toObject();
    }

    Entity entity(newDevice);
    entity.setEntityName(m_entityName);
    return entity;
}

void DeviceModel::loadData()
{
    if(m_jsonModel && API::instance()->getApiReady())
    {
        QJsonDocument jsonDoc;
        QString json = API::instance()->getDeviceList(m_controllerID, m_controllerModuleID, DeviceUnknown);
        jsonDoc = QJsonDocument::fromJson(json.toLatin1());
        if(json.length() > 0 && jsonDoc.isNull())
        {
            QMessageBox::warning(NULL, "Device Load Error", "Download error.", QMessageBox::Ok);
        }
        beginResetModel();
        m_jsonModel->setJson(jsonDoc, false);
        endResetModel();
    }
}

void DeviceModel::setFilterText(const QString &filterText)
{
    if(filterText != m_filterText)
    {
        m_filterText = filterText;
        if(m_filterText.length() > 0)
            setClass(0);
        emit filterTextChanged();
        invalidateFilter();
    }
}

void DeviceModel::createEmptyObject(QJsonObject &obj)
{
    QHash<int, QByteArray> names(roleNames());

    for(int x = 0; x < names.values().count(); x++)
    {
        QString key(names.values().value(x));
        QVariant v;
        if(key == "port")
            v = 0;
        else if(key == "deviceClass")
            v = 0;
        else if(key == "controllerModuleID")
            v = -1;
        obj[key] = QJsonValue::fromVariant(v);
    }
}
