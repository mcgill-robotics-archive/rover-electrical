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
    return in_messages.dequeue();
}

void SerialInterface::send_message(uint8_t frameType, const char* payload)
{
    Message message = { 
        .systemID = ID, 
        .frameID = next_outgoing_frame_id, 
        .checksum = crc8ccitt(payload, strlen(payload)),
        .frameType = frameType, 
        .data = String(payload) }; 

    if (frameType != 'R' && frameType != 'A')
    {
        message_cache[next_outgoing_frame_id++] = message;

        next_outgoing_frame_id %= MAX_QUEUE_SIZE;
    }
    
    out_messages.enqueue(message);
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
        // TODO: this quantity should be stored under message.frameID
        uint8_t requested_id = message.data.toInt();
        reset_window(requested_id);
    } 
    else if (message.frameType == 'A')  // If the message is a "ACK" message
    {
        // TODO: this quantity should be stored under message.frameID.
        uint8_t acked_id = message.data.toInt();
        /*
         * Register the id as the last successfully received frame so we know
         * where to reset the sliding window to in the event of a failure.
         */
        last_acked_frame = acked_id;
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
        // send an ack and add it to the queue
        ack_message(message.frameID);
        in_messages.enqueue(message);
        // Then set the expected frame id. This quantity will wrap around the size of a queue
        expected_frame_id = (expected_frame_id + 1); //% MAX_QUEUE_SIZE;
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
    if (message.checksum != crc8ccitt(message.data.c_str(), message.data.length()))
        return false;

    return true;
}

void SerialInterface::send_priority_message(uint8_t frameType, const char* payload)
{
    Message message = { 
        .systemID = ID, 
        .frameID = next_outgoing_frame_id, 
        .checksum = crc8ccitt(payload, strlen(payload)),
        .frameType = frameType, 
        .data = String(payload) }; 

    if (frameType != 'A' && frameType != 'R')
    {
        message_cache[next_outgoing_frame_id++] = message;

        next_outgoing_frame_id %= MAX_QUEUE_SIZE;
    }
    
    priority_out_messages.enqueue(message);
}

void SerialInterface::reset_window(uint8_t id)
{
    // Send out all missed messages again
    // TODO: implement sending _all_ missed messages and not just the start of missed block
    priority_out_messages.enqueue(message_cache[id]);

}

void SerialInterface::request_retransmission(uint8_t id)
{
    String rq_payload = String(id);
    send_message('R', rq_payload.c_str());
}

void SerialInterface::ack_message(uint8_t id)
{
    String ack_payload = String(id);
    send_message('A', ack_payload.c_str());
}