#include "SignalAspectModel.h"
#include "GlobalDefs.h"
#include "JSonModel.h"
#include "API.h"

SignalAspectModel::SignalAspectModel(QObject *parent)
    : EntityModel("signalAspect", parent), m_deviceID(0)
{
    connect(API::instance(), SIGNAL(apiReady()), this, SLOT(apiReady()));
}

QHash<int, QByteArray> SignalAspectModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("signalAspectID");
    roleNames[Qt::UserRole + 1] = QByteArray("deviceID");
    roleNames[Qt::UserRole + 2] = QByteArray("sortIndex");
    roleNames[Qt::UserRole + 3] = QByteArray("redMode");
    roleNames[Qt::UserRole + 4] = QByteArray("yellowMode");
    roleNames[Qt::UserRole + 5] = QByteArray("greenMode");

    return roleNames;
}

void SignalAspectModel::setdeviceID(int value)
{
    if(m_deviceID != value)
    {
        m_deviceID = value;
        emit deviceIDChanged();
        QJsonDocument jsonDoc;
        if(API::instance()->getApiReady())
        {
            QString json = API::instance()->getSignalAspectList(m_deviceID);
            jsonDoc = QJsonDocument::fromJson(json.toLatin1());
            beginResetModel();
            m_jsonModel->setJson(jsonDoc, false);
            endResetModel();
            emit rowCountChanged();
        }
    }
}

void SignalAspectModel::loadData()
{
    if(m_jsonModel && API::instance()->getApiReady())
    {
        QJsonDocument jsonDoc;
        QString json = API::instance()->getSignalAspectList(m_deviceID);
        jsonDoc = QJsonDocument::fromJson(json.toLatin1());
        beginResetModel();
        m_jsonModel->setJson(jsonDoc, false);
        endResetModel();
    }
}

int SignalAspectModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return roleNames().keys().count();
}

bool SignalAspectModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
    Q_UNUSED(source_row);
    bool ret = true;
    return ret;
}

void SignalAspectModel::apiReady()
{
    loadData();
}

void SignalAspectModel::createEmptyObject(QJsonObject &obj)
{
    QHash<int, QByteArray> names(roleNames());

    for(int x = 0; x < names.values().count(); x++)
    {
        QString key(names.values().value(x));
        QVariant v;
        if(key == "redMode" || key == "greenMode" || key == "yellowMode")
            v = 0;
        else if(key == "sortIndex")
            v = sourceModel()->rowCount();
        else if(key == "deviceID")
            v = m_deviceID;
        obj[key] = QJsonValue::fromVariant(v);
    }
}
