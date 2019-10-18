#pragma once
#include <Arduino.h>
#include "utils/Queue.h"

enum SerialStates
{
    S_WAITING,          // Waiting to receive start byte
    S_READING_HEADER,   // Reading n-bytes of header
    S_READING_PAYLOAD   // Reading payload until stop byte 
};

class SerialInterface
{
public:

    void begin(int _baudrate)
    {
        Serial.begin(_baudrate);
    }

    void update()
    {
        while (Serial.available() > 0)
        {
            process_incoming(Serial.read());
        }
    }

    String next_message()
    {
        return messages.dequeue();
    }

private:
    void process_incoming(const byte in_byte)
    {
        static String message;
        static SerialStates state = S_WAITING;

        switch (in_byte)
        {
            case '~': // Start of message
                if (state == S_WAITING)
                {
                    message = "";
                    state = S_READING_PAYLOAD;
                    break;
                }
                else
                {
                    validate_message(message);
                    message = "";
                    state = S_WAITING;
                    break;
                }              
            default: // start pushing data into the string
                if (state == S_WAITING) break;
                message = message + (char)in_byte;
                break;
        }   
    }

    void validate_message(String& message)
    {
        if (!message.equals(""))
            messages.enqueue(message);
    }

    Queue<String> messages;
};