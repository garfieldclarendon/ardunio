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
}


SignalDevice::~SignalDevice()
{
}

void SignalDevice::process(ModuleData &moduleData)
{
//	DEBUG_PRINT("SignalDevice::process: START\n");
	byte dataA;
	byte dataB;
	dataA = moduleData.getByteA();
	dataB = moduleData.getByteB();

	if (/* m_lockout == false && */ m_updateValues)
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
		if (t - m_currentBlinkTimeout > m_blinkingTimeout)
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
	DEBUG_PRINT("SignalDevice::updateValues: START\n");
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
			DEBUG_PRINT("SignalDevice::updateValues: ASPECT COUNT %d  %d %d %d\n", aspect.conditionCount, aspect.redMode, aspect.greenMode, aspect.yellowMode);
			bool valid = aspect.conditionCount > 0;
			for (int index = 0; index < aspect.conditionCount; index++)
			{
				byte state = getCurrentState(aspect.conditions[index].deviceID);
				DEBUG_PRINT("SignalDevice::updateValues: deviceID %d status %d == %d\n", aspect.conditions[index].deviceID, state, aspect.conditions[index].deviceState);
				if (state > 0)
				{
					if (aspect.conditions[index].operand == ConditionEquals)
					{
						if (state != aspect.conditions[index].deviceState)
							valid = false;
					}
					else if (aspect.conditions[index].operand == ConditionNotEquals)
					{
						if (state == aspect.conditions[index].deviceState)
							valid = false;
					}
					else
					{
						valid = false;
						setInvalidAspect();
						done = true;
					}
				}
				else
				{
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
				DEBUG_PRINT("SignalDevice::updateValues: DONE PIN: %d NEW MODES: %d %d %d\n", getPort(), m_redMode, m_greenMode, m_yellowMode);
			}
			x++;
			if (x >= m_aspectCount)
				done = true;
		}
		else
		{
			setInvalidAspect();
			done = true;
		}
	}
	m_updateValues = false;
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

void SignalDevice::processUDPMessage(ModuleData &moduleData, const UDPMessage &message)
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
	else if (message.getMessageID() == SYS_LOCK_DEVICE && message.getID() == getID())
	{
		controllerLockout(message.getField(0));
	}
	else if (message.getMessageID() == SYS_RESET_DEVICE_CONFIG)
	{
		if (message.getID() == getID())
		{
			DEBUG_PRINT("processUDPMessage RESET DEVICE CONFIG\n", message.getID());
			String json = NetManager.getDeviceConfig(getID());
			parseConfig(json, true);
			saveConfig(json);
			setup(getID(), getPort());
		}
	}
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
	DEBUG_PRINT("SignalDevice::parseConfig\n");
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

	SignalAspectStruct aspect;

	JsonArray &aspects = json["aspects"];
	m_aspectCount = aspects.size();
	byte deviceIndex = 0;
	for (byte x = 0; x < aspects.size(); x++)
	{
		memset(&aspect, 0, sizeof(SignalAspectStruct));
		if (m_aspectDownload != NULL)
		{
			if (m_aspectDownload->aspectID == 0)
			{
				m_aspectDownload->aspectID = aspects[x]["aspectID"];
				DEBUG_PRINT("ADDING ASPECT TO DOWNLOAD ARRAY: %d", m_aspectDownload->aspectID);
			}
			else
			{
				m_aspectDownload->next = new AspectDownloadStruct;
				m_aspectDownload->next->aspectID = aspects[x]["aspectID"];
				m_aspectDownload->next->next = NULL;
				DEBUG_PRINT("ADDING ASPECT TO DOWNLOAD ARRAY: %d", m_aspectDownload->next->aspectID);
			}
		}
	}
	JsonArray &devices = json["devices"];
	for (byte x = 0; x < devices.size(); x++)
	{
		m_deviceStates[x].deviceID = devices[x]["deviceID"];
		DEBUG_PRINT("parseConfig  ADDING DEVICE: %d.\n", m_deviceStates[x].deviceID);
	}

	if (setVersion)
	{
		jsonText = "";
		json.printTo(jsonText);
	}
	return true;
}

void SignalDevice::serverFound(void)
{
	DEBUG_PRINT("SignalDevice::serverFound\n");
	if (m_downloadConfig)
	{
		DEBUG_PRINT("SignalDevice::serverFound DOWNLOADING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		m_aspectDownload = new  AspectDownloadStruct;
		m_aspectDownload->aspectID = 0;
		m_aspectDownload->next = NULL;

		String json = NetManager.getDeviceConfig(getID());
		if (json.length() > 0)
		{
			parseConfig(json, true);
			saveConfig(json);
			m_downloadConfig = false;
			setup(getID(), getPort());
			downloadAspects();
		}
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
//	DEBUG_PRINT("LOAD ASPECT %s\n", fileName.c_str());

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

void SignalDevice::downloadAspects(void)
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

void SignalDevice::downloadAspect(int aspectID, byte index)
{
	String json = NetManager.getSignalAspect(aspectID);
	DEBUG_PRINT("SignalDevice::downloadAspect: %d\n", aspectID);
	StaticJsonBuffer<1024> jsonBuffer;
	JsonObject &aspectObj = jsonBuffer.parseObject(json);

	SignalAspectStruct aspect;

	memset(&aspect, 0, sizeof(SignalAspectStruct));
	aspect.redMode = (PinStateEnum)(int)aspectObj["redMode"];
	aspect.greenMode = (PinStateEnum)(int)aspectObj["greenMode"];
	aspect.yellowMode = (PinStateEnum)(int)aspectObj["yellowMode"];
	JsonArray &conditions = aspectObj["conditions"];
	aspect.conditionCount = conditions.size();
	for (byte index = 0; index < conditions.size(); index++)
	{
		DEBUG_PRINT("parseConfig  CONDITION %d.\n", index);
		aspect.conditions[index].deviceID = conditions[index]["deviceID"];
		aspect.conditions[index].operand = (ConditionEnum)(int)conditions[index]["conditionOperand"];
		aspect.conditions[index].deviceState = conditions[index]["deviceState"];
	}
	saveAspect(index, &aspect);
}
