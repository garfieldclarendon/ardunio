#include <QUrl>
#include <QJsonDocument>
#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>

#include "APIEntity.h"
#include "Database.h"
#include "WebServer.h"
#include "EntityMetadata.h"

APIEntity::APIEntity(QObject *parent) : QObject(parent)
{

}

void APIEntity::handleClient(QTcpSocket *socket, const QString &path, const QString &actionText, const QString &payload)
{
    QUrl url(path);

    QStringList parts = url.path().split('/', QString::SkipEmptyParts);
    QString entityName = parts.value(parts.count() - 1);
    qDebug(QString("HANDLEENTITY:  PATH: %1").arg(url.path()).toLatin1());
    qDebug(QString("HANDLEENTITY:  ENTITY NAME: %1").arg(entityName).toLatin1());

    QByteArray data(payload.toLatin1());

    if(actionText == "PUT")
        data = saveEntity(entityName, payload).toLatin1();
    else if(actionText == "POST")
        data = addEntity(entityName, payload).toLatin1();
    else if(actionText == "DELETE")
        deleteEntity(entityName, payload);
    else
        data = fetchEntity(entityName).toLatin1();

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();
}

QString APIEntity::fetchEntity(const QString &name)
{
    Database db;

    QString sql = QString("SELECT * FROM %1").arg(EntityMetadata::instance()->getTableName(name));
    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    return doc.toJson();
}

QString APIEntity::saveEntity(const QString &name, const QString &jsonText)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toLatin1());
    QJsonObject obj = doc.object();
    Database db;
    QSqlTableModel model(this, db.getDatabase());
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);

    QString keyField = EntityMetadata::instance()->getKeyField(name);
    QString tableKey = EntityMetadata::instance()->getTableField(name);
    QString tableName = EntityMetadata::instance()->getTableName(name);
    QStringList skip;
    skip << "keyField" << "tableKeyField" << "tableName" << keyField;

    int id = obj[keyField].toVariant().toInt();

    qDebug(QString("handleSaveEntity.  ID = %1").arg(id).toLatin1());

    QString filter = QString("%1 = %2").arg(tableKey).arg(id);
    model.setTable(tableName);
    model.setFilter(filter);
    if(model.select() == false)
    {
        QString error = model.lastError().text();
        obj["dbError"] = error;
    }

    if(model.rowCount() > 0)
    {
        QSqlRecord r = model.record(0);
        foreach (QString key, obj.keys())
        {
            if(skip.contains(key) == false)
            {
                QJsonValue val = obj[key];
                QVariant v = val.toVariant();
                r.setValue(key, v);
            }
        }
        model.setRecord(0, r);
        if(model.submitAll() == false)
        {
            QString error = model.lastError().text();
            obj["dbError"] = error;
        }
    }
    else
    {
        obj["dbError"] = QString("Failed to find record in table '%1' with the key value '%2'").arg(tableName).arg(id);
    }
    QJsonDocument retDoc;
    retDoc.setObject(obj);

    return retDoc.toJson();
}

QString APIEntity::addEntity(const QString &name, const QString &jsonText)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toLatin1());
    QJsonObject obj = doc.object();
    Database db;
    QSqlTableModel model(this, db.getDatabase());
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);

    QString keyField = EntityMetadata::instance()->getKeyField(name);
    QString tableKey = EntityMetadata::instance()->getTableField(name);
    QString tableName = EntityMetadata::instance()->getTableName(name);
    QStringList skip;
    skip << "keyField" << "tableKeyField" << "tableName" << keyField;

    int id = obj[keyField].toVariant().toInt();

    qDebug(QString("addEntity.  ID = %1").arg(id).toLatin1());

    QString filter = QString("%1 = %2").arg(tableKey).arg(id);
    model.setTable(tableName);
    model.setFilter(filter);
    model.select();
    model.insertRow(model.rowCount());

    if(model.rowCount() > 0)
    {
        QSqlRecord r = model.record(0);
        foreach (QString key, obj.keys())
        {
            if(skip.contains(key) == false)
            {
                QJsonValue val = obj[key];
                QVariant v = val.toVariant();
                r.setValue(key, v);
                r.setGenerated(key, true);
            }
        }
        r.setGenerated(keyField, false);
        model.setRecord(0, r);
        if(model.submitAll() == false)
        {
            QString error = model.lastError().text();
            obj["dbError"] = error;
        }
        else
        {
            QSqlQuery q = db.executeQuery("SELECT last_insert_rowid()");

            if(q.next())
            {
                 int id = q.value(0).toInt();
                qDebug(QString("addEntity.  NEW ID = %1").arg(id).toLatin1());
                if(id > 0)
                    obj[keyField] = id;
            }
        }
    }
    else
    {
        obj["dbError"] = QString("Failed to find record in table '%1' with the key value '%2'").arg(tableName).arg(tableKey);
    }
    QJsonDocument retDoc;
    retDoc.setObject(obj);

    return retDoc.toJson();
}

void APIEntity::deleteEntity(const QString &name, const QString &jsonText)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toLatin1());
    QJsonObject obj = doc.object();
    QString keyField = EntityMetadata::instance()->getKeyField(name);
    QString tableKey = EntityMetadata::instance()->getTableField(name);
    QString tableName = EntityMetadata::instance()->getTableName(name);
    int id = obj[keyField].toString().toInt();

    qDebug(QString("handleSaveDevice.  id = %1").arg(id).toLatin1());
    Database db;

    QString sql = QString("DELETE FROM %1 WHERE %2 = %3").arg(tableName).arg(tableKey).arg(id);
    qDebug(QString("handleSaveDevice.  %1").arg(sql).toLatin1());

    db.executeQuery(sql);
}

