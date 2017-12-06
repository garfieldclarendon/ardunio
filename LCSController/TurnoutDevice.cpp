#include "TurnoutDevice.h"
#include "TurnoutModule.h"
#include "NetworkManager.h"

TurnoutDevice::TurnoutDevice()
	: m_lastState(TrnUnknown), m_currentState(TrnUnknown), m_currentTimeout(0), m_currentData(0), m_lockedRoute(0), m_downloadConfig(false), m_motorBA(0), m_inputBA(false)
{
	memset(m_routeMap, 0, sizeof(TurnoutRouteStruct) * MAX_ROUTE_ENTRIES);
}

TurnoutDevice::~TurnoutDevice()
{
}

void TurnoutDevice::process(ModuleData &data)
{
//	DEBUG_PRINT("TURNOUTDEVICE::process:  CURRENT STATE: %d  DATA: %d\n", m_currentState, d);
	TurnoutState current = readCurrentState(data.getByteA());
	long t = millis();
	if (current != TrnUnknown && (t - m_currentTimeout) > TIMEOUT_INTERVAL)
	{
		if (current == m_lastState && m_lastState != m_currentState)
		{
//			DEBUG_PRINT("TURNOUTDEVICE::process ID: %d  LAST STATE: %d  CURRENT STATE: %d\n", getID(), m_lastState, m_currentState);
			if (m_currentState == TrnToDiverging && current == TrnNormal)
			{

			}
			else if (m_currentState == TrnToNormal && current == TrnDiverging)
			{

			}
			else
			{
				m_currentState = current;
//				DEBUG_PRINT("TURNOUTDEVICE::process ID: %d  State changed: %d  DATA: %d\n", getID(), m_currentState, data.getByteA());
				sendStatusMessage();
			}
		}
	}
	byte tmp = data.getByteA();
	bitWrite(tmp, m_motorA, bitRead(m_currentData, m_motorA));
	bitWrite(tmp, m_motorB, bitRead(m_currentData, m_motorB));
	data.setByteA(tmp);
	m_lastState = current;
}

void TurnoutDevice::setup(int deviceID, byte port)
{
	setID(deviceID);
	setPort(port);

	m_lockedRoute = 0;
	String json = loadConfig();
	DEBUG_PRINT("%s\n", json.c_str());
	if (json.length() == 0)
		m_downloadConfig = true;
	else
		m_downloadConfig = (parseConfig(json, false) == false);

	if (port == 0)
	{
		m_motorA = m_motorBA == 1 ? 1 : 0;
		m_motorB = m_motorBA == 1 ? 0 : 1;
		m_feedbackPinA = m_inputBA ? 3 : 2;
		m_feedbackPinB = m_inputBA ? 2 : 3;
	}
	else
	{
		m_motorA = m_motorBA == 1 ? 5 : 4;
		m_motorB = m_motorBA == 1 ? 4 : 5;
		m_feedbackPinA =  m_inputBA ? 7 : 6;
		m_feedbackPinB = m_inputBA ? 6 : 7;
	}
}

bool TurnoutDevice::parseConfig(String &jsonText, bool setVersion)
{
	DEBUG_PRINT("TurnoutDevice::parseConfig\n");
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

	if (json.containsKey("MOTORPIN"))
		m_motorBA = (int)json["MOTORPIN"];
	// Reverse the input pins if the motor pins are reversed
	m_inputBA = m_motorBA == 1;
	// Allow the configuration to override the above change
	if (json.containsKey("INPUTPIN"))
		m_inputBA = json["INPUTPIN"] == 1;

	JsonArray &routes = json["routes"];
	for (byte x = 0; x < routes.size(); x++)
	{
		m_routeMap[x].routeID = routes[x]["routeID"];
		m_routeMap[x].state = (TurnoutState)(int)routes[x]["turnoutState"];
	}
	if (setVersion)
	{
		jsonText = "";
		json.printTo(jsonText);
	}
	DEBUG_PRINT("TurnoutDevice::parseConfig: %d INPUTFLAG: %d MOTORPIN %d\n", getID(), m_inputBA, m_motorBA);
	return true;
}

void TurnoutDevice::sendStatusMessage(void)
{
	sendStatusMessage(m_currentState);
}

void TurnoutDevice::sendStatusMessage(TurnoutState newState)
{
	UDPMessage message;

	message.setID(getID());
	message.setMessageID(TRN_STATUS);
	message.setField(0, newState);

//	DEBUG_PRINT("TurnoutDevice::sendStatusMessage:%d STATUS %d\n", getID(), newState);
	NetManager.sendUdpMessage(message);
}

void TurnoutDevice::processUDPMessage(ModuleData &data, const UDPMessage &message)
{
	TurnoutState newState(TrnUnknown);
	int newLockoutRoute = 0;
	if (message.getMessageID() == TRN_ACTIVATE && message.getID() == getID())
	{
		newState = (TurnoutState)message.getField(0);
		if (newState == TrnUnknown)
		{
			if (m_currentState == TrnNormal || m_currentState == TrnToNormal)
				newState = TrnDiverging;
			else
				newState = TrnNormal;
		}
//		DEBUG_PRINT("processUDPMessage setTurnout for deviceID: %d  New State: %d\n", getID(), newState);
	}
	else if (message.getMessageID() == TRN_ACTIVATE_ROUTE)
	{
		DEBUG_PRINT("processUDPMessage deviceID: %d setTurnout for routeID: %d\n", getID(), message.getID());
		newState = getTurnoutStateForRoute(message.getID());
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
			byte data = 0;
			setTurnout(data, TrnNormal);
		}
	}
	else if (message.getMessageID() == SYS_LOCK_DEVICE && message.getID() == getID())
	{
		setLockoutRoute(message.getField(0) == 1 ? 9999 : 0);
	}
	else if (message.getMessageID() == SYS_LOCK_ROUTE)
	{
		newState = getTurnoutStateForRoute(message.getID());
		if (newState != TrnUnknown)
		{
			if (message.getField(0) == 0)
			{
				// We're unlocking a previously locked route.  Clear the lock flag but do not actually set
				// the turnout to the new route.  ONLY UNLOCK AND ONLY IF THE ROUTE MATCHES THE ORIGNAL LOCKED ROUTE!
				// Lockouts have to be sent in pairs.  First a lock, then an unlock.  Then the turnout can be set and, optionally,
				// locked to a new route
				if (m_lockedRoute == message.getID())
					setLockoutRoute(0);
				newState = TrnUnknown;
			}
			else if (m_lockedRoute == 0)
			{
				// This turnout is included in the locked route.  NOTE:  Only set newLockoutRoute if the turnout is not currently locked to
				//  a different route
				newLockoutRoute = message.getID();
			}
		}
	}


	if (m_lockedRoute == 0 && newState != TrnUnknown && newState != m_currentState)
	{
		byte tmp = data.getByteA();
		setTurnout(tmp, newState);
		data.setByteA(tmp);
		if (newState == TrnNormal)
			m_currentState = TrnToNormal;
		else
			m_currentState = TrnToDiverging;
		sendStatusMessage();
	}
	if (m_lockedRoute == 0)
		setLockoutRoute(newLockoutRoute);
}

TurnoutState TurnoutDevice::getTurnoutStateForRoute(int routeID)
{
	TurnoutState state(TrnUnknown);

	for (byte x = 0; x < MAX_ROUTE_ENTRIES; x++)
	{
		if (m_routeMap[x].routeID == routeID)
		{
			state = m_routeMap[x].state;
			break;
		}
	}
	return state;
}
void TurnoutDevice::networkOnline(void)
{
}

void TurnoutDevice::serverFound(void)
{
	if (m_downloadConfig)
	{
		String json = NetManager.getDeviceConfig(getID());
		if (json.length() > 0)
		{
			parseConfig(json, true);
			saveConfig(json);
			m_downloadConfig = false;
			setup(getID(), getPort());
			byte data = 0;
			setTurnout(data, TrnNormal);
		}
	}
	sendStatusMessage();
}

TurnoutState TurnoutDevice::readCurrentState(byte data)
{
	TurnoutState current(TrnUnknown);

	if (bitRead(data,m_feedbackPinA) == LOW)
	{
		current = TrnNormal;
	}
	else if (bitRead(data, m_feedbackPinB) == LOW)
	{
		current = TrnDiverging;
	}

	return current;
}

void TurnoutDevice::setTurnout(byte &data, TurnoutState newState)
{
//	DEBUG_PRINT("BEFORE SETTING TURNOUT A: %d B: %d DATA: %d\n", m_motorA, m_motorB, data);
	if (newState == TrnDiverging || newState == TrnToDiverging)
	{
//		DEBUG_PRINT("SETTING TURNOUT TO DIVERGING\n");
		bitWrite(data, m_motorA, HIGH);
		bitWrite(data, m_motorB, LOW);
	}
	else if (newState == TrnNormal || newState == TrnToNormal)
	{
//		DEBUG_PRINT("SETTING TURNOUT TO NORMAL\n");
		bitWrite(data, m_motorA, LOW);
		bitWrite(data, m_motorB, HIGH);
	}
	m_currentData = data;
//	DEBUG_PRINT("SETTING TURNOUT A: %d B: %d DATA: %d\n", m_motorA, m_motorB, data);
}
