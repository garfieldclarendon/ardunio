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
                                            if(createPanelInputEntryTable())
                                                if(createPanelOutputEntryTable())
                                                    if(createSignalTable())
                                                        if(createsignalAspect())
                                                            if(createSignalConditionTable())
                                                                if(createBlockTable())
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

QByteArray Database::getTurnoutConfig(quint32 serialNumber, int address)
{
    int deviceIDs[MAX_TURNOUTS];
    int turnoutInputPinSetting[MAX_TURNOUTS];
    memset(&deviceIDs, 0, sizeof(int[MAX_TURNOUTS]));

    QString buffer;

    QSqlQuery query(db);
    query.exec(QString("SELECT device.id, deviceProperty.value FROM device JOIN controllerModule ON device.controllerModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.ID LEFT OUTER JOIN deviceProperty on device.id = deviceProperty.deviceID WHERE serialNumber = %1 AND controllerModule.address = %2 ORDER BY controllerModule.address").arg(serialNumber).arg(address));
    int index = 0;
    while (query.next())
    {
       deviceIDs[index] = query.value(0).toInt();
       turnoutInputPinSetting[index++] = query.value(1).toInt();

    }
    for(int x = 0; x < MAX_TURNOUTS; x++)
    {
        buffer += QString("ID,%1;INPUTPIN,%2;").arg(deviceIDs[x]).arg(turnoutInputPinSetting[x]);

        QSqlQuery query2(db);
        query2.exec(QString("SELECT routeID, turnoutState FROM routeEntry WHERE deviceID = %1").arg(deviceIDs[x]));
        while (query2.next())
        {
            buffer += QString("ROUTE,%1;STATE,%2;").arg(query2.value(0).toString()).arg(query2.value(1).toString());
        }
    }

    buffer += "\n";
    qDebug(buffer.toLatin1());

    return buffer.toLatin1();
}

QByteArray Database::getSignalConfig(quint32 , int )
{
    QString buffer;

    return buffer.toLatin1();
}

QByteArray Database::getBlockConfig(quint32 serialNumber, int address)
{
    QString buffer;

    QSqlQuery query(db);
    query.exec(QString("SELECT block.id FROM block JOIN controller ON block.controllerID = controller.ID WHERE serialNumber = %1 AND controllerModule.address = %2 ORDER BY address").arg(serialNumber).arg(address));

    while (query.next())
    {
        buffer = QString("ID,%1;").arg(query.value(0).toString());
    }

    buffer += "\n";
    qDebug(buffer.toLatin1());

    return buffer.toLatin1();
}

QByteArray Database::getPanelConfig(quint32 serialNumber)
{
    QJsonDocument jsonDoc;
    QJsonObject obj;
    QJsonArray array;

    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    QSqlQuery query(db);
    query.exec(QString("SELECT moduleClass, address FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID WHERE serialNumber = %1 ORDER BY address").arg(serialNumber));
    while (query.next())
    {
        QJsonObject o;
        o["class"] = query.value(0).toString();
        o["index"] = query.value(1).toString();
        array.append(o);
    }
    obj["messageUri"] = "/controllerConfig";
    obj["modules"] = array;
    jsonDoc.setObject(obj);

    return jsonDoc.toJson();
}

QByteArray Database::getPanelRouteConfig(quint32 serialNumber)
{
    QSqlQuery routesQuery(db);
    routesQuery.exec(QString("SELECT routeID, deviceID, turnoutState FROM routeEntry WHERE RouteID IN (SELECT inputID FROM panelInputEntry WHERE inputType = 1 AND panelModuleID IN (SELECT controllerModule.id FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID WHERE serialNumber = %1)) ORDER BY routeID").arg(serialNumber));
    int currentRouteID = 0;
    QString buffer;

    while (routesQuery.next())
    {
        if(currentRouteID == 0)
        {
            currentRouteID = routesQuery.value(0).toInt();
            buffer += QString("ID,%1;").arg(currentRouteID);
        }
        if(currentRouteID == routesQuery.value(0).toInt())
        {
            buffer += QString("TURNOUT,%1;STATE,%2;").arg(routesQuery.value(1).toString()).arg(routesQuery.value(2).toString());
        }
        else
        {
            currentRouteID = routesQuery.value(0).toInt();
            buffer += QString("ID,%1;").arg(currentRouteID);
        }
    }

    buffer += "\n";
    qDebug(buffer.toLatin1());

    return buffer.toLatin1();
}

QByteArray Database::getMultiControllerConfig(quint32 serialNumber)
{
    QJsonDocument jsonDoc;
    QJsonObject obj;
    QJsonArray array;

    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    QSqlQuery query(db);
    query.exec(QString("SELECT moduleClass, address FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID WHERE serialNumber = %1 ORDER BY address").arg(serialNumber));
    while (query.next())
    {
        QJsonObject o;
        o["class"] = query.value(0).toString();
        o["index"] = query.value(1).toString();
        array.append(o);
    }
    obj["messageUri"] = "/controllerConfig";
    obj["modules"] = array;
    jsonDoc.setObject(obj);

    return jsonDoc.toJson();
}

QByteArray Database::getControllerModuleConfig(quint32 serialNumber, quint32 address)
{
    QString classCode;

    QSqlQuery query(db);
    query.exec(QString("SELECT moduleClass FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID WHERE serialNumber = %1 AND address = %2").arg(serialNumber).arg(address));
    while (query.next())
    {
        classCode = query.value(0).toString();
    }
    if(classCode == "1")
        return getTurnoutConfig(serialNumber, address);
    else if(classCode == "2")
        return getPanelConfig(serialNumber);
    else if(classCode == "4")
        return getSignalConfig(serialNumber, address);
    else if(classCode == "6")
        return getBlockConfig(serialNumber, address);

    QString configData("\n7");
    return configData.toLatin1();
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
            obj[query.record().fieldName(x)] = query.value(x).toString();
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

bool Database::createSignalTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS signal "
              "(id INTEGER primary key, "
              "controllerModuleID INTEGER, "
              "signalName VARCHAR(20), "
              "signalDescription VARCHAR(50))");
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
              "sortIndex INTEGER, "
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

bool Database::createBlockTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS block "
                     "(id INTEGER primary key, "
                     "controllerID INTEGER, "
                     "blockName VARCHAR(20), "
                     "blockDescription VARCHAR(50))");
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
                     "address INTEGER, "
                     "deviceName VARCHAR(20), "
                     "deviceDescription VARCHAR(50))");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    else
    {
        if(getDBVersion() > 0 && CurrentDatabaseVersion <= getDBVersion())
            return true;

        int moduleID = -1;
        {
            ret = db.exec("ALTER TABLE signal RENAME TO signal_old").lastError().isValid() == false;
            ret = createSignalTable();
            ret = db.exec(QString("INSERT INTO controllerModule (id, controllerID, address, moduleName) SELECT id, controllerID, panelIndex, panelName FROM panelModule")).lastError().isValid() == false;
            QSqlQuery moduleQuery(db);
            ret = moduleQuery.exec("SELECT MAX(id) FROM controllerModule");
            while(moduleQuery.next())
                moduleID = moduleQuery.value(0).toInt() + 1;
        }

        if(moduleID > 0)
        {
            QSqlQuery itemQuery(db);
            ret = itemQuery.exec("SELECT id, controllerID, itemName, itemDescription FROM layoutItem ORDER BY controllerID");
            if(ret)
            {
                int idHold = 0;
                while(itemQuery.next())
                {
                    int itemID = itemQuery.value("id").toInt();
                    int controllerID = itemQuery.value("controllerID").toInt();
                    QString moduleName = itemQuery.value("itemName").toString();
                    QString description = itemQuery.value("itemDescription").toString();

                    if(controllerID != idHold)
                    {
                        idHold = controllerID;
                        ret = db.exec(QString("INSERT INTO controllerModule (id, controllerID, moduleName, address) VALUES(%1, %2, '%3', 0)").arg(moduleID++).arg(controllerID).arg(moduleName)).lastError().isValid() == false;
                    }
                    ret = db.exec(QString("INSERT INTO device (id, controllerModuleID, deviceName, deviceDescription) VALUES(%1, %2, '%3', '%4')").arg(itemID).arg(moduleID).arg(moduleName).arg(description)).lastError().isValid() == false;
                }
            }
            {
                QSqlQuery signalQuery(db);
                ret = signalQuery.exec("SELECT id, controllerID FROM signal_old ORDER BY controllerID");
                while(signalQuery.next())
                {
                    int deviceID = signalQuery.value("id").toInt();
                    int controllerID = signalQuery.value("controllerID").toInt();
                    ret = db.exec(QString("INSERT INTO controllerModule (id, controllerID, moduleName, address) VALUES(%1, %2, 'Signal', 0)").arg(moduleID).arg(controllerID)).lastError().isValid() == false;
                    ret = db.exec(QString("INSERT INTO signal (id, controllerModuleID, signalName) VALUES(%1, %2, 'Signal')").arg(deviceID).arg(moduleID++)).lastError().isValid() == false;
                }
            }
            // clean up old tables
            ret = db.exec("DROP TABLE panelModule").lastError().isValid() == false;
            ret = db.exec("DELETE FROM layoutItem").lastError().isValid() == false;
            ret = db.exec(QString("DROP TABLE signal_old")).lastError().isValid() == false;
        }
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

void Database::updateDatabaseSchema(int /*currentVersion*/)
{
    // Add db schema changes here from version to versioin
    // If all changes are succesful, call setDBVersion() with the CurrentDatabaseVersion
    // REMEMBER!  currentVersion will = 0 if the database did not exist and was just
    // created....In this instance, the database will have the most recent database
    // schema, so no changes will be needed.
    setDBVersion(CurrentDatabaseVersion);
}

