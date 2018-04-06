#include <FS.h>
#include <ArduinoJson.h>

#include "SignalDevice.h"
#include "NetworkManager.h"

SignalDevice::SignalDevice()
	: m_downloadConfig(false), m_lockout(false), m_updateValues(true), m_redMode(PinOn), 
	m_greenMode(PinOff), m_yellowMode(PinOff), m_currentBlinkTimeout(0), m_blinkingTimeout(750), 
	m_aspectCount(0), m_aspectDownload(NULL)
{
	memset(m_deviceStates, 0, sizeof(DeviceStateStruct) * MAX_SIGNAL_DEVICES);
  m_aspectDownload = NULL;
  m_aspectCount = 0;
}


SignalDevice::~SignalDevice()
{
}

void SignalDevice::process(ModuleData &moduleData, UDPMessage &, byte &)
{
//	DEBUG_PRINT("SignalDevice::process: START\n");
	if (m_aspectDownload != NULL)
		downloadAspects();

	byte dataA;
	byte dataB;
	dataA = moduleData.getByteA();
	dataB = moduleData.getByteB();

	if (m_lockout == false && m_updateValues)
	{
		updateValues();
	}

	// Set RED pin
	if (getPort() < 8)
		setPin(dataA, getPort(), m_redMode);
	else
		setPin(dataB, getPort(), m_redMode);

	// Set GREEN pin
	if (getPort() + 1 < 16)
	{
		if (getPort() + 1 < 8)
			setPin(dataA, getPort() + 1, m_greenMode);
		else
			setPin(dataB, getPort() + 1, m_greenMode);
	}

	// Set YELLOW pin
	if (getPort() + 2 < 16)
	{
		if (getPort() + 2 < 8)
			setPin(dataA, getPort() + 2, m_yellowMode);
		else
			setPin(dataB, getPort() + 2, m_yellowMode);
	}

	moduleData.setByteA(dataA);
	moduleData.setByteB(dataB);
//	DEBUG_PRINT("SignalDevice::process: DONE PIN: %d MODES: %d %d %d\n", getPort(), m_redMode, m_greenMode, m_yellowMode);
}

void SignalDevice::setPin(byte &data, byte pin, PinStateEnum state)
{
	byte p;
	if (pin < 8)
		p = pin;
	else
		p = pin - 8;
	if (state == PinOn)
	{
		bitWrite(data, p, LOW);
	}
	else if (state == PinFlashing)
	{
		static bool flash = true;
		unsigned long t = millis();
		if ((t - m_currentBlinkTimeout) > m_blinkingTimeout)
		{
			m_currentBlinkTimeout = t;
			if (flash)
				bitWrite(data, p, LOW);
			else
				bitWrite(data, p, HIGH);
			flash = !flash;
		}
	}
	else
	{
		bitWrite(data, p, HIGH);
	}
}

void SignalDevice::updateValues(void)
{
	DEBUG_PRINT("SignalDevice::updateValues(%d): START  TOTAL ASPECTS: %d\n", getID(), m_aspectCount);
	SignalAspectStruct aspect;

	m_redMode = PinOn;
	m_greenMode = PinOff;
	m_yellowMode = PinOff;

	byte x = 0;
	bool done = false;
	while (done == false)
	{
		if (loadAspect(x, &aspect))
		{
			DEBUG_PRINT("SignalDevice::updateValues(%d): ASPECT CONDITION COUNT %d\n", getID(), aspect.conditionCount);
			bool valid = aspect.conditionCount > 0;
			for (int index = 0; index < aspect.conditionCount; index++)
			{
				byte state = getCurrentState(aspect.conditions[index].deviceID);
				DEBUG_PRINT("SignalDevice::updateValues(%d): deviceID %d status %d == %d\n", getID(), aspect.conditions[index].deviceID, state, aspect.conditions[index].deviceState);
				if (state > 0)
				{
					if (aspect.conditions[index].operand == ConditionEquals)
					{
						if (state != aspect.conditions[index].deviceState)
						{
							valid = false;
						}
					}
					else if (aspect.conditions[index].operand == ConditionNotEquals)
					{
						if (state == aspect.conditions[index].deviceState)
						{
							valid = false;
						}
					}
					else
					{
						valid = false;
						DEBUG_PRINT("INVALID ConditionOperand! %d\n", aspect.conditions[index].operand);
						setInvalidAspect();
						done = true;
					}
					if (valid == false)
					{
						if (isNextConditionOR(&aspect, index + 1))
							valid = true;
						else
							break;
					}
					else
					{
						while (isNextConditionOR(&aspect, index + 1))
							index++;
					}
				}
				else
				{
					DEBUG_PRINT("INVALID DEVICE STATE!\n");
					valid = false;
					setInvalidAspect();
					done = true;
				}
			}

			if (valid)
			{
				m_redMode = aspect.redMode;
				m_greenMode = aspect.greenMode;
				m_yellowMode = aspect.yellowMode;
				done = true;
				DEBUG_PRINT("SignalDevice::updateValues(%d): DONE PIN: %d NEW MODES: %d %d %d\n", getID(), getPort(), m_redMode, m_greenMode, m_yellowMode);
			}
			x++;
			if (x >= m_aspectCount)
			{
				done = true;
			}
		}
		else
		{
			setInvalidAspect();
			done = true;
		}
	}
	m_updateValues = false;
	DEBUG_PRINT("SignalDevice::updateValues(%d): END\n", getID());
}

bool SignalDevice::isNextConditionOR(SignalAspectStruct *aspect, byte nextIndex)
{
	bool ret = false;

	if(nextIndex < aspect->conditionCount)
	{
		ret = aspect->conditions[nextIndex].connection == ConnectionOR;
	}
	DEBUG_PRINT("SignalDevice::isNextConditionOR(%d): nextIndex %d  conditionCount %d returning %d\n", getID(), nextIndex, aspect->conditionCount, ret);
	return ret;
}

void SignalDevice::setup(int deviceID, byte port)
{
	m_updateValues = true;
	setID(deviceID);
	setPort(port);

	String json = loadConfig();
	DEBUG_PRINT("%s\n", json.c_str());
	if (json.length() == 0)
		m_downloadConfig = true;
	else
		m_downloadConfig = (parseConfig(json, false) == false);
}

void SignalDevice::processUDPMessage(ModuleData &moduleData, const UDPMessage &message, UDPMessage &, byte &)
{
	if (message.getMessageID() == TRN_STATUS || message.getMessageID() == BLK_STATUS)
	{
		for (byte x = 0; x < MAX_SIGNAL_DEVICES; x++)
		{
			if (m_deviceStates[x].deviceID == message.getID())
			{
				m_deviceStates[x].state = message.getField(0);
				m_updateValues = true;
				break;
			}
		}
	}
	else if (message.getMessageID() == DEVICE_STATUS)
	{
		byte index = 0;
		while (message.getDeviceID(index) > 0)
		{
			for (byte x = 0; x < MAX_SIGNAL_DEVICES; x++)
			{
				if (m_deviceStates[x].deviceID == message.getDeviceID(index))
				{
					m_deviceStates[x].state = message.getDeviceStatus(index);
					m_updateValues = true;
					break;
				}
			}
			index++;
		}
	}
	else if (message.getMessageID() == SYS_LOCK_DEVICE && message.getID() == getID())
	{
		handleLockoutMessage(message);
	}
	else if (message.getMessageID() == SYS_RESET_DEVICE_CONFIG)
	{
		if (message.getID() == getID())
		{
			DEBUG_PRINT("processUDPMessage RESET DEVICE CONFIG\n", message.getID());
			m_downloadConfig = true;
			downloadConfig();
		}
	}
}

void SignalDevice::handleLockoutMessage(const UDPMessage &message)
{
	// First field contains the the lock flag
	controllerLockout(message.getField(0));
	// If locked and field1 contains 1, then fields 2,3 and 4 contain the red, green and yellow settings respectively.
	// If locked and field1 is 0, then the current pin settings are "locked" as is.
	if (m_lockout && message.getField(1) == 1)
	{
		m_redMode = (PinStateEnum)message.getField(2);
		m_greenMode = (PinStateEnum)message.getField(3);
		m_yellowMode = (PinStateEnum)message.getField(4);
	}

	// If locked is false, force the signal to re-canculate the proper aspect.
	if(m_lockout == false)
		m_updateValues = true;
}

byte SignalDevice::getCurrentState(int deviceID) const
{
	byte state = 0;

	for (byte x = 0; x < MAX_SIGNAL_DEVICES; x++)
	{
		if (m_deviceStates[x].deviceID == deviceID)
		{
			state = m_deviceStates[x].state;
			break;
		}
	}

	return state;
}

bool SignalDevice::parseConfig(String &jsonText, bool setVersion)
{
	DEBUG_PRINT("SignalDevice::parseConfig LENGTH: %d\n", jsonText.length());
	StaticJsonBuffer<1024> jsonBuffer;
	JsonObject &json = jsonBuffer.parseObject(jsonText);

	if (setVersion)
	{
		json["version"] = CONFIG_VERSION;
	}
	else if (json["version"] != (int)CONFIG_VERSION)
	{
		DEBUG_PRINT("parseConfig  WRONG VERSION.\n");
		return false;
	}

	JsonArray &aspects = json["aspects"];
	m_aspectCount = aspects.size();
	byte deviceIndex = 0;
	if (setVersion == false)
	{
		AspectDownloadStruct *current = m_aspectDownload;
		for (byte x = 0; x < aspects.size(); x++)
		{
			if (current != NULL)
			{
				if (current->aspectID == 0)
				{
					current->aspectID = aspects[x]["id"];
					DEBUG_PRINT("ADDING ASPECT TO DOWNLOAD ARRAY: %d\n", current->aspectID);
				}
				else
				{
					current->next = new AspectDownloadStruct;
					current->next->aspectID = aspects[x]["id"];
					current->next->next = NULL;
					DEBUG_PRINT("ADDING ASPECT TO DOWNLOAD ARRAY: %d\n", current->next->aspectID);
					current = current->next;
				}
			}
		}
	}
	JsonArray &devices = json["devices"];
	for (byte x = 0; x < devices.size(); x++)
	{
		m_deviceStates[x].deviceID = devices[x]["id"];
		DEBUG_PRINT("parseConfig  ADDING DEVICE: %d.\n", m_deviceStates[x].deviceID);
	}

	if (setVersion)
	{
		jsonText = "";
		json.printTo(jsonText);
	}
	return true;
}

void SignalDevice::serverFound(UDPMessage &outMessage, byte &)
{
	DEBUG_PRINT("SignalDevice::serverFound\n");
	if (m_downloadConfig)
	{
		downloadConfig();
	}
}

void SignalDevice::saveAspect(byte index, const SignalAspectStruct *aspect)
{
	String fileName(createFileName(index));
	DEBUG_PRINT("SAVE ASPECT %s\n", fileName.c_str());

	File f = SPIFFS.open(fileName, "w");

	if (f)
	{
		f.write((uint8_t*)aspect, sizeof(SignalAspectStruct));
		f.close();
	}
	else
	{
		DEBUG_PRINT("Error saving Signal Aspect file %s\n", fileName.c_str());
	}
}

bool SignalDevice::loadAspect(byte index, SignalAspectStruct *aspect)
{
	memset(aspect, 0, sizeof(SignalAspectStruct));
	bool ret = false;
	String fileName(createFileName(index));
	DEBUG_PRINT("LOAD ASPECT %s\n", fileName.c_str());

	File f = SPIFFS.open(fileName, "r");

	if (f)
	{
		f.read((uint8_t*)aspect, sizeof(SignalAspectStruct));
		f.close();
		ret = true;
	}
	else
	{
		DEBUG_PRINT("Signal Aspect file %s is missing or can not be opened\n", fileName.c_str());
	}

	return ret;
}

String SignalDevice::createFileName(byte index)
{
	String fileName("/SignalAspect_");
	fileName += getID();
	fileName += "_";
	fileName += index;
	fileName += ".dat";

	return fileName;
}

void SignalDevice::setInvalidAspect(void)
{
	DEBUG_PRINT("SignalDevice %d  SETTING INVALID ASPECT\n", getID());
	m_redMode = PinOn;
	m_greenMode = PinOn;
	m_yellowMode = PinOn;
}

void SignalDevice::downloadConfig(void)
{
	DEBUG_PRINT("SignalDevice::downloadConfig DOWNLOADING!\n");
	m_aspectDownload = new  AspectDownloadStruct;
	m_aspectDownload->aspectID = 0;
	m_aspectDownload->next = NULL;

	String json = NetManager.getDeviceConfig(getID());
  DEBUG_PRINT("SignalDevice::downloadConfig SIZE %d!\n", json.length());
	if (json.length() > 0)
	{
		parseConfig(json, true);
		saveConfig(json);
		m_downloadConfig = false;
		setup(getID(), getPort());
	}
}

void SignalDevice::downloadAspects(void)
{
  if(NetManager.getServerAddress() != (uint32_t)0)
  {
  	DEBUG_PRINT("SignalDevice::downloadAspects: TOTAL %d\n", m_aspectCount);
  	AspectDownloadStruct *download = m_aspectDownload;
  	byte x = 0;
  	while (download)
  	{
  		downloadAspect(download->aspectID, x++);
  		AspectDownloadStruct *toDelete = download;
  		download = download->next;
  		delete toDelete;
  	}
  	m_aspectDownload = NULL;
  }
}

void SignalDevice::downloadAspect(int aspectID, byte index)
{
	String json = NetManager.getSignalAspect(aspectID);
	DEBUG_PRINT("SignalDevice::downloadAspect: %d\n", aspectID);

	StaticJsonBuffer<2048> jsonBuffer;
	JsonObject &aspectObj = jsonBuffer.parseObject(json);

	SignalAspectStruct aspect;

	memset(&aspect, 0, sizeof(SignalAspectStruct));
	aspect.redMode = (PinStateEnum)(int)aspectObj["redMode"];
	aspect.greenMode = (PinStateEnum)(int)aspectObj["greenMode"];
	aspect.yellowMode = (PinStateEnum)(int)aspectObj["yellowMode"];
	JsonArray &conditions = aspectObj["conditions"];
	aspect.conditionCount = conditions.size();
	for (byte x = 0; x < conditions.size(); x++)
	{
		aspect.conditions[x].connection = (ConditionConnectionEnum)(int)conditions[x]["connectionType"];
		aspect.conditions[x].deviceID = conditions[x]["deviceID"];
		aspect.conditions[x].operand = (ConditionEnum)(int)conditions[x]["conditionOperand"];
		aspect.conditions[x].deviceState = conditions[x]["deviceState"];
		DEBUG_PRINT("parseConfig  CONDITION %d. DEVICEID %d\n", x, aspect.conditions[x].deviceID);
	}
	saveAspect(index, &aspect);
}
