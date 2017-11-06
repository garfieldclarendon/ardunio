#include <Arduino.h>
#include <ArduinoJson.h>

#include "TurnoutModule.h"
#include "network.h"

TurnoutModule::TurnoutModule(void)
	: m_currentState(0)

{
	memset(&m_config, 0, sizeof(TurnoutControllerConfigStruct));
	m_turnouts[0].setConfig(&m_config.turnout1);
	m_turnouts[1].setConfig(&m_config.turnout2);
}

byte TurnoutModule::getIODirConfig(void) const
{
	byte iodir = 0;

	bitWrite(iodir, motorAPin1, 0);
	bitWrite(iodir, motorBPin1, 0);
	bitWrite(iodir, motorAPin2, 0);
	bitWrite(iodir, motorBPin2, 0);
	bitWrite(iodir, feedbackAPin1, 1);
	bitWrite(iodir, feedbackBPin1, 1);
	bitWrite(iodir, feedbackAPin2, 1);
	bitWrite(iodir, feedbackBPin2, 1);

	return iodir;
}

void TurnoutModule::setup(void)
{
	setup(0, motorAPin1, motorBPin1, feedbackAPin1, feedbackBPin1);
	setup(1, motorAPin2, motorBPin2, feedbackAPin2, feedbackBPin2);

	m_currentState = 0;
	// Force the diverging pin ON so that it resets to normal
	// if the turnout happens to be set to the diverging route
	bitWrite(m_currentState, feedbackAPin1, 1);
	bitWrite(m_currentState, feedbackBPin1, 0);
	bitWrite(m_currentState, feedbackAPin2, 1);
	bitWrite(m_currentState, feedbackBPin2, 0);

	DEBUG_PRINT("TurnoutModule::setup  Turnout1\n");
	m_turnouts[0].setTurnout(m_config.turnout1.inputPinSetting);
	DEBUG_PRINT("TurnoutModule::setup  Turnout2\n");
	m_turnouts[1].setTurnout(m_config.turnout2.inputPinSetting);

	DEBUG_PRINT("TurnoutModule::setup  SETTING PINMODE\n");
	pinMode(motor1_pinA, OUTPUT);
	pinMode(motor2_pinA, OUTPUT);
	pinMode(motor1_pinB, OUTPUT);
	pinMode(motor2_pinB, OUTPUT);

	pinMode(normal1_pin, INPUT);
	pinMode(diverge1_pin, INPUT);
	pinMode(normal2_pin, INPUT);
	pinMode(diverge2_pin, INPUT);
}

void TurnoutModule::setupWire(byte address)
{
	DEBUG_PRINT("TurnoutModule::setupWire  Address: %d\n", address);
	setAddress(address);

	setup(0, motorAPin1, motorBPin1, feedbackAPin1, feedbackBPin1);
	setup(1, motorAPin2, motorBPin2, feedbackAPin2, feedbackBPin2);

	m_currentState = 0;
	// Force the diverging pin ON so that it resets to normal
	// if the turnout happens to be set to the diverging route
	bitWrite(m_currentState, feedbackAPin1, 1);
	bitWrite(m_currentState, feedbackBPin1, 0);
	bitWrite(m_currentState, feedbackAPin2, 1);
	bitWrite(m_currentState, feedbackBPin2, 0);

	DEBUG_PRINT("TurnoutModule::setup  Turnout1\n");
	m_turnouts[0].setTurnout(m_config.turnout1.inputPinSetting);
	DEBUG_PRINT("TurnoutModule::setup  Turnout2\n");
	m_turnouts[1].setTurnout(m_config.turnout2.inputPinSetting);

	expanderBegin();
	byte iodir = getIODirConfig();
	expanderWrite(IODIR, iodir);
	delay(100);
	byte data = getCurrentState();
	expanderWrite(GPIO, data);
}

void TurnoutModule::setup(byte index, byte motorAPin, byte motorBPin, byte feedbackAPin, byte feedbackBPin)
{
	m_turnouts[index].setup(motorAPin, motorBPin, feedbackAPin, feedbackBPin);
}

void TurnoutModule::processNoWire(void)
{
	byte data(getCurrentState());
//	DEBUG_PRINT("processNoWire:  CURRENT_STATE %d != %d\n", m_currentState, data);
	readPins(data);

	process(data);
	setPins();
}

bool TurnoutModule::process(byte &data)
{
	bool sendStatus = false;
	for (byte x = 0; x < MAX_TURNOUTS; x++)
		if (m_turnouts[x].process(data))
			sendStatus = true;
	
	if (m_currentState != data)
	{
//		DEBUG_PRINT("process:  CURRENT_STATE %d != %d\n", m_currentState, data);
		m_currentState = data;
	}

	bool ret = false;
	if (sendStatus)
	{
		sendStatusMessage();
		ret = true;
	}

	return ret;
}

void TurnoutModule::setTurnout(byte index, byte motorPinSetting)
{
	m_turnouts[index].setTurnout(motorPinSetting);
}

void TurnoutModule::netModuleCallbackNoWire(NetActionType action, byte address, const JsonObject &json)
{
	byte data(getCurrentState());
	readPins(data);
	netModuleCallback(action, address, json, data);
	setPins();
}

void TurnoutModule::netModuleCallback(NetActionType action, byte address, const JsonObject &json, byte &data)
{
	if (action == NetActionUpdate)
	{
		int port = json["port"];
		byte motorPinSetting = json["motorPinSetting"];

		setTurnout(port, motorPinSetting);
		data = m_currentState;
	}

	createCurrentStatusJson();
}

void TurnoutModule::netModuleConfigCallback(NetActionType action, byte address, const JsonObject &json)
{
	if (action == NetActionUpdate)
	{
		JsonArray &a = json["motorPinSettings"];

		for (byte x = 0; x < a.size(); x++)
		{
			byte motorPinSetting = a[x]["motorPinSetting"];
			byte port = a[x]["port"];
			DEBUG_PRINT("TurnoutModule::netModuleConfigCallback PORT %d  MOTOR PIN SETTNG %d\n", port, motorPinSetting);
			if (port == 0)
				m_config.turnout1.inputPinSetting = motorPinSetting;
			else
				m_config.turnout2.inputPinSetting = motorPinSetting;
		}
	}
}

void TurnoutModule::sendStatusMessage(void)
{
	createCurrentStatusJson();
//	DEBUG_PRINT("sendStatusMessage:  %s\n", json.c_str());
//	Network.sendUdpMessageToServer(NetActionUpdate, ClassTurnout, getAddress(), json);
}

String TurnoutModule::createCurrentStatusJson(void)
{
	String json;
	StaticJsonBuffer<256> jsonBuffer;
	JsonObject &root = jsonBuffer.createObject();
	root["messageUri"] = "/controller/module";
	root["address"] = getAddress();
	root["class"] = (int)ClassTurnout;
	root["action"] = (int)NetActionUpdate;
	JsonArray &turnouts = root.createNestedArray("turnouts");

	for (byte x = 0; x < MAX_TURNOUTS; x++)
	{
		JsonObject& turnout = turnouts.createNestedObject();
		turnout["port"] = x;
		turnout["feedbackA"] = m_turnouts[x].getFeedbackA();
		turnout["feedbackB"] = m_turnouts[x].getFeedbackB();
	}
	
	root.printTo(json);
	Network.sendMessageToServer(root);

	return json;
}
//const byte motorAPin1 = 0;
//const byte motorBPin1 = 1;
//const byte motorAPin2 = 4;
//const byte motorBPin2 = 5;
//const byte feedbackAPin1 = 2;
//const byte feedbackBPin1 = 3;
//const byte feedbackAPin2 = 6;
//const byte feedbackBPin2 = 7;

void TurnoutModule::setPins(void)
{
	byte data = getCurrentState();
	//	DEBUG_PRINT("DATA %d\n", data);

	digitalWrite(motor1_pinA, bitRead(data, motorAPin1));
//	DEBUG_PRINT("Morotr1_A %d\n", bitRead(data, motorAPin1));
	digitalWrite(motor1_pinB, bitRead(data, motorBPin1));
//	DEBUG_PRINT("Morotr1_B %d\n", bitRead(data, motorBPin1));
	digitalWrite(motor2_pinA, bitRead(data, motorAPin2));
	//	DEBUG_PRINT("Morotr2_A %d\n", bitRead(data, motor2_logicalPinA));
	digitalWrite(motor2_pinB, bitRead(data, motorBPin2));
	//	DEBUG_PRINT("Morotr2_B %d\n", bitRead(data, motor2_logicalPinB));
}

void TurnoutModule::readPins(byte &data)
{
	bitWrite(data, feedbackBPin1, digitalRead(diverge1_pin));
	bitWrite(data, feedbackAPin1, digitalRead(normal1_pin));
	bitWrite(data, feedbackBPin2, digitalRead(diverge2_pin));
	bitWrite(data, feedbackAPin2, digitalRead(normal2_pin));
}
