#include <OneWire.h> // used by temperature sensors
#include <DallasTemperature.h> // used by temperature sensors
#include <Wire.h> // used by lps pressure and lsm303 sensors
#include <SoftwareSerial.h> // used by display
#include <LPS_Pressure.h>
#include <LSM303_Magnet.h>

// in milliseconds
const int TempRefreshInterval = 2000;
const int AltIMURefreshInterval = 500;

// temperature sensors OneWire signal pin
const int TemperatureSensorsPin = 3;

// temperature chip i/o
OneWire oneWire(TemperatureSensorsPin);
DallasTemperature tempSensors(&oneWire);

//DS18S20 hardware addresses
DeviceAddress InTemp =  { 0x28, 0x4E, 0x76, 0x74, 0x06, 0x00, 0x00, 0xFE };
DeviceAddress OutTemp = { 0x28, 0xFF, 0x7E, 0x74, 0x06, 0x00, 0x00, 0xF0 };

// temperature sensors resolution (1 to 12)
const int TempResolution = 10;

// screen TX and RX pins (RX pin is needed and yet not used)
const int ScreenTxPin = 10;
const int ScreenRxPin = 11;

// screen dim button pin
//const int ScreenDimBtnPin = 7;

// screen dim values
//const int ScreenDimOn = 140;
//const int ScreenDimOff = 150;

// AltIMU specific settings
LSM303::vector<int16_t> CompassMin = LSM303::vector<int16_t> {  +846,   +400,  -2451};
LSM303::vector<int16_t> CompassMax = LSM303::vector<int16_t> {  +869,   +423,  -2409};

// AltIMU sensors
LPS pressureSensor;
LSM303 magneticSensor;

// AltIMU sensors detection
boolean pressureSensorDetected = false;
boolean magneticSensorDetected = false;

// last screen symbol on the second line
char animationChars[] = "^<^>";
int loadingIndex = 0;

// screen output
SoftwareSerial screenSerial(ScreenTxPin, ScreenRxPin);

void setup(void) {
  // for debugging purposes
  // Serial.begin(9600);

  // setting up screen
  screenSerial.begin(9600);
  clearScreen();
  changeScreenBrightness(140);

  // setting up temperature sensors
  tempSensors.begin();

  // setting the resolution of the temperature sensors
  tempSensors.setResolution(InTemp, TempResolution);
  tempSensors.setResolution(OutTemp, TempResolution);

  // setting up Wire instance for pressure and magnetic
  // sensor
  Wire.begin();

  // setting up atmospheric pressure sensor
  if (pressureSensor.init()) {
    pressureSensorDetected = true;
    pressureSensor.enableDefault();
  }

  // setting up compass
  if (magneticSensor.init()) {
    magneticSensorDetected = true;
    magneticSensor.enableDefault();

    magneticSensor.m_min = CompassMin;
    magneticSensor.m_max = CompassMax;
  }

  //  pinMode(ScreenDimBtnPin, INPUT);
}

void loop(void) {
  // get temperature values from sensors
  tempSensors.requestTemperatures();

  // temperature is in Celsius
  float temperatureIn = tempSensors.getTempC(InTemp);
  float temperatureOut = tempSensors.getTempC(OutTemp);

  refreshDisplayFirstLine(temperatureIn, temperatureOut);

  // heading is in degrees
  float heading = -1.0;
  if (magneticSensorDetected) {
    magneticSensor.read();
    heading = magneticSensor.heading();
  }

  char *direction;
  getHeadingAsString(heading, &direction);

  // altitude is in meters
  int altitude = -1;
  if (pressureSensorDetected) {
    float pressure = pressureSensor.readPressureMillibars();
    float altitudeAsFloat = pressureSensor.pressureToAltitudeMeters(pressure);

    altitude = (int)altitudeAsFloat;
  }

  refreshDisplaySecondLine(direction, heading, altitude);

  waitAndUpdateAnimation(TempRefreshInterval, 4);
}
