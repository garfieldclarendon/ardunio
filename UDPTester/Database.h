#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>

const int CurrentDatabaseVersion = 2;

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = 0);

    ~Database(void);

    bool init(const QString &pathAndFile);
    bool checkDatabase(void);
    bool createDatabase(void);

    // Table functions
    int getControllerID(long serialNumber);
    int addController(int controllerClass, const QString &controllerName, const QString &controllerDescription);
    QSqlDatabase getDatabase(void) const { return db; }
    int getNextID(const QString &tableName);
    int getDBVersion(void);
    void setDBVersion(int newVersion);

    QByteArray getTurnoutConfig(quint32 serialNumber);
    QByteArray getPanelConfig(quint32 serialNumber);
    QByteArray getPanelRouteConfig(quint32 serialNumber);
    QByteArray getSignalConfig(quint32 serialNumber);
    QByteArray getBlockConfig(quint32 serialNumber);

    QString getTurnoutName(int turnoutID);
    int getTurnoutID(const QString &name);

signals:
    void logError(int category, int code, const QString &errorText);

public slots:

private:
    bool createControllerTable(void);
    bool createClassTable(void);
    bool createLayoutItemTypeTable(void);
    bool createLayoutItemTable(void);
    bool createRouteTable(void);
    bool createRouteEntryTable(void);
    bool createPanelModuleTable(void);
    bool createPanelInputEntryTable(void);
    bool createPanelOutputEntryTable(void);
    bool createSignalTable(void);
    bool createSignalConditionTable(void);
    bool createSignalAspectCondition(void);
    bool createBlockTable(void);

    void updateDatabaseSchema(int currentVersion);

    QString fullPathAndFile;
    QSqlDatabase db;
};

#endif // DATABASE_H
