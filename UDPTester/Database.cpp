#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "Database.h"
#include "GlobalDefs.h"
#include "ConfigStructures.h"

Database::Database(QObject *parent) : QObject(parent)
{
}

Database::~Database()
{

}

bool Database::init(const QString &pathAndFile)
{
    fullPathAndFile = pathAndFile;
    bool ret = true;

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(pathAndFile);

    ret = checkDatabase();
    return ret;
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
                if(createControllerTable())
                    if(createLayoutItemTable())
                        if(createRouteTable())
                            if(createRouteEntryTable())
                                if(createPanelModuleTable())
                                    if(createPanelInputEntryTable())
                                        if(createPanelOutputEntryTable())
                                            if(createSignalTable())
                                                if(createSignalAspectCondition())
                                                    if(createSignalConditionTable())
                                                        ret = createBlockTable();
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

QByteArray Database::getTurnoutConfig(quint32 serialNumber)
{
    TurnoutControllerConfigStruct *configStruct = new TurnoutControllerConfigStruct;
    memset(configStruct, 0, sizeof(TurnoutControllerConfigStruct));

    int turnoutIDs[2];
    turnoutIDs[0] = 0;
    turnoutIDs[1] = 0;

    QSqlQuery query(db);
    query.exec(QString("SELECT layoutItem.id FROM layoutItem JOIN controller ON layoutItem.controllerID = controller.ID WHERE serialNumber = %1").arg(serialNumber));
//    query.exec(QString("SELECT device.id FROM device JOIN controller ON layoutItem.controllerID = controller.ID WHERE controllerID = %1").arg(serialNumber));
    int index = 0;
    while (query.next())
    {
       turnoutIDs[index++] = query.value(0).toInt();

    }
    configStruct->turnout1.turnoutID = turnoutIDs[0];
    configStruct->turnout2.turnoutID = turnoutIDs[1];

    QSqlQuery query2(db);
    query2.exec(QString("SELECT routeID, turnoutState FROM routeEntry WHERE turnoutID = %1").arg(turnoutIDs[0]));
    index = 0;
    while (query2.next())
    {
        if(index < MAX_ROUTE_ENTRIES)
        {
            configStruct->turnout1.routeEntries[index].routeID = query2.value(0).toInt();
            configStruct->turnout1.routeEntries[index++].state   = (TurnoutState)query2.value(1).toInt();
        }
    }

    QSqlQuery query3(db);
    query3.exec(QString("SELECT routeID, turnoutState FROM routeEntry WHERE turnoutID = %1").arg(turnoutIDs[1]));
    index = 0;
    while (query3.next())
    {
        if(index < MAX_ROUTE_ENTRIES)
        {
            configStruct->turnout2.routeEntries[index].routeID = query3.value(0).toInt();
            configStruct->turnout2.routeEntries[index++].state   = (TurnoutState)query3.value(1).toInt();
        }
    }

    QByteArray config((const char *)configStruct, sizeof(TurnoutControllerConfigStruct));
    delete configStruct;
    return config;
}

QByteArray Database::getSignalConfig(quint32 serialNumber)
{
    SignalControllerConfigStruct *configStruct = new SignalControllerConfigStruct;
    memset(configStruct, 0, sizeof(SignalControllerConfigStruct));

    int signalIDs[2];
    signalIDs[0] = 0;
    signalIDs[1] = 0;

    QSqlQuery query(db);
    query.exec(QString("SELECT signal.id FROM signal JOIN controller ON signal.controllerID = controller.ID WHERE serialNumber = %1").arg(serialNumber));
    int index = 0;
    while (query.next())
    {
       signalIDs[index++] = query.value(0).toInt();

    }
    configStruct->signal1.signalID = signalIDs[0];
    configStruct->signal2.signalID = signalIDs[1];

    QSqlQuery query2(db);
    query2.exec(QString("SELECT id, redMode, yellowMode, greenMode FROM signalAspectCondition WHERE signalID = %1 ORDER BY sortIndex").arg(signalIDs[0]));
    index = 0;
    while (query2.next())
    {
        if(index < MAX_SIGNAL_CONDITIONS)
        {
            int aspectID = query2.value(0).toInt();
            configStruct->signal1.conditions[index].aspect.redMode = query2.value(1).toInt();
            configStruct->signal1.conditions[index].aspect.yellowMode = query2.value(2).toInt();
            configStruct->signal1.conditions[index].aspect.greenMode = query2.value(3).toInt();

            QSqlQuery query3(db);
            query3.exec(QString("SELECT deviceID, conditionOperand, deviceState FROM signalCondition WHERE signalAspectConditionID = %1 ORDER BY sortIndex").arg(aspectID));
            int conditionIndex = 0;
            while (query3.next())
            {
                if(index < MAX_SIGNAL_CONDITIONS)
                {
                    configStruct->signal1.conditions[index].conditions[conditionIndex].deviceID = query3.value(0).toInt();
                    configStruct->signal1.conditions[index].conditions[conditionIndex].operand = query3.value(1).toInt();
                    configStruct->signal1.conditions[index].conditions[conditionIndex++].deviceState = query3.value(2).toInt();
                }
            }
            index++;
        }
    }

    QSqlQuery query4(db);
    query4.exec(QString("SELECT id, redMode, yellowMode, greenMode FROM signalAspectCondition WHERE signalID = %1 ORDER BY sortIndex").arg(signalIDs[1]));
    index = 0;
    while (query4.next())
    {
        if(index < MAX_SIGNAL_CONDITIONS)
        {
            int aspectID = query4.value(0).toInt();
            configStruct->signal2.conditions[index].aspect.redMode = query4.value(1).toInt();
            configStruct->signal2.conditions[index].aspect.yellowMode = query4.value(2).toInt();
            configStruct->signal2.conditions[index].aspect.greenMode = query4.value(3).toInt();

            QSqlQuery query5(db);
            query5.exec(QString("SELECT deviceID, conditionOperand, deviceState FROM signalCondition signalAspectConditionID = %1 ORDER BY sortIndex").arg(aspectID));
            int conditionIndex = 0;
            while (query5.next())
            {
                if(index < MAX_SIGNAL_CONDITIONS)
                {
                    configStruct->signal2.conditions[index].conditions[conditionIndex].deviceID = query5.value(0).toInt();
                    configStruct->signal2.conditions[index].conditions[conditionIndex].operand = query5.value(1).toInt();
                    configStruct->signal2.conditions[index].conditions[conditionIndex++].deviceState = query5.value(2).toInt();
                }
            }
            index++;
        }
    }

    QByteArray config((const char *)configStruct, sizeof(SignalControllerConfigStruct));
    delete configStruct;
    return config;
}

QByteArray Database::getBlockConfig(quint32 serialNumber)
{
    BlockControllerConfigStruct *configStruct = new BlockControllerConfigStruct;
    memset(configStruct, 0, sizeof(BlockControllerConfigStruct));

    QSqlQuery query(db);
    query.exec(QString("SELECT block.id FROM block JOIN controller ON block.controllerID = controller.ID WHERE serialNumber = %1").arg(serialNumber));
    int index = 0;
    while (query.next())
    {
        if(index++ == 0)
            configStruct->block1.blockID = query.value(0).toInt();
        else
            configStruct->block2.blockID = query.value(0).toInt();
    }

    QByteArray config((const char *)configStruct, sizeof(BlockControllerConfigStruct));
    delete configStruct;
    return config;
}

QByteArray Database::getPanelConfig(quint32 serialNumber)
{
    int panelModuleIDs[MAX_PANEL_MODULES];
    memset(&panelModuleIDs, 0, sizeof(panelModuleIDs));
    PanelControllerConfigStruct configStruct;
    memset(&configStruct, 0, sizeof(PanelControllerConfigStruct));

    QSqlQuery query(db);
    query.exec(QString("SELECT panelModule.id FROM panelModule JOIN controller ON panelModule.controllerID = controller.ID WHERE serialNumber = %1 ORDER BY panelIndex").arg(serialNumber));
    int index = 0;
    while (query.next())
    {
       panelModuleIDs[index++] = query.value(0).toInt();
       configStruct.mdouleCount++;
    }

    for(int x = 0; x < configStruct.mdouleCount; x++)
    {
        {
            QSqlQuery inputsQuery(db);
            inputsQuery.exec(QString("SELECT pinIndex, inputID, inputType FROM panelInputEntry WHERE panelModuleID = %1 ORDER BY pinIndex").arg(panelModuleIDs[x]));
            int inputIndex = 0;
            while (inputsQuery.next())
            {
                inputIndex = inputsQuery.value(0).toInt();
                configStruct.moduleConfigs[x].inputs[inputIndex].id = inputsQuery.value(1).toInt();
                configStruct.moduleConfigs[x].inputs[inputIndex].inputType = inputsQuery.value(2).toInt();
//                configStruct.moduleConfigs[x].inputs[inputIndex].value = inputsQuery.value(3).toInt();
            }
            QSqlQuery outputsQuery(db);
            outputsQuery.exec(QString("SELECT pinIndex, itemID, itemType, onValue, flashingValue FROM panelOutputEntry WHERE panelModuleID = %1 ORDER BY pinIndex").arg(panelModuleIDs[x]));
            int outputIndex = 0;
            while (outputsQuery.next())
            {
                outputIndex = outputsQuery.value(0).toInt();
                configStruct.moduleConfigs[x].outputs[outputIndex].itemID = outputsQuery.value(1).toInt();
                configStruct.moduleConfigs[x].outputs[outputIndex].itemType = outputsQuery.value(2).toInt();
                configStruct.moduleConfigs[x].outputs[outputIndex].onValue = outputsQuery.value(3).toInt();
                configStruct.moduleConfigs[x].outputs[outputIndex].flashingValue = outputsQuery.value(4).toInt();
            }
        }
    }

    QByteArray config((const char *)&configStruct, sizeof(PanelControllerConfigStruct));
    qDebug(QString("SENDING PANEL CONFIG DATA.  SIZE: %1.  Structure size: %2").arg(config.size()).arg(sizeof(PanelControllerConfigStruct)).toLatin1());
    return config;
}

QByteArray Database::getPanelRouteConfig(quint32 serialNumber)
{
    PanelControllerRouteConfigStruct configStruct;
    memset(&configStruct, 0, sizeof(PanelControllerRouteConfigStruct));

    QSqlQuery routesQuery(db);
    routesQuery.exec(QString("SELECT routeID, turnoutID, turnoutState FROM routeEntry WHERE RouteID IN (SELECT inputID FROM panelInputEntry WHERE inputType = 1 AND panelModuleID IN (SELECT panelModule.id FROM panelModule JOIN controller ON panelModule.controllerID = controller.ID WHERE serialNumber = %1)) ORDER BY routeID").arg(serialNumber));
    int routeIndex = 0;
    int routeEntryIndex = 0;
    int currentRouteID = 0;

    while (routesQuery.next())
    {
        configStruct.count++;
        if(currentRouteID == 0)
        {
            currentRouteID = routesQuery.value(0).toInt();
            configStruct.routes[routeIndex].routeID = currentRouteID;
        }
        if(currentRouteID == routesQuery.value(0).toInt())
        {
            configStruct.routes[routeIndex].entries[routeEntryIndex].turnoutID = routesQuery.value(1).toInt();
            configStruct.routes[routeIndex].entries[routeEntryIndex++].state = (TurnoutState)routesQuery.value(2).toInt();
        }
        else
        {
            currentRouteID = routesQuery.value(0).toInt();
            configStruct.routes[++routeIndex].routeID = currentRouteID;
        }
    }

    QByteArray config((const char *)&configStruct, sizeof(PanelControllerRouteConfigStruct));
    return config;
}

QString Database::getTurnoutName(int turnoutID)
{
    QString name;

    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    QSqlQuery query(db);

    bool ret = query.exec(QString("SELECT itemName FROM layoutItem WHERE id = %1").arg(turnoutID));

    if(ret == false)
        qDebug(query.lastError().text().toLatin1());

    while(query.next())
    {
       name = query.value(0).toString();
    }

    return name;
}

int Database::getTurnoutID(const QString &name)
{
    int id = -1;

    if(db.isValid() == false)
        db = QSqlDatabase::database();
    db.open();
    QSqlQuery query(db);

    bool ret = query.exec(QString("SELECT id FROM layoutItem WHERE itemName = '%1'").arg(name));

    if(ret == false)
        qDebug(query.lastError().text().toLatin1());

    while(query.next())
    {
       id = query.value(0).toInt();
    }

    return id;
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
              "turnoutID INTEGER NOT NULL, "
              "turnoutState INTEGER NOT NULL)");
    if(ret == false)
    {
        qDebug(query.lastError().text().toLatin1());
        emit logError(1, query.lastError().number(), query.lastError().text());
    }
    return ret;
}

bool Database::createPanelModuleTable()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS panelModule "
              "(id INTEGER primary key, "
              "controllerID INTEGER, "
              "panelIndex INTEGER, " // valid entries are 0 through 7
              "panelName VARCHAR(20)"
              ")");
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
              "controllerID INTEGER "
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
              "signalAspectConditionID INTEGER, "
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

bool Database::createSignalAspectCondition()
{
    bool ret = false;
    QSqlQuery query(db);
    ret = query.exec("CREATE TABLE IF NOT EXISTS signalAspectCondition "
              "(id INTEGER primary key, "
              "signalID INTEGER, "
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

void Database::updateDatabaseSchema(int /*currentVersion*/)
{
    // Add db schema changes here from version to versioin
    // If all changes are succesful, call setDBVersion() with the CurrentDatabaseVersion
    // REMEMBER!  currentVersion will = 0 if the database did not exist and was just
    // created....In this instance, the database will have the most recent database
    // schema, so no changes will be needed.
    setDBVersion(CurrentDatabaseVersion);
}

