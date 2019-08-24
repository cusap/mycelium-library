// Example usage for mycelium library by Dhvanil Shah.

#include "mycelium.h"

// Initialize objects from the lib
Mycelium mycelium("wicking", "starfish");

void setup()
{

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

void myceliumSubscribe(const char *event, const char *data)
{
    mycelium.mycelHandler(event, data);
    return;
}