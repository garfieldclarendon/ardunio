#pragma once

#include "SignalAspect.h"
#include "DeviceState.h"
#include "ConfigStructures.h"

class SignalCondition
{
public:
  enum SignalOperand 
  {
    Equals,
    NotEquals
  };
  SignalCondition(void) : deviceID(0), operand(Equals), deviceState(0) { }
  SignalCondition(const SignalCondition &other) { copy(other); }

  SignalCondition& operator=(const SignalCondition &other) { copy(other); return *this; }
  
  int getDeviceID(void) const { return deviceID; }
  void setDeviceID(int value) { deviceID = value; }
  SignalOperand getOperand(void) const { return operand; }
  void setOperand(SignalOperand value) { operand = value; }
  byte getDeviceState(void) const { return deviceState; }
  void setDeviceState(byte value) { deviceState = value; }
  
  void setConfig(const SignalConditionStruct &value) { deviceID = value.deviceID; operand = (SignalOperand)value.operand; deviceState = value.deviceState; }

private:
  void copy(const SignalCondition &other)
  {
    deviceID = other.deviceID;
    operand = other.operand;
    deviceState = other.deviceState;
  }
  int deviceID;
  SignalOperand operand;
  byte deviceState;    
};

class SignalAspectCondition
{
public:
  SignalAspect getAspect(void) const { return aspect; }
  void setAspect(const SignalAspect &value) { aspect = value; }
  SignalCondition getSignalCondition(byte index) { return conditions[index]; }

  void setConfig(const SignalAspectConditionStruct &value) 
  { 
	  aspect.setRedMode((SignalAspect::LightMode)value.aspect.redMode); 
	  aspect.setYellowMode((SignalAspect::LightMode)value.aspect.yellowMode);
	  aspect.setGreenMode((SignalAspect::LightMode)value.aspect.greenMode);
	  for (byte x = 0; x < MAX_SIGNAL_CONDITIONS; x++)
	  {
		  conditions[x].setConfig(value.conditions[x]);
	  }
  }
  bool evaluate(void) const
  {
    bool ret = true;
    if(conditions[0].getDeviceID() == 0)
      return false;
      
    for(byte x = 0; x < MAX_SIGNAL_CONDITIONS; x++)
    {
      byte currentState = DeviceState::getDeviceState(conditions[x].getDeviceID());
      if(conditions[x].getOperand() == SignalCondition::Equals)
      {
        if(currentState != conditions[x].getDeviceState())
        {
          ret = false;
          break;  
        }
      }
      else if(conditions[x].getOperand() == SignalCondition::NotEquals)
      {
        if(currentState == conditions[x].getDeviceState())
        {
          ret = false;
          break;  
        }
      }
    }
    
    return ret;
  }
private:
  SignalCondition conditions[MAX_SIGNAL_CONDITIONS];  
  SignalAspect aspect;
};

