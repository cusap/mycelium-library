#pragma once

/* mycelium library by Dhvanil Shah
 */

// This will load the definition for common Particle variable types
#include "Particle.h"
#include "Adafruit_DHT.h"

#define SEC_TO_MILLIS 1000UL
#define SEC_ONE_DAY 86400

#define BATT_OP 0x01
#define LIQUID_LEVEL 0x02
// #define TDS 0x04
// #define PH 0x08

#define LIGHTS 0x01
#define PUMP 0x02
// #define FAN 0x04

struct System
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  float timezone;
  time_t timeUpdated;
  int cycleDay;
};

struct ActionRequest
{
  String action = "";
  String color = "";
  String id = "";
  unsigned int timeout = 0;
  int priority = 0;
  bool active = false;
};

typedef struct
{
  int lowThres;
  int highThresh;
  int lastReading;
  int (*check)();
  bool activeIssue;
  String name;
} Sensor;

// This is your main class that users will import into their application
class Mycelium
{
public:
  /**
   * Constructor
   */
  Mycelium(String type, String product);

  /**
   * Begin Method
   * Must be called inside of setup
   * @param type - Defines the type of hydroponics 
   * @param bitmask - Defines the specificities of the system
   */
  void begin();

  /**
   * Example method
   */
  void process();
  int attachLights(int (*on)(int), void (*off)());
  int attachPump(int (*on)(int), void (*off)());

  // Sensors
  // Basic format:int attach<SENSORNAME>(int (*check)(), int low, int high)
  int attachBattery();
  int attachLiquidLevel(int (*check)(), int low, int high);

  void mycelHandler(const char *event, const char *data);

private:
  /**
   * Private global variables
   */
  String hydroponicMethod;
  String productName;

  int actuatorBitmask;
  int sensorBitmask;

  int (*turnOnLights)(int);
  void (*turnOffLights)();

  int (*turnOnPump)(int);
  void (*turnOffPump)();

  // struct Sensor liquidLevelSensor;

  int batteryPercentage;

  Sensor sensorArray[8];

  // System Health Check
  struct System health;
  Timer *healthTimer;
  void healthCheck();

  // Basic Light/Pump operations
  Timer *lightsTimer;
  Timer *pumpTimer;
  int setLights(String command);
  int setPump(String command);

  // Configures device core information such as RGB color and time zone
  int configure(String command);

  // Check Sensors
  int checkSensors();
  String activeIssue;

  // MESH REQUESTS
  struct ActionRequest blankRequest;
  struct ActionRequest activeRequest;
  Timer *actionTimeout;

  ActionRequest requestFormatter(const char *data);
  ActionRequest requestCreator(String action, int timeout);
  void resetAction();
};
