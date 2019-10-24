#include "SerialInterface.h"

enum SerialReadStates
{
    SR_WAITING,          // Waiting to receive start byte
    SR_READING_HEADER,   // Reading n-bytes of header
    SR_READING_PAYLOAD   // Reading payload until stop byte 
};

SerialInterface::SerialInterface()
{
    // initialize the array to false
    for (uint8_t i = 0; i < MAX_QUEUE_SIZE; i++)
    {
        priority_ids[i] = false;
    }
}

void SerialInterface::begin(int _baudrate)
{
    Serial.begin(_baudrate);
}

void SerialInterface::update()
{
    while (Serial.available() > 0)
    {
        process_incoming(Serial.read());
    }
}

Message SerialInterface::get_next_message()
{
    // Get the priority messages before anything else
    if (!priority_in_messages.is_empty())
    {
        Message result = priority_in_messages.dequeue();
    }
    
    return in_messages.dequeue();
}

void SerialInterface::send_message(uint8_t frameType, const char* payload)
{
    Message message = { 
        .systemID = ID, 
        .frameID = next_message_id, 
        .checksum = crc8ccitt(payload, strlen(payload)),
        .frameType = frameType, 
        .data = String(payload) }; 

    out_messages[next_message_id++] = message;

    next_message_id %= MAX_QUEUE_SIZE;
    Serial.flush(); //TODO: non-blocking
    Serial.println(package_frame(message));
}

String SerialInterface::package_frame(const Message& message)
{
    String result;
    result.reserve(sizeof(char) * (6 + message.data.length()));

    result += '~';
    result += (char)message.systemID;
    result += (char)message.frameID;
    result += (char)message.checksum;
    result += (char)message.frameType;
    result += message.data;
    result += "#";

    return result;
}

void SerialInterface::process_incoming(const byte in_byte)
{
    static Message message;
    static SerialReadStates state = SR_WAITING;
    static unsigned int header_pos = 0;

    switch (state)
    {
        case SR_WAITING: // Waiting for start of a new packet
            // if expecting a packet and byte is not start byte, break.
            if (in_byte != '~') break;
            state = SR_READING_HEADER;
            header_pos = 0;
            break;
        case SR_READING_HEADER: // Reading the header information for a packet.
            if (in_byte == '#') // If the end byte is read, break.
            {
                message = Message();
                state = SR_WAITING;
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
                    state = SR_READING_PAYLOAD;
                    break;
                default:
                    break;
            }
            break;
        case SR_READING_PAYLOAD:
            if (in_byte == '#') // If the end byte is read, validate the message, reset, and break.
            {
                if (validate_message(message))
                {
                    // If the message is valid, place it in the proper queue
                    if (priority_ids[message.frameID])
                        priority_in_messages.enqueue(message);
                    else
                        in_messages.enqueue(message);
                }
                message = Message();
                state = SR_WAITING;
                break;
            }
            else
            {
                message.data = message.data + (char)in_byte;
                break;
            }
    }
}

bool SerialInterface::validate_message(Message& message)
{
    // An empty message is invalid
    if (message.data.equals(""))
        return false;

    // Check against the checksum
    if (message.checksum != crc8ccitt(message.data.c_str(), message.data.length()))
        return false;

    return true;
}