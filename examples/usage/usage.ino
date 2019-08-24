// Example usage for mycelium library by Dhvanil Shah.

#include "mycelium.h"

// Initialize objects from the lib
Mycelium mycelium("wicking", "starfish");

void setup()
{
    // Call functions on initialized library objects that require hardware
    delay(2000);
    Serial.println("delay ended");
    pinMode(D7, OUTPUT);
    digitalWrite(D7, LOW);

    mycelium.attachLights(turnOnlight, turnOfflight);
    mycelium.attachPump(turnOnlight, turnOfflight);
    // mycelium.attachBattery();
    mycelium.attachLiquidLevel(checkLiquidLevel, 0, 2);

    Mesh.subscribe("mycelium", myceliumSubscribe);

    mycelium.begin();
}

void loop()
{
    // Use the library's initialized objects and functions
    mycelium.process();
    // Serial.println("runnung");
    // delay(1000);
}

int turnOnlight(int value)
{
    digitalWrite(D7, HIGH);
    return 0;
}

void turnOfflight()
{
    digitalWrite(D7, LOW);
}

int checkLiquidLevel()
{
    return random(0, 3);
}

void myceliumSubscribe(const char *event, const char *data)
{
    mycelium.mycelHandler(event, data);
    return;
}