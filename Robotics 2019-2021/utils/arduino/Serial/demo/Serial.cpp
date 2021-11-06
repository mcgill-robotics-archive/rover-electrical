#include <Arduino.h>
#include "SerialInterface.h"
#include "SerialParser.hpp"

// Set up serial interface with:
//  baudrate 9600, 
//  system id of '0', 
//  and timeout of 1 minute
SerialInterface serial = SerialInterface(9600, '0', 60000);

void setup() 
{
    // Initialize serial
    serial.begin();
}

void loop() 
{
    // Tell serial to do a bit of work
    serial.update();

    // Grab the next message from the interface if there is one
    Message message = serial.get_next_message();

    // Parse the message for the relevant data field
    Variant speed = parse_message(message, 'S');

    // If there actually is an integer, do something with it
    if (speed.type == Variant::TYPE_INTEGER)
    {
        // Do stuff with speed
    }
}