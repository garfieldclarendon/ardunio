#pragma once

#include "message.h"
#include "globaldefs.h"
#include <EEPROM.h>


bool checkEEPROM(void)
{
	// return true if the first byte in memory is set to 0XAB.
	// this means the EEPROM memory has valid configuration data stored
	bool valid = true;
	if(EEPROM.read(0) != 0xAB)
	{
		Serial.println("-----------------------------");
		Serial.println("CONFIGURATION DATA IS INVALID!!!");
		Serial.println("-----------------------------");
		Serial.println("-----------------------------");
		valid = false;
		int controllerID = 255;
		EEPROM.put(CONTROLLER_ID_ADDRESS, controllerID);  
		EEPROM.put(CONTROLLER_ID_ADDRESS + sizeof(int), 0);
		EEPROM.put(CONTROLLER_ID_ADDRESS + (sizeof(int) * 2), 0);
		EEPROM.write(0, 0xAB);
		EEPROM.commit();
	}
	return valid;
}

int setControllerID(const Message &message, int currentControllerID)
{
  byte newControllerID(0);
  Serial.print("Current ControllerID: ");
  Serial.println(message.getControllerID());
  // Check to see if this controller is the target of the message
  if(currentControllerID == message.getControllerID())
  {
    // Get the controller ID
    newControllerID = message.getField(0);
    EEPROM.put(CONTROLLER_ID_ADDRESS, newControllerID);
	// Along with the new ControllerID, the message includes a new MAC address for this
	// controller.  Update the MAC address in memory
	//byte fieldIndex = 1;
	//for(byte x = MAC_ADDRESS_ADDRESS; x < MAC_ADDRESS_ADDRESS + 6; x++)
	//	EEPROM.put(x, message.getField(fieldIndex++));
  }
  
  Serial.print("Set New ControllerID: ");
  Serial.println(newControllerID);
  
  return newControllerID;
}

