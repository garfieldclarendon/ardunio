#include "DeviceHandler.h"
#include "DeviceManager.h"

DeviceHandler::DeviceHandler(DeviceClassEnum classCode, QObject *parent)
    : QObject(parent), m_classCode(classCode)
{
    DeviceManager::instance()->addDeviceHandler(m_classCode, this);
}

