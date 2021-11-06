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

SerialInterface::SerialInterface(int _baudrate, uint8_t sys_id, uint64_t timeout) :
    baudrate(_baudrate),
    system_id(sys_id),
    timeout(timeout)
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
    // last time update was called. Used to compute delta_times
    static unsigned long last_time = 0;

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

    if (state != WAITING)
    {
        ack_timer += millis() - last_time;
        last_time = millis();

        // If the last acked message was more than _timeout_ seconds ago, drop connection
        if (ack_timer > timeout)
        {
            reset_state();
        }
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
    // Prevent sending messages while not in the established state
    if (state != ESTABLISHED) return;

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

bool SerialInterface::is_priority(const Message& message)
{
    // If the message is a control type, it is priority 
    if (is_control(message))
        return true;

    // If we are receiving the message, check it against the list of priority ids
    if (message.systemID != system_id)
    {
        // This will return true if the message is marked as priority
        return priority_ids[message.frameID];
    }

    return false;
}

bool SerialInterface::is_control(const Message& message)
{
    // control messages are defined only by their frametypes.
    switch (message.frameType)
    {
        case 'A': return true;  // ack
        case 'R': return true;  // rq
        case 'S': return true;  // syn
        case 'Y': return true;  // syn/ack
        case 'F': return true;  // fin
        case 'Q': return true;  // fin/ack
        default : return false;
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
    {
        // If the message is not valid, 
        // request a retransmission of the expected frame
        
        // Don't attempt to recover control messages
        if (!is_control(message) && state == ESTABLISHED)
            request_retransmission(expected_frame_id);
        return;
    }

    // If a synchronize attempt is made, acknowledge it
    if (state == WAITING && message.frameType == 'S') 
    {
        syn_ack(message.frameID, next_outgoing_frame_id);

        expected_frame_id = message.frameID; 
        state = SYN_RECEIVED;
        ack_timer = 0;
        return;
    }

    if (state == SYN_RECEIVED && message.frameType == 'A')
    {
        if (message.frameID == next_outgoing_frame_id)
        {
            // Connection is now established
            state = ESTABLISHED;
            ack_timer = 0;
        }
        
        return;
    }

    if (message.frameType == 'F' && state == ESTABLISHED)
    {
        uint8_t frameid = (message.frameID + 1) % MAX_QUEUE_SIZE;
        fin_ack(frameid);
        state = FIN_RECEIVED;
        return;
    }

    if (state == FIN_RECEIVED && message.frameType == 'A')
    {
        // Connection was terminated gracefully,
        // reset everything and wait for a new synchronization request
        reset_state();
        state = WAITING;
    }

    // Before getting to processing regular messages, 
    // ensure we are in the established state
    if (state != ESTABLISHED)
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
        ack_timer = 0;
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
        
        // If we are waiting for a requested message and it comes in
        if (waiting_request && message.frameID == requested_frame_id)
            waiting_request = false;

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

bool SerialInterface::validate_message(const Message& message)
{
    // control messages are checked first
    if (message.frameType == 'A' && message.checksum == 0xc0)
        return true;
    if (message.frameType == 'R' && message.checksum == 0xb9)
        return true;
    if (message.frameType == 'S' && message.checksum == 0xbe)
        return true;
    if (message.frameType == 'F' && message.checksum == 0xd5)
        return true;
    if (message.frameType == 'Q' && message.checksum == 0xb0)
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

    // Don't send another request if this frame was already requested recently
    if (waiting_request)
        return;

    requested_frame_id = id;
    waiting_request = true;

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

void SerialInterface::syn_ack(uint8_t id, uint8_t next_id)
{
    String to_check = String('Y') + String(next_id);
    Message syn_ack = {
        .systemID = system_id,
        .frameID = id,
        .checksum = crc8ccitt(to_check.c_str(), to_check.length()),
        .frameType = 'Y',
        .data = String(next_id) };

    enqueue_message(syn_ack);
}

void SerialInterface::fin_ack(uint8_t id)
{
    Message fin_ack = {
        .systemID = system_id,
        .frameID = id,
        .checksum = 0xb0, // precomputed checksum for 'Q'
        .frameType = 'Q',
        .data = String("") };

    enqueue_message(fin_ack);
}

void SerialInterface::reset_state()
{
    ack_timer = 0;
    waiting_request = 0;

    // The connection was dropped so we should clear up some unnecessary memory.
    out_messages.clear();
    priority_out_messages.clear();

    state = WAITING;

    for (uint8_t i = 0; i < MAX_QUEUE_SIZE; ++i)
    {
        priority_ids[i] = false;
    }
}