#ifndef CONTROLLERMANAGER_H
#define CONTROLLERMANAGER_H

#include <QObject>
#include <QMap>

#include "GlobalDefs.h"
#include "UDPMessage.h"

class ControllerEntry;

/// ControllerManager
/// \brief  Singleton class manitoring the current state of controllers.
///
/// This class monitors UDP messages collecting information about the current state of LCS controllers.  When a controller's online status changes,
/// a controllerStatusChanged signal is emitted.  On a SYS_CONTROLLER_ONLINE message, the controllerAdded signal is emitted.
class ControllerManager : public QObject
{
    Q_OBJECT
    /// Contstructor
    explicit ControllerManager(QObject *parent = 0);
public:
    /// Deststructor
    ~ControllerManager(void);

    /// Returns/creates the singleton instance
    static ControllerManager *instance(void);
    /// Returns the controller's serial number at the given index.
    long getConnectionSerialNumber(int index) const;
    /// Fills version and status with the current information for a controller.
    /// @param serialNumber long Serial number of the controller to lookup.
    void getConnectedInfo(long serialNumber, QString &version, ControllerStatusEnum &status);
    /// Returns the count of connected controllers.
    int getConnectionCount(void) const { return m_controllerMap.keys().count(); }

signals:
    /// Emitted when a SYS_CONTROLLER_ONLINE UDP message is received.
    void controllerAdded(long serialNumber);
    /// Currently, this signal is not emitted as there is no way (currently) for the detection of a controller going offline.
    void controllerRemoved(long serialNumber);
    /// Emitted when a SYS_CONTROLLER_ONLINE and SYS_RESTARTING UDP messages are received.
    void controllerStatusChanged(long serialNumber, ControllerStatusEnum newStatus);

public slots:
    /// Returns the serial number for the given controllerID.
    unsigned long getSerialNumber(int controllerID);

protected slots:
    /// UDP Message handler.
    void newUDPMessage(const UDPMessage &message);

private:

    static ControllerManager *m_instance;

    QMap<long, ControllerEntry *> m_controllerMap;
};

#endif // CONTROLLERMANAGER_H
