#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QVariant>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "Database.h"
#include "GlobalDefs.h"

QString Database::fullPathAndFile;

Database::Database(QObject *parent) : QObject(parent)
{
    setupDb();
}

Database::~Database()
{

}

bool Database::init(const QString &pathAndFile)
{
    fullPathAndFile = pathAndFile;
    setupDb();
    bool ret = true;

    ret = checkDatabase();
    return ret;
}

void Database::removeDatabase()
{
    QString dbName = QString("QSQLITE_%1").arg((qlonglong)QThread::currentThreadId());
    db.close();
    QSqlDatabase::removeDatabase(dbName);
}

bool Database::checkDatabase(void)
{
    bool ret = createDatabase();
    if(ret)
    {
        int version = getDBVersion();
        updateDatabaseSchema(version);
    }
    return ret;
}

bool Database::createDatabase(void)
{
    bool ret = false;

    if(db.open())
    {
        if(createClassTable())
            if(createLayoutItemTable())
                if(createLayoutItemTypeTable())
                    if(createControllerTable())
                        if(createControllerModuleTable())
                            if(createDeviceTable())
                                if(createDevicePropertyTable())
                                    if(createRouteTable())
                                        if(createRouteEntryTable())
//                                            if(createPanelInputEntryTable())
//                                                if(createPanelOutputEntryTable())
                                                    if(createsignalAspect())
                                                        if(createSignalConditionTable())
                                                            ret = createPanelRouteTable();
    }
    else
    {
        qDebug(db.lastError().text().toLatin1());
        emit logError(1, db.lastError().number(), db.lastError().text());
    }

    return ret;
}

int Database::getControllerID(long serialNumber)
{
    int controllerID(-1);

    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    QSqlQuery query(db);

    bool ret = query.exec(QString("SELECT id FROM controller WHERE serialNumber = %1").arg(serialNumber));

    if(ret == false)
        qDebug(query.lastError().text().toLatin1());

    while(query.next())
    {
       controllerID = query.value(0).toInt();
       qDebug(QString("Found controllerID %1 for SerialNumber %2").arg(controllerID).arg(serialNumber).toLatin1());
    }

    return controllerID;
}

unsigned long Database::getSerialNumber(int controllerID)
{
    unsigned long serialNumber(-1);

    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    QSqlQuery query(db);

    bool ret = query.exec(QString("SELECT serialNumber FROM controller WHERE id = %1").arg(controllerID));

    if(ret == false)
        qDebug(query.lastError().text().toLatin1());

    while(query.next())
    {
       serialNumber = query.value(0).toInt();
       qDebug(QString("Found serialNumber %1 for ControllerID %2").arg(serialNumber).arg(controllerID).toLatin1());
    }

    return serialNumber;
}

int Database::addController(int controllerClass, const QString &controllerName, const QString &controllerDescription)
{
    int controllerID(-1);
    QByteArray config;

    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    controllerID = getNextID("controller");

    if(controllerID == 1)
        controllerID++;
    else if(controllerID >= 255)
        controllerID = -1;

    if(controllerID > 0)
    {
        QSqlQuery query(db);
        query.prepare("INSERT INTO controller (id, controllerName, controllerClass, controllerDescription) "
                      "VALUES (:id, :controllerName, :controllerClass, :controllerDescription)");
        query.bindValue(0, controllerID);
        query.bindValue(1, controllerName);
        query.bindValue(2, controllerClass);
        query.bindValue(3, controllerDescription);

        bool ret = query.exec();
        if(ret == false)
        {
            qDebug(query.lastError().text().toLatin1());
            emit logError(1, query.lastError().number(), query.lastError().text());
        }
    }
    return controllerID;
}

int Database::getNextID(const QString &tableName)
{
    int id(-1);

    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();

    QSqlQuery idQuery(db);

    bool ret = idQuery.exec(QString("SELECT Max(ID) FROM %1").arg(tableName));
    if(ret == false)
    {
        qDebug(idQuery.lastError().text().toLatin1());
        emit logError(1, idQuery.lastError().number(), idQuery.lastError().text());
        return id;
    }

    while (idQuery.next())
    {
       id = idQuery.value(0).toInt() + 1;
    }
    return id;
}

int Database::getDBVersion()
{
    int version(-1);
    QSqlQuery query(db);
    bool ret = query.exec("PRAGMA user_version");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    else
    {
        while (query.next())
        {
           version = query.value(0).toInt();
        }
    }
    return version;
}

void Database::setDBVersion(int newVersion)
{
    QSqlQuery query(db);
    bool ret = query.exec(QString("PRAGMA user_version = %1").arg(newVersion));
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
}

QString Database::getDeviceConfig(int deviceID)
{
    QJsonDocument doc;
    QJsonObject obj = fetchItem(QString("SELECT deviceClass FROM device WHERE id = %1").arg(deviceID));
    DeviceClassEnum deviceClass = (DeviceClassEnum)obj["deviceClass"].toVariant().toInt();
    getDeviceProperties(deviceID, obj);

    if(deviceClass == DeviceTurnout)
        getTurnoutConfig(deviceID, obj);
    else if(deviceClass == DeviceSignal)
        getSignalConfig(deviceID, obj);

    doc.setObject(obj);
    return doc.toJson();
}

void Database::getDeviceProperties(int deviceID, QJsonObject &device)
{
    QJsonArray array = fetchItems(QString("SELECT key, value FROM deviceProperty WHERE deviceID = %1").arg(deviceID));
    for(int x = 0; x < array.size(); x++)
    {
        QJsonObject o = array.at(x).toObject();
        for (int index = 0; index < o.size(); index++)
        {
            device[o["key"].toString()] = o["value"];
        }
    }
}

void Database::getTurnoutConfig(int deviceID, QJsonObject &device)
{
    QJsonArray routes = fetchItems(QString("SELECT routeID, turnoutState FROM routeEntry WHERE deviceID = %1").arg(deviceID));
    device["routes"] = routes;
}

void Database::getSignalConfig(int deviceID, QJsonObject &device)
{
    QJsonArray aspects = fetchItems(QString("SELECT id as aspectID, redMode, greenMode, yellowMode FROM signalAspect WHERE deviceID = %1 ORDER BY sortIndex").arg(deviceID));

    for(int x = 0; x < aspects.count(); x++)
    {
        QJsonObject o = aspects[x].toObject();
        int aspectID = o["aspectID"].toVariant().toInt();
        QJsonArray conditions = fetchItems(QString("select deviceID, conditionOperand, deviceState FROM signalCondition WHERE signalAspectID = %1").arg(aspectID));
        o["conditions"] = conditions;
        aspects[x] = o;
    }
    device["aspects"] = aspects;
}

QByteArray Database::getMultiControllerConfig(quint32 serialNumber)
{
    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    QString controllerClass;
    QString controllerID;
    QJsonObject currentModule;
    QJsonArray moduleArray;
    {
        QSqlQuery query(db);
        QString sql = QString("SELECT DISTINCT controller.id AS controllerID, moduleClass, address, controllerClass FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID WHERE serialNumber = %1 AND controllerModule.disable <> 1 ORDER BY address").arg(serialNumber);
        query.exec(sql);

        QString address;
        QString currentAddress;

        while (query.next())
        {
            controllerClass = query.value("controllerClass").toString();
            controllerID =  query.value("controllerID").toString();
            address = query.value("address").toString();
            if(currentAddress.length() == 0)
            {
                currentAddress = address;
                currentModule["class"] = query.value("moduleClass").toString();
                currentModule["address"] = address;
            }
            if(address != currentAddress)
            {
                currentAddress = address;
                moduleArray << currentModule;

                currentModule = QJsonObject();
                currentModule["class"] = query.value("moduleClass").toString();
                currentModule["address"] = address;
            }
        }
    }
    moduleArray << currentModule;
    QJsonArray array = getNotificationList(serialNumber);
    QJsonDocument jsonDoc;
    QJsonObject obj;
    obj["messageUri"] = "/controllerConfig";
    obj["controllerClass"] = controllerClass;
    obj["controllerID"] = controllerID;
    obj["modules"] = moduleArray;
    obj["controllersToNotify"] = array;
    jsonDoc.setObject(obj);

    return jsonDoc.toJson();
}

QJsonArray Database::getNotificationList(quint32 serialNumber)
{
    QList<int> ids;
    int controllerID = getControllerID(serialNumber);
    ids << controllerID;
    QJsonArray array1 = fetchItems(QString("SELECT DISTINCT controllerID as id FROM device JOIN controllerModule ON device.controllerModuleID = controllerModule.id WHERE device.ID IN (SELECT deviceID FROM deviceProperty JOIN device ON deviceProperty.value = device.id JOIN controllerModule ON device.controllerModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.ID WHERE key = 'ITEMID' AND serialNumber = %1)").arg(serialNumber));
    for(int x = array1.size() - 1; x >= 0; x--)
    {
        int id = array1[x].toObject()["id"].toVariant().toInt();
        if(ids.contains(id) == false)
            ids << id;
        else
            array1.removeAt(x);
    }
    QJsonArray array2 = fetchItems(QString("SELECT DISTINCT controllerID as id FROM device JOIN controllerModule ON device.controllerModuleID = controllerModule.id WHERE controllerModule.controllerID IN (SELECT pc.controllerID FROM controllerModule AS pc JOIN device on pc.id = device.controllerModuleID JOIN signalAspect on device.id = signalAspect.deviceID JOIN signalCondition ON signalAspect.id = signalCondition.signalAspectID WHERE signalCondition.deviceID IN (SELECT device.id FROM controller JOIN controllerModule ON controller.id = controllerModule.controllerID JOIN device ON device.controllerModuleID = controllerModule.id WHERE serialNumber = %1))").arg(serialNumber));
    for(int x = array2.size() - 1; x >= 0; x--)
    {
        QJsonObject o = array2[x].toObject();
        if(ids.contains(o["id"].toVariant().toInt()) == false)
        {
            array1 += array2[x];
            ids += array2[x].toVariant().toInt();
        }
        else
        {
            array2.removeAt(x);
        }
    }
    QJsonArray array3 = fetchItems(QString("SELECT DISTINCT controllerID from routeEntry JOIN device ON routeEntry.deviceID = device.id JOIN controllerModule ON device.controllerModuleID = controllerModule.id  WHERE routeID IN (SELECT value FROM deviceProperty JOIN device ON deviceProperty.deviceID = device.id JOIN controllerModule ON device.controllerModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.ID WHERE key = 'ROUTEID' AND serialNumber = %1)").arg(serialNumber));
    for(int x = array3.size() - 1; x >= 0; x--)
    {
        QJsonObject o = array3[x].toObject();
        if(ids.contains(o["id"].toVariant().toInt()) == false)
        {
            array1 += array3[x];
        }
    }

    return array1;
}

QByteArray Database::getControllerModuleConfig(quint32 serialNumber, quint32 address)
{
    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    QJsonArray deviceArray;

    QSqlQuery query(db);
    query.exec(QString("SELECT DISTINCT controller.id, moduleClass, address, device.id as deviceID, deviceClass, port FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID LEFT OUTER JOIN device ON controllerModule.id = device.controllerModuleID WHERE serialNumber = %1 AND controllerModule.address = %2 AND controllerModule.disable <> 1 ORDER BY address").arg(serialNumber).arg(address));

    while (query.next())
    {
        QJsonObject device;
        device["id"] = query.value("deviceID").toString();
        device["c"] = query.value("deviceClass").toString();
        device["p"] = query.value("port").toString();
        deviceArray << device;
    }

    QJsonDocument jsonDoc;
    QJsonObject obj;
    obj["messageUri"] = "/controllerModuleConfig";
    obj["devices"] = deviceArray;
    jsonDoc.setObject(obj);

    return jsonDoc.toJson();
}

QString Database::getTurnoutName(int deviceID)
{
    QString name;

    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    QSqlQuery query(db);

    bool ret = query.exec(QString("SELECT deviceName FROM device WHERE id = %1").arg(deviceID));

    if(ret == false)
        qDebug(query.lastError().text().toLatin1());

    while(query.next())
    {
       name = query.value(0).toString();
    }

    return name;
}

int Database::getdeviceID(const QString &name)
{
    int id = -1;

    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    QSqlQuery query(db);

    bool ret = query.exec(QString("SELECT id FROM device WHERE deviceName = '%1'").arg(name));

    if(ret == false)
        qDebug(query.lastError().text().toLatin1());

    while(query.next())
    {
       id = query.value(0).toInt();
    }

    return id;
}

QList<int> Database::getExcludeRouteList(int routeID)
{
    QList<int> ids;

    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    QSqlQuery query(db);

    bool ret = query.exec(QString("SELECT DISTINCT routeID FROM routeEntry WHERE deviceID IN (SELECT deviceID FROM routeEntry JOIN device ON routeEntry.deviceID = device.id WHERE routeID = %1)").arg(routeID));

    if(ret == false)
        qDebug(query.lastError().text().toLatin1());

    while(query.next())
    {
       ids << query.value(0).toInt();
    }

    return ids;
}

void Database::getControllerIDAndName(quint32 serialNumber, int &deviceID, QString &controllerName)
{
    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    QSqlQuery query(db);

    bool ret = query.exec(QString("SELECT id, controllerName FROM controller WHERE serialNumber = %1").arg(serialNumber));

    if(ret == false)
        qDebug(query.lastError().text().toLatin1());

    while(query.next())
    {
       deviceID = query.value(0).toInt();
       controllerName = query.value(1).toString();
    }
}

QJsonObject Database::fetchItem(const QString &queryString)
{
    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    QSqlQuery query(db);

    bool ret = query.exec(queryString);

    if(ret == false)
        qDebug(query.lastError().text().toLatin1());
    return createJsonObject(query);
}

QJsonArray Database::fetchItems(const QString &queryString)
{
    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    QSqlQuery query(db);

    bool ret = query.exec(queryString);

    if(ret == false)
        qDebug(query.lastError().text().toLatin1());
    return createJsonArray(query);
}

QJsonObject Database::createJsonObject(QSqlQuery &query)
{
    QJsonObject obj;
    if(query.next())
    {
        for(int x = 0; x < query.record().count(); x++)
        {
            QVariant v = query.value(x);
            obj[query.record().fieldName(x)] = v.toString();
        }
    }
    return obj;
}

QJsonArray Database::createJsonArray(QSqlQuery &query)
{
    QJsonArray array;
    while(query.next())
    {
        QJsonObject obj;
        for(int x = 0; x < query.record().count(); x++)
        {
            obj[query.record().fieldName(x)] = query.value(x).toString();
        }
        array.append(obj);
    }
    return array;
}

QSqlQuery Database::executeQuery(const QString &queryString)
{
    if(db.isValid() == false)
        db = QSqlDatabase::database();
    if(db.isOpen() == false)
        db.open();
    QSqlQuery query(db);

    bool ret = query.exec(queryString);

    if(ret == false)
        qDebug(query.lastError().text().toLatin1());
    return query;
}

void Database::setupDb()
{
    if(fullPathAndFile.length() > 0)
    {
        QString dbName = QString("QSQLITE_%1").arg((qlonglong)QThread::currentThreadId());
        if(QSqlDatabase::contains(dbName))
        {
            db = QSqlDatabase::database(dbName);
        }
        else
        {
            db = QSqlDatabase::addDatabase("QSQLITE", dbName);
            db.setDatabaseName(fullPathAndFile);
        }
    }
}

bool Database::createControllerTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS controller "
              "(id INTEGER primary key, "
              "controllerName VARCHAR(20), "
              "controllerClass INTEGER, "
              "serialNumber INTEGER, "
              "controllerDescription VARCHAR(50))");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    return ret;
}

bool Database::createControllerModuleTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS controllerModule "
              "(id INTEGER primary key, "
              "controllerID INTEGER, "
              "address INTEGER, "
              "moduleClass INTEGER, "
              "disable INTEGER, "
              "moduleName VARCHAR(20))");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    return ret;
}

bool Database::createClassTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS classCode "
              "(id INTEGER primary key, "
              "className VARCHAR(20))");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    else
    {
        bool count(0);
        {
            QSqlQuery countQuery(db);

            bool ret = countQuery.exec(QString("SELECT count(ID) FROM classCode"));
            if(ret == false)
            {
                qDebug(countQuery.lastError().text().toLatin1());
                emit logError(1, countQuery.lastError().number(), countQuery.lastError().text());
                return ret;
            }

            while (countQuery.next())
            {
               count = countQuery.value(0).toInt() + 1;
            }
        }
        if(count == 0)
        {
            QSqlQuery insertQuery(db);
            insertQuery.exec("INSERT INTO classCode (id, className) VALUES(0, 'Unknown')");
            insertQuery.exec("INSERT INTO classCode (id, className) VALUES(1, 'Turnout')");
            insertQuery.exec("INSERT INTO classCode (id, className) VALUES(2, 'Panel')");
            insertQuery.exec("INSERT INTO classCode (id, className) VALUES(3, 'Route')");
            insertQuery.exec("INSERT INTO classCode (id, className) VALUES(4, 'Signal')");
            insertQuery.exec("INSERT INTO classCode (id, className) VALUES(5, 'Block')");
            insertQuery.exec("INSERT INTO classCode (id, className) VALUES(6, 'System')");
            insertQuery.exec("INSERT INTO classCode (id, className) VALUES(7, 'Application')");
        }
    }
    return ret;
}

bool Database::createLayoutItemTypeTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS layoutItemType "
              "(id INTEGER primary key, "
              "typeName VARCHAR(20))");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    else
    {
        bool count(0);
        {
            QSqlQuery countQuery(db);

            bool ret = countQuery.exec(QString("SELECT count(ID) FROM layoutItemType"));
            if(ret == false)
            {
                qDebug(countQuery.lastError().text().toLatin1());
                emit logError(1, countQuery.lastError().number(), countQuery.lastError().text());
                return ret;
            }

            while (countQuery.next())
            {
               count = countQuery.value(0).toInt() + 1;
            }
        }
        if(count == 0)
        {
            QSqlQuery insertQuery(db);
            insertQuery.exec("INSERT INTO layoutItemType (id, typeName) VALUES(0, 'Unknown')");
            insertQuery.exec("INSERT INTO layoutItemType (id, typeName) VALUES(1, 'Turnout')");
            insertQuery.exec("INSERT INTO layoutItemType (id, typeName) VALUES(2, 'Block')");
            insertQuery.exec("INSERT INTO layoutItemType (id, typeName) VALUES(3, 'Siding')");
            insertQuery.exec("INSERT INTO layoutItemType (id, typeName) VALUES(4, 'Signal')");
            insertQuery.exec("INSERT INTO layoutItemType (id, typeName) VALUES(5, 'Spur')");
        }
    }
    return ret;
}

bool Database::createLayoutItemTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS layoutItem "
              "(id INTEGER primary key, "
              "controllerID INTEGER NOT NULL, "
              "itemName VARCHAR(20), "
              "itemDescription VARCHAR(50))");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    return ret;
}

bool Database::createRouteTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS route "
              "(id INTEGER primary key, "
              "routeName VARCHAR(20), "
              "routeDescription VARCHAR(50))");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    return ret;
}

bool Database::createRouteEntryTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS routeEntry "
              "(id INTEGER primary key, "
              "routeID INTEGER NOT NULL, "
              "deviceID INTEGER NOT NULL, "
              "turnoutState INTEGER NOT NULL)");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    return ret;
}

bool Database::createPanelInputEntryTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS panelInputEntry "
              "(id INTEGER primary key, "
              "panelModuleID INTEGER, "
              "inputName VARCHAR(20), "
              "inputDescription VARCHAR(50), "
              "inputID INTEGER, "
              "inputType INTEGER, " // Currently, the only valid entry is 1 = Route
              "pinIndex INTEGER "
              ")");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    return ret;
}

bool Database::createPanelOutputEntryTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS panelOutputEntry "
              "(id INTEGER primary key, "
              "panelModuleID INTEGER, "
              "outputName VARCHAR(20), "
              "outputDescription VARCHAR(50), "
              "itemID INTEGER, "
              "itemType INTEGER, " // Valid entries are: 1 = Turnout, 2 = Block
              "onValue INTEGER, "
              "flashingValue INTEGER, "
              "pinIndex INTEGER "
              ")");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    return ret;
}

bool Database::createSignalConditionTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS signalCondition "
              "(id INTEGER primary key, "
              "signalAspectID INTEGER, "
              "deviceID INTEGER, "
              "conditionOperand INTEGER, "
              "deviceState INTEGER "
              ")");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    return ret;
}

bool Database::createsignalAspect()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS signalAspect "
              "(id INTEGER primary key, "
              "deviceID INTEGER, "
              "sortIndex INTEGER, "
              "redMode INTEGER, "
              "yellowMode INTEGER, "
              "greenMode INTEGER "
              ")");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    return ret;
}

bool Database::createPanelRouteTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS panelRoute "
                     "(id INTEGER primary key, "
                     "panelOutputID INTEGER, "
                     "routeID INTEGER)");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    return ret;
}

bool Database::createDeviceTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS device "
                     "(id INTEGER primary key, "
                     "controllerModuleID INTEGER, "
                     "port INTEGER, "
                     "deviceClass INTEGER, "
                     "deviceName VARCHAR(20), "
                     "deviceDescription VARCHAR(50))");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    return ret;

}

bool Database::createDevicePropertyTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS deviceProperty "
                     "(id INTEGER primary key, "
                     "deviceID INTEGER, "
                     "parentID INTEGER, "
                     "key VARCHAR(25), "
                     "value VARCHAR(255))");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }

    return ret;
}

void Database::updateDatabaseSchema(int /* currentVersion */)
{
    // Add db schema changes here from version to versioin
    // If all changes are succesful, call setDBVersion() with the CurrentDatabaseVersion
    // REMEMBER!  currentVersion will = 0 if the database did not exist and was just
    // created....In this instance, the database will have the most recent database
    // schema, so no changes will be needed.

    setDBVersion(CurrentDatabaseVersion);
}

void Database::upgradeToVersion4()
{
    QJsonArray array = this->fetchItems("SELECT pinIndex, inputName, inputID, panelModuleID FROM panelInputEntry JOIN controllerModule ON panelInputEntry.panelModuleID = controllerModule.id");

    for(int x = 0; x < array.count(); x++)
    {
        QJsonObject obj = array[x].toObject();
        QString pinIndex = obj["pinIndex"].toString();
        QString moduleID = obj["panelModuleID"].toString();
        QString name = obj["inputName"].toString();

        QString sql = QString("INSERT INTO device (deviceClass, port, controllerModuleID, deviceName) VALUES(2, %1, %2, '%3')").arg(pinIndex).arg(moduleID).arg(name);
        this->executeQuery(sql);
    }
    array = this->fetchItems("SELECT device.id AS deviceID, inputID FROM panelInputEntry JOIN device ON panelInputEntry.inputName = device.deviceName AND deviceClass = 2");
    for(int x = 0; x < array.count(); x++)
    {
        QJsonObject obj = array[x].toObject();
        QString deviceID = obj["deviceID"].toString();
        QString inputID = obj["inputID"].toString();

        QString sql = QString("INSERT INTO deviceProperty (deviceID, key, value) VALUES(%1, 'ROUTEID', %2)").arg(deviceID).arg(inputID);
        this->executeQuery(sql);
    }



    array = this->fetchItems("SELECT pinIndex, outputName, panelModuleID FROM panelOutputEntry JOIN controllerModule ON panelOutputEntry.panelModuleID = controllerModule.id");

    for(int x = 0; x < array.count(); x++)
    {
        QJsonObject obj = array[x].toObject();
        QString pinIndex = obj["pinIndex"].toString();
        QString moduleID = obj["panelModuleID"].toString();
        QString name = obj["outputName"].toString();

        QString sql = QString("INSERT INTO device (deviceClass, port, controllerModuleID, deviceName) VALUES(3, %1, %2, '%3')").arg(pinIndex).arg(moduleID).arg(name);
        this->executeQuery(sql);
    }
    array = this->fetchItems("SELECT device.id AS deviceID, itemID, onValue, flashingValue FROM panelOutputEntry JOIN device ON panelOutputEntry.outputName = device.deviceName AND deviceClass = 3");
    for(int x = 0; x < array.count(); x++)
    {
        QJsonObject obj = array[x].toObject();
        QString deviceID = obj["deviceID"].toString();
        QString itemID = obj["itemID"].toString();
        QString onValue = obj["onValue"].toString();
        QString flashingValue = obj["flashingValue"].toString();

        QString sql = QString("INSERT INTO deviceProperty (deviceID, key, value) VALUES(%1, 'ITEMID', %2)").arg(deviceID).arg(itemID);
        this->executeQuery(sql);
        sql = QString("INSERT INTO deviceProperty (deviceID, key, value) VALUES(%1, 'ONVALUE', %2)").arg(deviceID).arg(onValue);
        this->executeQuery(sql);
        sql = QString("INSERT INTO deviceProperty (deviceID, key, value) VALUES(%1, 'FLASHINGVALUE', %2)").arg(deviceID).arg(flashingValue);
        this->executeQuery(sql);
    }
}

