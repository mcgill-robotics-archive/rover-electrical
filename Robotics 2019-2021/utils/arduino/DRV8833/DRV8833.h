#pragma once

/**
 * A class to manage motor connections.
 */
class DRV8833
{
public:
    DRV8833();

    /**
     * Setup the respective pins and register the pin modes with the arduino.
     * @param _pin1 XIN1 on controller board.
     * @param _pin2 XIN2 on controller board.
     */
    void Initialize(int _pin1, int _pin2);

    /**
     * Set the power and direction of the motor.
     * If the power is negative, the motor will run in reverse.
     * @param power integer between -255 and 255.
     */
    void move(int power);

    /**
     * Immediately stops the motor
     */
    void stop();
private:
    int pin1;
    int pin2;
};
