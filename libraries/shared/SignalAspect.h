#pragma once

class SignalAspect
{
public:
  enum LightMode
  {
    Off,
    On,
    Flashing  
  };
  
  SignalAspect(void) : red(On), yellow(Off), green(Off) { }
  SignalAspect(const SignalAspect &other) { copy(other); }

  SignalAspect& operator=(const SignalAspect &other) { copy(other); return *this; }
  bool operator==(const SignalAspect &other) { return (other.red == this->red && other.yellow == this->yellow && other.green == this->green); }
  bool operator!=(const SignalAspect &other) { return (other.red != this->red || other.yellow != this->yellow || other.green != this->green); }

  LightMode getRedMode(void) const { return red; }
  void setRedMode(LightMode value) { red = value; }
  LightMode getYellowMode(void) const { return yellow; }
  void setYellowMode(LightMode value) { yellow = value; }
  LightMode getGreenMode(void) const { return green; }
  void setGreenMode(LightMode value) { green = value; }
  
private:
  void copy(const SignalAspect &other)
  {
    red = other.red;
    yellow = other.yellow;
    green = other.green;
  }
  
  LightMode red;
  LightMode yellow;
  LightMode green;
};

