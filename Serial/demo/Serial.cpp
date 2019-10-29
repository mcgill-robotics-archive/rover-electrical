#include <Arduino.h>
#include "SerialInterface.h"
#include "utils/crc.h"

SerialInterface serial;
int i = 0;

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
        serial.send_message('1', message.data);
    }
}