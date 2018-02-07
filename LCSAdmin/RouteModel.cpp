#include "RouteModel.h"
#include "GlobalDefs.h"
#include "JSonModel.h"
#include "API.h"

RouteModel::RouteModel(QObject *parent)
    : EntityModel("route", parent)
{
    connect(API::instance(), SIGNAL(apiReady()), this, SLOT(apiReady()));
    connect(API::instance(), SIGNAL(routeChanged(int,bool,bool,bool)), this, SLOT(routeChanged(int,bool,bool,bool)));
    apiReady();
}

QHash<int, QByteArray> RouteModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("routeID");
    roleNames[Qt::UserRole + 1] = QByteArray("routeName");
    roleNames[Qt::UserRole + 2] = QByteArray("routeDescription");
    roleNames[Qt::UserRole + 3] = QByteArray("isActive");
    roleNames[Qt::UserRole + 4] = QByteArray("isLocked");
    roleNames[Qt::UserRole + 5] = QByteArray("canLock");

    return roleNames;
}


bool RouteModel::filterAcceptsRow(int , const QModelIndex &) const
{
    bool ret = true;
    return ret;
}

void RouteModel::apiReady()
{
    loadData();
}

void RouteModel::routeChanged(int routeID, bool isActive, bool isLocked, bool canLock)
{
    for(int x = 0; x < m_jsonModel->rowCount(); x++)
    {
        int d = m_jsonModel->data(x, "routeID", Qt::EditRole).toInt();
        if(d == routeID)
        {
            m_jsonModel->setData(x, "isActive", isActive);
            m_jsonModel->setData(x, "isLocked", isLocked);
            m_jsonModel->setData(x, "canLock", canLock);
            break;
        }
    }
}

int RouteModel::getRouteRow(int routeID)
{
    int row = -1;
    for(int x = 0; x < m_jsonModel->rowCount(); x++)
    {
        if(m_jsonModel->data(x, "routeID").toInt() == routeID)
        {
            QModelIndex i = m_jsonModel->index(x, 0);
            i = mapFromSource(i);
            row = i.row();
            break;
        }
    }
    return row;
}

void RouteModel::loadData()
{
    if(m_jsonModel && API::instance()->getApiReady())
    {
        QJsonDocument jsonDoc;
        QString json = API::instance()->getRouteList();
        jsonDoc = QJsonDocument::fromJson(json.toLatin1());
        beginResetModel();
        m_jsonModel->setJson(jsonDoc, false);
        endResetModel();
    }
}

void RouteModel::createEmptyObject(QJsonObject &obj)
{
    QHash<int, QByteArray> names(roleNames());

    for(int x = 0; x < names.values().count(); x++)
    {
        QString key(names.values().value(x));
        QVariant v;
        if(key == "isActive" || key == "isLocked" || key == "canLock")
            v = 0;
        obj[key] = QJsonValue::fromVariant(v);
    }
}
