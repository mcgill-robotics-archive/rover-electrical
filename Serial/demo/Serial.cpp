#include <Arduino.h>
#include "SerialInterface.h"
#include "utils/crc.h"

SerialInterface serial = SerialInterface(9600, 'D');

void setup() 
{
    serial.begin();
    pinMode(2, INPUT);
}

void loop() 
{
    serial.update();

    Message message = serial.get_next_message();

    if (!message.data.equals(""))
    {
        serial.send_message('0', "0");
    }
}