#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>

const int CurrentDatabaseVersion = 3;

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
    int addController(int controllerClass, const QString &controllerName, const QString &controllerDescription);
    QSqlDatabase getDatabase(void) const { return db; }
    int getNextID(const QString &tableName);
    int getDBVersion(void);
    void setDBVersion(int newVersion);

    QByteArray getTurnoutConfig(quint32 serialNumber, int moduleIndex);
    QByteArray getPanelConfig(quint32 serialNumber);
    QByteArray getPanelRouteConfig(quint32 serialNumber);
    QByteArray getSignalConfig(quint32 serialNumber, int moduleIndex);
    QByteArray getBlockConfig(quint32 serialNumber, int moduleIndex);
    QByteArray getMultiControllerConfig(quint32 serialNumber);
    QByteArray getControllerModuleConfig(quint32 serialNumber, quint32 moduleIndex);

    QString getTurnoutName(int turnoutID);
    int getTurnoutID(const QString &name);

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
    bool createSignalTable(void);
    bool createSignalConditionTable(void);
    bool createSignalAspectCondition(void);
    bool createBlockTable(void);
    bool createDeviceTable(void);
    bool createDevicePropertyTable(void);
    bool createPanelRouteTable(void);

    void updateDatabaseSchema(int currentVersion);

    static QString fullPathAndFile;
    QSqlDatabase db;
};

#endif // DATABASE_H
