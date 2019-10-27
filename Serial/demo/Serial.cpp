#include <Arduino.h>
#include "SerialInterface.h"
#include "utils/crc.h"

SerialInterface serial;

void setup() 
{
    serial.begin(9600);
}

void loop() 
{
    serial.update();

    Message message = serial.get_next_message();

    if (!message.data.equals(""))
    {
        Serial.print("SID: ");
        Serial.write(message.systemID);
        Serial.print(" FID: ");
        Serial.write(message.frameID);
        Serial.print(" CHK: ");
        Serial.write(message.checksum);
        Serial.print(" FT: ");
        Serial.write(message.frameType);
        Serial.println(" DATA: " + message.data);
    }

    if (!message.data.equals(""))
    {
        serial.send_message('1', "hello");
    }
}