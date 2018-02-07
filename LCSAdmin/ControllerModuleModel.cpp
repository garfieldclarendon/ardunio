#include "ControllerModuleModel.h"
#include "GlobalDefs.h"
#include "JSonModel.h"
#include "API.h"

ControllerModuleModel::ControllerModuleModel(QObject *parent)
    : EntityModel("controllerModule", parent), m_controllerID(0), m_controllerModuleID(0)
{
    connect(API::instance(), SIGNAL(apiReady()), this, SLOT(apiReady()));
}

QHash<int, QByteArray> ControllerModuleModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("controllerModuleID");
    roleNames[Qt::UserRole + 1] = QByteArray("controllerID");
    roleNames[Qt::UserRole + 2] = QByteArray("address");
    roleNames[Qt::UserRole + 3] = QByteArray("moduleName");
    roleNames[Qt::UserRole + 4] = QByteArray("moduleClass");
    roleNames[Qt::UserRole + 5] = QByteArray("disable");

    return roleNames;
}

void ControllerModuleModel::setControllerID(int value)
{
    if(m_controllerID != value)
    {
        m_controllerID = value;
        emit controllerIDChanged();
        QJsonDocument jsonDoc;
        if(API::instance()->getApiReady())
        {
            QString json = API::instance()->getControllerModuleListByControllerID(m_controllerID);
            jsonDoc = QJsonDocument::fromJson(json.toLatin1());
            beginResetModel();
            m_jsonModel->setJson(jsonDoc, false);
            endResetModel();
            emit rowCountChanged();
        }
    }
}

void ControllerModuleModel::setControllerModuleID(int value)
{
    if(m_controllerModuleID != value)
    {
        m_controllerModuleID = value;
        emit controllerIDChanged();
        QJsonDocument jsonDoc;
        if(API::instance()->getApiReady())
        {
            QString json = API::instance()->getControllerModuleListByModuleID(m_controllerModuleID);
            jsonDoc = QJsonDocument::fromJson(json.toLatin1());
            beginResetModel();
            m_jsonModel->setJson(jsonDoc, false);
            endResetModel();
            emit rowCountChanged();
        }
    }
}

void ControllerModuleModel::loadData()
{
    if(m_jsonModel && API::instance()->getApiReady())
    {
        QJsonDocument jsonDoc;
        QString json = API::instance()->getControllerModuleListByControllerID(m_controllerID);
        jsonDoc = QJsonDocument::fromJson(json.toLatin1());
        beginResetModel();
        m_jsonModel->setJson(jsonDoc, false);
        endResetModel();
    }
}

int ControllerModuleModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return roleNames().keys().count();
}

bool ControllerModuleModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
    Q_UNUSED(source_row);
    bool ret = true;
    return ret;
}

void ControllerModuleModel::apiReady()
{
    loadData();
}

void ControllerModuleModel::createEmptyObject(QJsonObject &obj)
{
    QHash<int, QByteArray> names(roleNames());

    for(int x = 0; x < names.values().count(); x++)
    {
        QString key(names.values().value(x));
        QVariant v;
        if(key == "moduleClass")
            v = 0;
        else if(key == "controllerID")
            v = m_controllerID;
        else if(key == "address")
            v = sourceModel()->rowCount();
        else if(key == "disable")
            v = 0;
        obj[key] = QJsonValue::fromVariant(v);
    }
}
