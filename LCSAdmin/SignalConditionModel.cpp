#include "SignalConditionModel.h"
#include "GlobalDefs.h"
#include "JSonModel.h"
#include "API.h"

SignalConditionModel::SignalConditionModel(QObject *parent)
    : EntityModel("signalCondition", parent), m_aspectID(0)
{
    connect(API::instance(), SIGNAL(apiReady()), this, SLOT(apiReady()));
}

QHash<int, QByteArray> SignalConditionModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("signalConditionID");
    roleNames[Qt::UserRole + 1] = QByteArray("signalAspectID");
    roleNames[Qt::UserRole + 2] = QByteArray("deviceID");
    roleNames[Qt::UserRole + 3] = QByteArray("conditionOperand");
    roleNames[Qt::UserRole + 4] = QByteArray("deviceState");

    return roleNames;
}

void SignalConditionModel::setAspectID(int value)
{
    if(m_aspectID != value)
    {
        m_aspectID = value;
        emit aspectIDChanged();
        QJsonDocument jsonDoc;
        if(API::instance()->getApiReady())
        {
            QString json = API::instance()->getSignalConditionList(m_aspectID);
            jsonDoc = QJsonDocument::fromJson(json.toLatin1());
            beginResetModel();
            m_jsonModel->setJson(jsonDoc, false);
            endResetModel();
            emit rowCountChanged();
        }
    }
}

int SignalConditionModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return roleNames().keys().count();
}

bool SignalConditionModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
    Q_UNUSED(source_row);
    bool ret = true;
    return ret;
}

void SignalConditionModel::apiReady()
{
    if(m_jsonModel && API::instance()->getApiReady())
    {
        QJsonDocument jsonDoc;
        QString json = API::instance()->getSignalConditionList(m_aspectID);
        jsonDoc = QJsonDocument::fromJson(json.toLatin1());
        beginResetModel();
        m_jsonModel->setJson(jsonDoc, false);
        endResetModel();
    }
}

void SignalConditionModel::createEmptyObject(QJsonObject &obj)
{
    QHash<int, QByteArray> names(roleNames());

    for(int x = 0; x < names.values().count(); x++)
    {
        QString key(names.values().value(x));
        QVariant v;
        if(key == "signalAspectID")
            v = m_aspectID;
        else if(key == "conditionOperand")
            v = 0;
        else if(key == "deviceID")
            v = -1;
        else if(key == "deviceState")
            v = 0;
        obj[key] = QJsonValue::fromVariant(v);
    }
}
