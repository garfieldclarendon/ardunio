#include "RouteEntryModel.h"
#include "GlobalDefs.h"
#include "JSonModel.h"
#include "API.h"

RouteEntryModel::RouteEntryModel(QObject *parent)
    : EntityModel("routeEntry", parent), m_routeID(-1)
{
    connect(API::instance(), SIGNAL(apiReady()), this, SLOT(apiReady()));
    apiReady();
}

QHash<int, QByteArray> RouteEntryModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("routeEntryID");
    roleNames[Qt::UserRole + 1] = QByteArray("routeID");
    roleNames[Qt::UserRole + 2] = QByteArray("deviceID");
    roleNames[Qt::UserRole + 3] = QByteArray("turnoutState");

    return roleNames;
}

void RouteEntryModel::setRouteID(int value)
{
    if(m_routeID != value)
    {
        m_routeID = value;
        emit routeIDChanged();
        QJsonDocument jsonDoc;
        if(API::instance()->getApiReady())
        {
            QString json = API::instance()->getRouteEntryList(value);
            jsonDoc = QJsonDocument::fromJson(json.toLatin1());
            beginResetModel();
            m_jsonModel->setJson(jsonDoc, false);
            endResetModel();
            emit rowCountChanged();
        }
    }
}


bool RouteEntryModel::filterAcceptsRow(int , const QModelIndex &) const
{
    bool ret = true;
    return ret;
}

void RouteEntryModel::apiReady()
{
    loadData();
}


int RouteEntryModel::getRouteEntryRow(int routeEntryID)
{
    int row = -1;
    for(int x = 0; x < m_jsonModel->rowCount(); x++)
    {
        if(m_jsonModel->data(x, "routeEntryID").toInt() == routeEntryID)
        {
            QModelIndex i = m_jsonModel->index(x, 0);
            i = mapFromSource(i);
            row = i.row();
            break;
        }
    }
    return row;
}

void RouteEntryModel::loadData()
{
    if(m_jsonModel && API::instance()->getApiReady() && m_routeID > 0)
    {
        QJsonDocument jsonDoc;
        QString json = API::instance()->getRouteEntryList(m_routeID);
        jsonDoc = QJsonDocument::fromJson(json.toLatin1());
        beginResetModel();
        m_jsonModel->setJson(jsonDoc, false);
        endResetModel();
    }
}

void RouteEntryModel::createEmptyObject(QJsonObject &obj)
{
    QHash<int, QByteArray> names(roleNames());

    for(int x = 0; x < names.values().count(); x++)
    {
        QString key(names.values().value(x));
        QVariant v;
        if(key == "turnoutState")
            v = 0;
        else if(key == "routeID")
            v = m_routeID;
        else if(key == "deviceID")
            v = 0;
        obj[key] = QJsonValue::fromVariant(v);
    }
}
