#pragma once
#include <Arduino.h>
#include "utils/Queue.h"
#include "utils/crc.h"

#define MAX_QUEUE_SIZE 32
#define ID '0'

// Store message information for processing
struct Message
{
    byte systemID;
    byte frameID;
    byte checksum;
    byte frameType;
    String data;
};

/**
 * Serial Interface to establish connection and communicate with
 * devices using the McGill Robotics Rover Serial Frame standard
 */
class SerialInterface
{
public:
    SerialInterface();

    /**
     * Initialize the serial connection.
     * @param _baudrate the baudrate to use
     */
    void begin(int _baudrate);

    /**
     * Process incoming and outgoing data each loop iteration.
     */
    void update();

    /**
     * Get the next message in the queue. If there are any
     * priority messages, pop from that queue before regular messages.
     * @return the next message in the queue
     */
    Message get_next_message();

    /**
     * Send a message over the serial connection.
     * @param frameType Indicates which type of frame this message is.
     * @param payload Message payload
     */
    void send_message(uint8_t frameType, const char* payload);
    /**
     * Send a message over the serial connection.
     * @param frameType Indicates which type of frame this message is.
     * @param payload Message payload
     */
    void send_message(uint8_t frameType, String& payload);

    /**
     * Sends a message over the serial connection _immediately_.
     * @see send_message
     */
    void send_priority_message(uint8_t frameType, String& payload);

    /**
     * Sends a message over the serial connection _immediately_.
     * @see send_message
     */
    void send_priority_message(uint8_t frameType, const char* payload);

private:
    /**
     * FSM which processes incoming binary data and packages it into Messages.
     */
    void process_incoming(const byte in_byte);

    /**
     * Intermediate function which passes a completed message to the correct place.
     */
    void handle_received_message(Message& message);

    /**
     * FSM which writes the next byte.
     */
    void process_outgoing();

    /**
     * Validates an incoming message by comparing with validity criteria:
     * 1. Checksum
     * 2. non-empty
     * @return returns true if a message is valid.
     */
    bool validate_message(Message& message);

    /**
     * Resets the sliding window from a given id and pushes all messages out.
     * @param id the id from which to begin retransmission.
     */
    void reset_window(uint8_t id);

    /**
     * Request that the window be reset at given frame id.
     * @param id the id from which the other should begin retransmissions.
     */
    void request_retransmission(uint8_t id);

    /**
     * Send out an ack frame for a given frame id.
     * @param id id of the acknowledged message.
     */
    void ack_message(uint8_t id);

    /** This queue stores all the incoming messages which have not yet been processed. **/
    Queue<Message> in_messages;
    /** This queue stores all the incoming _priority_ messages that have not yet been processed. **/
    Queue<Message> priority_in_messages;
    /** This queue stores all outgoing messages which have not yet been processed. **/
    Queue<Message> out_messages;
    /** This queue stores all priority outgoing messages which have not yet been processed. **/
    Queue<Message> priority_out_messages;
    /** For each frame id, the value of this array is set to true if the corresponding message is a priority message.**/
    bool priority_ids[MAX_QUEUE_SIZE];
    /** A cache of the last (MAX_QUEUE_SIZE) messages. **/
    Message message_cache[MAX_QUEUE_SIZE];

    /** The frame id for the next message that will be sent. **/
    uint8_t next_outgoing_frame_id = 0;

    /** The expected frame id for the next incoming message. **/
    uint8_t expected_frame_id = 0;

    /** The id of the last frame we received an acknowledge for. **/
    uint8_t last_acked_frame = 0;
};