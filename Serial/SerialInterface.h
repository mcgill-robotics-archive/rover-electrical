#pragma once
/**
 * Basic specifications:
 * 1. The flag "[" (0x5B) signifies the start of a message and "]" (0x5D), the end
 * 2. The flag 0x7E is an escape byte
 * 3. Each sent message expects an ACK
 * @author Roey Borsteinas
 * @date October 7, 2019
 */

#include <Arduino.h>

class SerialInterface
{
public:
    SerialInterface() {}

    void initialize(int _baudRate);

    void send(const String message);
    String receive();

private:
    // These methods are low level and deal directly with binary data sent over serial
    String read();
    void write(const String message);

    int baudRate;
};