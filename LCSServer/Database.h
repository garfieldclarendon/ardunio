#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>

#include "GlobalDefs.h"

const int CurrentDatabaseVersion = 4;

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = 0);

    ~Database(void);

    bool init(const QString &pathAndFile);
    void removeDatabase(void);
    bool checkDatabase(void);
    bool createDatabase(void);

    // Table functions
    int getControllerID(long serialNumber);
    unsigned long getSerialNumber(int controllerID);
    int addController(int controllerClass, const QString &controllerName, const QString &controllerDescription);
    QSqlDatabase getDatabase(void) const { return db; }
    int getNextID(const QString &tableName);
    int getDBVersion(void);
    void setDBVersion(int newVersion);

    QByteArray getMultiControllerConfig(quint32 serialNumber);
    QJsonArray getNotificationList(quint32 serialNumber);
    QByteArray getControllerModuleConfig(quint32 serialNumber, quint32 address);

    QString getDeviceConfig(int deviceID);
    void getDeviceProperties(int deviceID, QJsonObject &device);
    void getTurnoutConfig(int deviceID, QJsonObject &device);
    void getSignalConfig(int deviceID, QJsonObject &device);

    QString getSignalAspectConfig(int aspectID);
    QString getTurnoutName(int deviceID);
    int getdeviceID(const QString &name);
    DeviceClassEnum getDeviceClass(int deviceID);

    QList<int> getExcludeRouteList(int routeID);

    void getControllerIDAndName(quint32 serialNumber, int &deviceID, QString &controllerName);

    QJsonObject fetchItem(const QString &queryString);
    QJsonArray fetchItems(const QString &queryString);
    QJsonObject createJsonObject(QSqlQuery &query);
    QJsonArray createJsonArray(QSqlQuery &query);
    QSqlQuery executeQuery(const QString &queryString);

signals:
    void logError(int category, int code, const QString &errorText);

public slots:

private:
    void setupDb(void);
    bool createControllerTable(void);
    bool createControllerModuleTable(void);
    bool createClassTable(void);
    bool createLayoutItemTypeTable(void);
    bool createLayoutItemTable(void);
    bool createRouteTable(void);
    bool createRouteEntryTable(void);
    bool createPanelInputEntryTable(void);
    bool createPanelOutputEntryTable(void);
    bool createSignalConditionTable(void);
    bool createsignalAspect(void);
    bool createDeviceTable(void);
    bool createDevicePropertyTable(void);
    bool createPanelRouteTable(void);

    void updateDatabaseSchema(int currentVersion);
    void upgradeToVersion4(void);

    static QString fullPathAndFile;
    QSqlDatabase db;
};

#endif // DATABASE_H
