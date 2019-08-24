/* mycelium library by Dhvanil Shah
 */

#include "mycelium.h"

DHT dht(2, DHT22);

/**
 * Constructor.
 */
Mycelium::Mycelium(String type, String product)
{
    hydroponicMethod = type;
    productName = product;
    EEPROM.get(0, health);
    if (health.cycleDay <= 0)
    {
        health =
            {0,
             0,
             0,
             0,
             0,
             0};
    }
}

/**
 * Example method.
 */
void Mycelium::begin()
{
    // initialize hardware
    // dht.begin();
    pinMode(D7, OUTPUT);
    digitalWrite(D7, LOW);

    //Check actuator bitmask to set cloud functions
    if (actuatorBitmask & LIGHTS)
    {
        Particle.function("setLights", &Mycelium::setLights, this);
    }
    if (actuatorBitmask & PUMP)
    {
        Particle.function("setPump", &Mycelium::setPump, this);
    }

    // Check sesnor bitmask to set cloud variables
    // if (sensorBitmask & LIQUID_LEVEL)
    // {
    //     Particle.variable("liquidLevel", liquidLevelSensor.lastReading);
    // }
    if (sensorBitmask & BATT_OP)
    {
        Particle.variable("batteryLevel", batteryPercentage);
    }

    // TESTING
    Particle.variable("ActiveIssue", activeIssue);

    // Set the cloud function to modify system struct
    // IMPLEMENT

    RGB.control(true);
    RGB.brightness(255);
    RGB.color(0, 255, 0);

    // Start health check timer
    healthTimer = new Timer(10 * SEC_TO_MILLIS, &Mycelium::healthCheck, *this);
    healthTimer->start();

    actionTimeout = new Timer(1 * SEC_TO_MILLIS, &Mycelium::resetAction, *this);
}

/**
 * Example method.
 */
void Mycelium::process()
{
    // do something useful
    // readDHT();

    // resume normal operation
    // RGB.control(false);
    // mycelHandler("nothing", "WaterHigh/#000000/a58hs3/6000"); //Sample format of what an action request should look like
    // ActionRequest newRew = requestFormatter("WaterHigh/#000000/a58hs3/6000"); //Sample format of what an action request should look like
    // delay(1000);
}

// LIGHTS
int Mycelium::attachLights(int (*on)(int), void (*off)())
{
    turnOnLights = on;
    turnOffLights = off;
    actuatorBitmask = actuatorBitmask | LIGHTS;
    return 1;
}

int Mycelium::setLights(String command)
{
    int timerInt = command.toInt();
    turnOnLights(0);
    lightsTimer = new Timer(timerInt, turnOffLights, true);
    lightsTimer->start();
    return 1;
}

// PUMPS
int Mycelium::attachPump(int (*on)(int), void (*off)())
{
    turnOnPump = on;
    turnOffPump = off;
    actuatorBitmask = actuatorBitmask | PUMP;
    return 1;
}

int Mycelium::setPump(String command)
{
    int timerInt = command.toInt();
    turnOnPump(0);
    pumpTimer = new Timer(timerInt, turnOffPump, true);
    pumpTimer->start();
    return 1;
}

/**
* ALL SENSOR CODE START
*/
// Liquid Level Sensor
int Mycelium::attachLiquidLevel(int (*check)(), int low, int high)
{
    sensorBitmask = sensorBitmask | LIQUID_LEVEL;

    Sensor liquidLevelSensor;
    liquidLevelSensor.highThresh = high;
    liquidLevelSensor.lowThres = low;
    liquidLevelSensor.check = check;
    liquidLevelSensor.name = "LiquidLevel";
    sensorArray[1] = liquidLevelSensor;

    Particle.variable("liquidLevel", sensorArray[1].lastReading);
    return 1;
}

// BATTERY
int Mycelium::attachBattery()
{
    sensorBitmask = sensorBitmask | BATT_OP;
    return 1;
}
/**
* ALL SENSOR CODE FINISH
*/

/**
* CHECK SENSORS BEGIN
*/
int Mycelium::checkSensors()
{
    int holderMask = sensorBitmask;
    int bitNumber = 0;

    while (bitNumber < 8)
    {
        if (holderMask & 0x01)
        {
            sensorArray[bitNumber].lastReading = sensorArray[bitNumber].check();
            if (sensorArray[bitNumber].lastReading <= sensorArray[bitNumber].lowThres)
            {
                // SENSOR READING LOW
                String issue = String(sensorArray[bitNumber].name + "LOW");
                // SET AN ACTION REQUEST FOR THE NETWORK
                if (!activeRequest.active)
                {
                    requestCreator(issue, SEC_ONE_DAY);
                    sensorArray[bitNumber].activeIssue = true;
                }
            }
            else if (sensorArray[bitNumber].lastReading >= sensorArray[bitNumber].highThresh)
            {
                // SENSOR READING HIGH
                String issue = String(sensorArray[bitNumber].name + "HIGH");
                // SET AN ACTION REQUEST FOR THE NETWORK
                if (!activeRequest.active)
                {
                    requestCreator(issue, SEC_ONE_DAY);
                    sensorArray[bitNumber].activeIssue = true;
                }
            }
            else
            {
                if (sensorArray[bitNumber].activeIssue == true)
                {
                    // RESOLVE ACTIVE ACTION REQUEST
                    // implement
                    sensorArray[bitNumber].activeIssue = false;
                    resetAction();
                }
            }
        }
        bitNumber++;
        holderMask = holderMask >> 1;
    }
    return 1;
}

/**
* HEALTH CHECK BEGIN
*/
void Mycelium::healthCheck()
{
    digitalWrite(D7, HIGH);
    delay(1000);
    digitalWrite(D7, LOW);
    if (health.cycleDay != 0)
    {
        if (Time.day() != Time.day(health.timeUpdated))
        {
            health.cycleDay = health.cycleDay + 1;
        }
    }
    // Serial.println(health.cycleDay);
    // Serial.println(Time.format(health.timeUpdated));
    health.timeUpdated = Time.now();
    EEPROM.put(0, health);

    checkSensors();
    // liquidLevelSensor.lastReading = liquidLevelSensor.check();
    // if ((liquidLevelSensor.lastReading <= liquidLevelSensor.lowThres) || liquidLevelSensor.lastReading >= liquidLevelSensor.highThresh)
    // {
    //     // we have a problem
    //     if (activeRequest.active)
    //     {
    //         // do nothing
    //     }
    //     else
    //     {
    //         requestCreator("WaterHigh", 5000);
    //         // make new request
    //     }
    // }
    return;
}
/**
* HEALTH CHECK FINISH
*/

/**
* MESH NETWORK COMMUNICATIONS
*/
void Mycelium::mycelHandler(const char *event, const char *data)
{
    ActionRequest newRequest = requestFormatter(data);

    if ((newRequest.action).equals("resolve") && (newRequest.id).equals(activeRequest.id))
    {
        resetAction();
        return;
    }

    if (activeRequest.priority < newRequest.priority)
    {
        Serial.println("We have a new king");
        Serial.print(activeRequest.priority);
        Serial.print(" < ");
        Serial.println(newRequest.priority);
        if (actionTimeout->isActive())
        {
            // Serial.println("Last timer stopped");
            actionTimeout->stop();
        }
        actionTimeout->changePeriod(newRequest.timeout);
        actionTimeout->start();
        Serial.print("Request Timer Started with period: ");
        Serial.println(newRequest.timeout);
        activeRequest = newRequest;
    }
    return;
    //ADD CODE HERE TO MAKE THE SYSTEM PERFORM SOME ACTIONS
}

void Mycelium::resetAction()
{
    if (actionTimeout->isActive())
    {
        actionTimeout->stop();
    }
    // Send out a resolve if youre resolving your own request
    if ((activeRequest.id).equals(System.deviceID()))
    {
        requestCreator("resolve", 10);
    }
    activeRequest = blankRequest;
    Serial.println("Request cleared");
}

ActionRequest Mycelium::requestFormatter(const char *data)
{
    char command[256];
    char *request[4];
    int i = 0;

    strcpy(command, data);
    char *token = strtok(command, "/");
    while (token)
    {
        request[i] = token;
        token = strtok(NULL, "/");
        i++;
    }
    // ActionRequest newRequest = {request[0], request[1], request[2], atol(request[3]), random(10)};
    ActionRequest newRequest = {String(request[0]), String(request[1]), String(request[2]), atol(request[3]), random(1, 10), true};
    return newRequest;
}

ActionRequest Mycelium::requestCreator(String action, int timeout)
{
    int timeoutMS = timeout * SEC_TO_MILLIS;
    String color = String(String(health.r) + "," + String(health.g) + "," + String(health.b));
    String reqBody = String(action + "/" + color + "/" + System.deviceID() + "/" + String(timeoutMS));

    if (!action.equals("resolve"))
    {
        ActionRequest newRequest = {action, color, System.deviceID(), 0, 11, true};
        activeRequest = newRequest;
    }
    Serial.println(reqBody);
    Mesh.publish("mycelium", reqBody);
    return blankRequest;
}
