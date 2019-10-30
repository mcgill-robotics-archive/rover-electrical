#include "SerialInterface.h"

enum SerialReadStates
{
    SR_WAITING,          // Waiting to receive start byte
    SR_READING_HEADER,   // Reading n-bytes of header
    SR_READING_PAYLOAD   // Reading payload until stop byte 
};

enum SerialWriteStates
{
    SW_WAITING,
    SW_WRITING_HEADER,
    SW_WRITING_PAYLOAD
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
    // While the input buffer has at least 1 byte, read and process
    while (Serial.available() > 0)
    {
        process_incoming(Serial.read());
    }

    // While the output buffer has room for at least 1 byte, process and write
    while(Serial.availableForWrite() > 0 && (!out_messages.is_empty() || !priority_out_messages.is_empty()))
    {
        process_outgoing();
        Serial.flush();
    }
}

Message SerialInterface::get_next_message()
{  
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

    message_cache[next_message_id++] = message;

    next_message_id %= MAX_QUEUE_SIZE;
    
    out_messages.enqueue(message);
}

void SerialInterface::send_message(uint8_t frameType, String& payload)
{
    Message message = { 
        .systemID = ID, 
        .frameID = next_message_id, 
        .checksum = crc8ccitt(payload.c_str(), payload.length()),
        .frameType = frameType, 
        .data = payload }; 

    message_cache[next_message_id++] = message;

    next_message_id %= MAX_QUEUE_SIZE;
    
    out_messages.enqueue(message);
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
                // Pass the completed message forward for processing
                handle_received_message(message);

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

void SerialInterface::handle_received_message(Message& message)
{
    // First check that the message is valid
    if (!validate_message(message))
        return;

    // If the message is a "RQ" message
    if (message.frameType == 'R')
    {
        uint8_t requested_id = message.data.toInt();
        retransmit_message(requested_id);
        return; 
    }

    // If the message is a "ACK" message
    if (message.frameType == 'A')
    {
        uint8_t acked_id = message.data.toInt();
        free_message(acked_id);
        return;
    }

    // Otherwise the message is just generic
    // NOTE: this block must remain at the bottom of the function
    {   // Handle generic messages
        String ack_payload = String(message.frameID);
        send_priority_message('A', ack_payload);

        // Current message is not the expected next message so request everything in between
        if (last_successfull_transmitted_id >= 0 && message.frameID != last_successfull_transmitted_id + 1) 
        {
            for (uint8_t i = last_successfull_transmitted_id + 1; i < message.frameID; i++)
            {
                String request_payload = String(i);
                send_priority_message('R', request_payload);
            }
        }

        // TODO: This assumes that the requested messages will actually be
        //       correctly received the second time.
        //       If the data is corrupted again, the frames are lost.
        last_successfull_transmitted_id = message.frameID;
        in_messages.enqueue(message);
    }
}

void SerialInterface::process_outgoing()
{
    static Message message_out;
    static SerialWriteStates state_write = SW_WAITING;
    static uint16_t pos_out = 0;
    static bool is_priority = false;

    switch (state_write)
    {
        case SW_WAITING:
            if (!priority_out_messages.is_empty()) // if there is a priority message, do this first.
            {
                message_out = priority_out_messages.peek();
                state_write = SW_WRITING_HEADER;
                pos_out = 0;
                is_priority = true;
                Serial.write('~');
            }
            else if (!out_messages.is_empty()) // if there is a message to be sent
            {
                message_out = out_messages.peek();
                state_write = SW_WRITING_HEADER;
                pos_out = 0;
                is_priority = false;
                Serial.write('~'); // start writing data on next pass
            }
            break;
        case SW_WRITING_HEADER:
            switch (pos_out)
            {
                case 0: // SysID
                    pos_out++;
                    Serial.write(message_out.systemID);
                    break;
                case 1: // FrameID
                    pos_out++;
                    Serial.write(message_out.frameID);
                    break;
                case 2: // Checksum
                    pos_out++;
                    Serial.write(message_out.checksum);
                    break;
                case 3: // Frametype
                    pos_out = 0; // reset position for use in payload
                    state_write = SW_WRITING_PAYLOAD;
                    Serial.write(message_out.frameType);
                    break;
            }
            break;
        case SW_WRITING_PAYLOAD:
            if (pos_out < message_out.data.length())
            {
                Serial.write(message_out.data[pos_out++]);
            }
            else
            {
                state_write = SW_WAITING;
                pos_out = 0;
                Serial.write('#');
                if (is_priority)
                    priority_out_messages.dequeue();
                else
                    out_messages.dequeue();
            }
            break;
    }
}

bool SerialInterface::validate_message(Message& message)
{
    // An empty message is invalid
    if (message.data.equals(""))
        return false;

    // Check against the checksum
    String string_to_check = String(message.frameType) + message.data;
    if (message.checksum != crc8ccitt(string_to_check.c_str(), string_to_check.length()))
        return false;

    return true;
}


void SerialInterface::send_priority_message(uint8_t frameType, String& payload)
{
    send_priority_message(frameType, payload.c_str());
}

void SerialInterface::send_priority_message(uint8_t frameType, const char* payload)
{
    Message message = { 
        .systemID = ID, 
        .frameID = next_message_id, 
        .checksum = crc8ccitt(payload, strlen(payload)),
        .frameType = frameType, 
        .data = String(payload) }; 

    message_cache[next_message_id++] = message;

    next_message_id %= MAX_QUEUE_SIZE;
    
    priority_out_messages.enqueue(message);
}

void SerialInterface::retransmit_message(uint8_t id)
{
    /*
     * If a message is requested and the message prior (this.id = last.id + 1) was
     * successfully ackowledged we just retransmit the requested message. 
     * However, if there is a gap, retransmit all messages in between. 
     */

    // Take the % queue_size to account for the circular array here
    if (id == (last_successfull_transmitted_id + 1) % MAX_QUEUE_SIZE) 
    {
        // Get the id of the requested message from the data field
        // Queue up the message in the priority queue.
        priority_out_messages.enqueue(message_cache[id]);
    }
    else // Here we enter the case where there was a gap between the last ack'd message
    {
        // Request all missing messages and the requested message.
        for (uint8_t i = last_successfull_transmitted_id + 1; i <= id; i++)
        {
           priority_out_messages.enqueue(message_cache[i]); 
        }
    }
}

void SerialInterface::free_message(uint8_t id)
{
    // TODO: This
    //if (id != last_successfull_transmitted_id + 1)
    //{
    //    retransmit_message(id - 1);
    //}
    last_successfull_transmitted_id = id;
}