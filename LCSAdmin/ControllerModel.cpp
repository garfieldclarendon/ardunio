#include "ControllerModel.h"
#include "GlobalDefs.h"
#include "JSonModel.h"
#include "API.h"

ControllerModel::ControllerModel(QObject *parent)
    : EntityModel("controller", parent), m_class(ControllerUnknown)
{
    connect(API::instance(), SIGNAL(apiReady()), this, SLOT(apiReady()));
    connect(API::instance(), SIGNAL(controllerChanged(int,ControllerStatusEnum,quint64)), this, SLOT(controllerChanged(int,ControllerStatusEnum,quint64)));
    apiReady();
}

QHash<int, QByteArray> ControllerModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("serialNumber");
    roleNames[Qt::UserRole + 1] = QByteArray("controllerID");
    roleNames[Qt::UserRole + 2] = QByteArray("controllerName");
    roleNames[Qt::UserRole + 3] = QByteArray("controllerDescription");
    roleNames[Qt::UserRole + 4] = QByteArray("controllerClass");
    roleNames[Qt::UserRole + 5] = QByteArray("status");
    roleNames[Qt::UserRole + 6] = QByteArray("version");
    roleNames[Qt::UserRole + 7] = QByteArray("pingLength");

    return roleNames;
}

void ControllerModel::setClass(int value)
{
    if(m_class != (ControllerClassEnum)value)
    {
        m_class = (ControllerClassEnum)value;
        emit classChanged();
        invalidateFilter();
    }
}

bool ControllerModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
    bool ret = true;
    if(m_class != ControllerUnknown)
    {
        if(m_jsonModel->data(source_row, "controllerClass", Qt::EditRole).toInt() != (int)m_class)
            ret = false;
    }
    return ret;
}

void ControllerModel::createEmptyObject(QJsonObject &obj)
{
    QHash<int, QByteArray> names(roleNames());

    for(int x = 0; x < names.values().count(); x++)
    {
        QString key(names.values().value(x));
        QVariant v;
        if(key == "serialNumber")
            v = -1;
        else if(key == "controllerClass")
            v = 0;
        obj[key] = QJsonValue::fromVariant(v);
    }
}

void ControllerModel::loadData()
{
    if(m_jsonModel && API::instance()->getApiReady())
    {
        QJsonDocument jsonDoc;
        QString json = API::instance()->getControllerList(0);
        jsonDoc = QJsonDocument::fromJson(json.toLatin1());
        beginResetModel();
        m_jsonModel->setJson(jsonDoc, false);
        endResetModel();
    }
}


void ControllerModel::apiReady()
{
    loadData();
}

void ControllerModel::controllerChanged(int serialNumber, ControllerStatusEnum status, quint64 pingLength)
{
    bool found = false;
    for(int x = 0; x < m_jsonModel->rowCount(); x++)
    {
        int d = m_jsonModel->data(x, "serialNumber", Qt::EditRole).toInt();
        if(d == serialNumber)
        {
            m_jsonModel->setData(x, "status", status);
            m_jsonModel->setData(x, "pingLength", pingLength);
            found = true;
            break;
        }
    }
    if(found == false && status == ControllerStatusConected)
    {
        emit newController(serialNumber);
    }
}

int ControllerModel::getControllerRow(int controllerID)
{
    int row = -1;
    for(int x = 0; x < m_jsonModel->rowCount(); x++)
    {
        if(m_jsonModel->data(x, "controllerID").toInt() == controllerID)
        {
            QModelIndex i = m_jsonModel->index(x, 0);
            i = mapFromSource(i);
            row = i.row();
            break;
        }
    }
    return row;
}

int ControllerModel::getControllerClass(int controllerID)
{
    int retClass = 0;
    for(int x = 0; x < m_jsonModel->rowCount(); x++)
    {
        if(m_jsonModel->data(x, "controllerID").toInt() == controllerID)
        {
            retClass = m_jsonModel->data(x, "deviceClass").toInt();
            break;
        }
    }
    return retClass;
}

void ControllerModel::setControllerID(int value)
{
    if(m_jsonModel && API::instance()->getApiReady())
    {
        QJsonDocument jsonDoc;
        QString json = API::instance()->getControllerList(value);
        jsonDoc = QJsonDocument::fromJson(json.toLatin1());
        beginResetModel();
        m_jsonModel->setJson(jsonDoc, false);
        endResetModel();
    }
}
