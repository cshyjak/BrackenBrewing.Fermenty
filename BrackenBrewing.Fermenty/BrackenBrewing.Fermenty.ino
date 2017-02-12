#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Attach the serial display's RX line to digital pin 2
SoftwareSerial _display(3, 2);
OneWire _oneWire(10);  // on pin 10
DallasTemperature _sensors(&_oneWire);

const int _relayPin = 3;
unsigned long _relayStartTime = 0;
bool _relayState = false;
const int _minimumRelayToggleTime = 15000;
double _desiredTemperature = 18.0;
char _msgBuffer[20];
const int _potPin = 0;

void setup() {
  Serial.begin(9600);

  _display.begin(9600);
  delay(500); // wait for display to boot up
  
  _sensors.begin();
  
  pinMode(_relayPin, OUTPUT);
  digitalWrite(_relayPin, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  _display.write(254); // cursor to beginning of first line
  _display.write(128);

  _display.write("DT:      IT:    "); // clear display + legends
  _display.write("CT:      OT:    ");
}

void loop() {
  _sensors.requestTemperatures();
  _desiredTemperature = ReadDesiredTemperature();
  
  WriteToScreen(1, 4, dtostrf(_desiredTemperature, 4, 1, _msgBuffer));
  WriteToScreen(2, 4, dtostrf(_sensors.getTempCByIndex(0), 4, 1, _msgBuffer));
  WriteToScreen(1, 13, dtostrf(_sensors.getTempCByIndex(1), 4, 1, _msgBuffer));
  WriteToScreen(2, 13, dtostrf(_sensors.getTempCByIndex(2), 4, 1, _msgBuffer));
  
  if(_sensors.getTempCByIndex(0) < _desiredTemperature)
  {
    ToggleRelay(false);
  }
  else
  {
    ToggleRelay(true);
  }

  Serial.print("{\"DesiredTemp\":");
  Serial.print(dtostrf(_desiredTemperature, 4, 1, _msgBuffer));
  Serial.print(",\"CurrentTemp\":");
  Serial.print(dtostrf(_sensors.getTempCByIndex(0), 4, 1, _msgBuffer));
  Serial.print(",\"IceZoneTemp\":");
  Serial.print(dtostrf(_sensors.getTempCByIndex(1), 4, 1, _msgBuffer));
  Serial.print(",\"AirTemp\":");
  Serial.print(dtostrf(_sensors.getTempCByIndex(2), 4, 1, _msgBuffer));
  Serial.print(",\"RelayState\":");
  Serial.print(_relayState);
  Serial.println("}");
}

void WriteToScreen(int line, int startPosition, const char* text) {
  int startCharacter = 128;

  if (line == 2) {
    startCharacter = 192;
  }

  startCharacter = startCharacter + startPosition - 1;

  _display.write(254);
  _display.write(startCharacter);
  _display.write(text);
}

void ToggleRelay(bool desiredState)
{
  if(_relayState != desiredState)
  {
    if (_relayState == false)
    {
      digitalWrite(_relayPin, HIGH);
      digitalWrite(LED_BUILTIN, HIGH);
      _relayStartTime = millis();
      _relayState = true;
    }
    else
    {
      if((unsigned long)(millis() - _relayStartTime) >= _minimumRelayToggleTime) 
      {
        digitalWrite(_relayPin, LOW);
        digitalWrite(LED_BUILTIN, LOW);
        _relayState = false;
      }
    }
  }
}

double ReadDesiredTemperature()
{
  int reading = analogRead(_potPin);
  return (((reading / 1023.0) * 20.0) + 5.0);
}

