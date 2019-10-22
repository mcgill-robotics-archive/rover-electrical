#pragma once
#include <Arduino.h>
#include "utils/Queue.h"
#include "utils/crc.h"

#define MAX_QUEUE_SIZE 32
#define ID '0'

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
    SerialInterface()
    {
        // initialize the array to false
        for (uint8_t i = 0; i < MAX_QUEUE_SIZE; i++)
        {
            priority_ids[i] = false;
        }
    }

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

    Message get_next_message()
    {
        // Get the priority messages before anything else
        if (!priority_in_messages.is_empty())
        {
            Message result = priority_in_messages.dequeue();
        }
        
        return in_messages.dequeue();
    }

    void send_message(uint8_t frameType, const char* payload)
    {
        Message message = { 
            .systemID = ID, 
            .frameID = next_message_id, 
            .checksum = '3', // TODO: implement checksum
            .frameType = frameType, 
            .data = String(payload) }; 

        out_messages[next_message_id++] = message;

        next_message_id %= MAX_QUEUE_SIZE;
        Serial.flush(); //TODO: non-blocking
        Serial.println(encode_message(message));
    }

    String encode_message(const Message& message)
    {
        String result;
        result.reserve(sizeof(char) * (6 + message.data.length()));

        result += '~';
        result += message.systemID;
        result += message.frameID;
        result += message.checksum;
        result += message.frameType;
        result += message.data;
        result += "#";

        return result;
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
            case S_READING_HEADER: // Reading the header information for a packet.
                if (in_byte == '#') // If the end byte is read, break.
                {
                    message = Message();
                    state = S_WAITING;
                    break;
                }
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
                if (in_byte == '#') // If the end byte is read, process the message and break.
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
        if (!message.data.equals("")) // currently this is the only qualification for a message being valid
        {
            // Check if the frameID is listed in the priority messages
            if (priority_ids[message.frameID])
                priority_in_messages.enqueue(message);
            else
                in_messages.enqueue(message);
        }
    }

    Queue<Message> in_messages;
    Queue<Message> priority_in_messages;
    bool priority_ids[MAX_QUEUE_SIZE];  // Each index into this array represents an id, if the value is true then it is a priority message
    Message out_messages[MAX_QUEUE_SIZE];

    uint8_t next_message_id = 0;
};