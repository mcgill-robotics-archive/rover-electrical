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

SerialInterface::SerialInterface(int _baudrate, uint8_t sys_id) :
    baudrate(_baudrate),
    system_id(sys_id)
{
    // initialize the array to false
    for (uint8_t i = 0; i < MAX_QUEUE_SIZE; i++)
    {
        priority_ids[i] = false;
    }
}

void SerialInterface::begin()
{
    Serial.begin(baudrate);
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
    if (!priority_in_messages.is_empty())
        return priority_in_messages.dequeue();
    return in_messages.dequeue();
}

void SerialInterface::send_message(uint8_t frameType, const char* payload)
{
    String check = (char)frameType + String(payload);
    Message message = { 
        .systemID = system_id, 
        .frameID = next_outgoing_frame_id, 
        .checksum = crc8ccitt(check.c_str(), check.length()),
        .frameType = frameType, 
        .data = String(payload) }; 

    enqueue_message(message);
}

void SerialInterface::enqueue_message(Message& message)
{
    if (is_priority(message))
    {
        priority_out_messages.enqueue(message);
        return;
    }

    // Otherwise the message is just a generic
    message_cache[next_outgoing_frame_id++] = message;
    next_outgoing_frame_id %= MAX_QUEUE_SIZE;

    out_messages.enqueue(message);
}
bool SerialInterface::is_priority(Message& message)
{
    // If the message is a special priority type 
    if (message.frameType == 'A' || message.frameType == 'R')
        return true;

    // If we are receiving the message, check it against the list of priority ids
    if (message.systemID != system_id)
    {
        // This will return true if the message is marked as priority
        return priority_ids[message.frameID];
    }
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

void SerialInterface::handle_received_message(Message& message)
{
    // First check that the message is valid
    if (!validate_message(message))
        return;

    // If the message is a "RQ" message
    if (message.frameType == 'R')
    {
        reset_window(message.frameID);
    } 
    else if (message.frameType == 'A')  // If the message is a "ACK" message
    {
        /*
         * Register the id as the last successfully received frame so we know
         * where to reset the sliding window to in the event of a failure.
         */
        last_acked_frame = message.frameID;
    }
    else // Otherwise the message is just generic
    {
        // Before anything happens, ensure that the received message
        // has the expected id. If not, request that the window be reset
        // at the expected id and do not continue
        if (message.frameID != expected_frame_id)
        {
            request_retransmission(expected_frame_id);
            return; // do not continue
        }

        // The message is received in the correct order,
        // send an ack
        ack_message(message.frameID);

        // Check if the message is marked as priority
        if (is_priority(message))
        {
            // queue it as priority
            priority_in_messages.enqueue(message);
            // mark the id as not priority anymore
            priority_ids[message.frameID] = false;
        }
        else
        {
            // otherwise it's just a regular message
            in_messages.enqueue(message);
        }
        // Then set the expected frame id. This quantity will wrap around the size of a queue
        expected_frame_id = (expected_frame_id + 1) % MAX_QUEUE_SIZE; 
    }
}

bool SerialInterface::validate_message(Message& message)
{
    // Special messages are checked first
    if (message.frameType == 'A' && message.checksum == 0xc0)
        return true;
    if (message.frameType == 'R' && message.checksum == 0xb9)
        return true;


    // An empty message is invalid
    if (message.data.equals(""))
        return false;

    // Check against the checksum
    String check = (char)message.frameType + message.data;
    if (message.checksum != crc8ccitt(check.c_str(), check.length()))
        return false;

    return true;
}

void SerialInterface::reset_window(uint8_t requested_id)
{
    // Retransmit all messages since the requested id
    while (requested_id != next_outgoing_frame_id)
    {
        priority_out_messages.enqueue(message_cache[requested_id]);
        requested_id = (requested_id + 1) % MAX_QUEUE_SIZE;
    }
}

void SerialInterface::request_retransmission(uint8_t id)
{
    // Construct the message here to skip some extra insructions
    // WARNING: the checksum is hardcoded here so if we ever change
    // the scheme, fix this first.

    Message rq {
        .systemID = system_id,
        .frameID = id,
        .checksum = 0xb9, // precomputed checksum for 'R'
        .frameType = 'R',
        .data = String("") };
    
    priority_ids[id] = true;
    enqueue_message(rq);
}

void SerialInterface::ack_message(uint8_t id)
{
    // Construct the message here to skip some extra insructions
    // WARNING: the checksum is hardcoded here so if we ever change
    // the scheme, fix this first.
    Message ack = {
        .systemID = system_id,
        .frameID = id,
        .checksum = 0xc0, // precomputed checksum for 'A'
        .frameType = 'A',
        .data = String("") };

    enqueue_message(ack);
}