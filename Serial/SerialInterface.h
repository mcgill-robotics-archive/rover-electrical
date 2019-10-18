#pragma once
#include <Arduino.h>
#include "utils/Queue.h"

enum SerialStates
{
    S_WAITING,          // Waiting to receive start byte
    S_READING_HEADER,   // Reading n-bytes of header
    S_READING_PAYLOAD   // Reading payload until stop byte 
};

// Store message information for processing
struct Message
{
    byte systemID;
    byte frameID;
    byte checksum;
    byte frameType;
    String data;
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

    Message next_message()
    {
        return messages.dequeue();
    }

private:
    void process_incoming(const byte in_byte)
    {
        static Message message;
        static SerialStates state = S_WAITING;
        static unsigned int header_pos = 0;

        switch (state)
        {
            case S_WAITING: // Waiting for start of a new packet
                // if expecting a packet and byte is not start byte, break.
                if (in_byte != '~') break;
                state = S_READING_HEADER;
                header_pos = 0;
                break;
            case S_READING_HEADER: // Reading the header information for a packet
                switch (header_pos)
                {
                    case 0:
                        message.systemID = in_byte;
                        header_pos++;
                        break;
                    case 1:
                        message.frameID = in_byte;
                        header_pos++;
                        break;
                    case 2:
                        message.checksum = in_byte;
                        header_pos++;
                        break;
                    case 3:
                        message.frameType = in_byte;
                        state = S_READING_PAYLOAD;
                        break;
                    default:
                        break;
                }
                break;
            case S_READING_PAYLOAD:
                if (in_byte == '~')
                {
                    validate_message(message);
                    message = Message();
                    state = S_WAITING;
                    break;
                }
                else
                {
                    message.data = message.data + (char)in_byte;
                    break;
                }
        }
    }

    void validate_message(Message& message)
    {
        if (!message.data.equals(""))
            messages.enqueue(message);
    }

    Queue<Message> messages;
};