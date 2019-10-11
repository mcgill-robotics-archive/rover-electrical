#include "SerialInterface.h"

void SerialInterface::initialize(int _baudRate)
{
    baudRate = _baudRate;
    // TODO: allow for a wider range of baudrates, for now only accepts 9600
    if (baudRate != 9600) return;
    Serial.setTimeout(1000);
    Serial.begin(baudRate);
}

String SerialInterface::read()
{
    String result;
    result = Serial.readStringUntil('\n');
    int start;
    int end;

    start = result.indexOf('[');
    if (start == -1)
        return "";

    // Find any other start of messages
    while (result.indexOf('[', start + 1) != -1)
        start = result.indexOf('[', start + 1);
    end = result.indexOf(']');

    start++;
    
    if (start < end && start != -1 && end != -1)
        return result.substring(start, end);

    return "";
}

String SerialInterface::receive()
{
    String result = read();
    // Dont print ack if received ACK or nothing
    if (!result.equals("") && !result.equals("ACK"))
        write("ACK");
    return result;
}

void SerialInterface::send(const String message)
{
    write(message);
}

void SerialInterface::write(const String message)
{
    // Find escape flags in message data
    // TODO
    
    // Find start-end flags in message data
    // TODO

    // Encapsulate with flags
    String result = String('[');
    result.concat(message);
    result.concat(']');

    // Wait till the buffer is empty before pushing a new message
    Serial.flush();
    Serial.write(result.c_str());
    Serial.println();
}