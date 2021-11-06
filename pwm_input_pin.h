/*
 * pwm_input_pin.h
 *
 *  Created on: 1 Haz 2021
 *      Author: Eren
 */

#ifndef PWM_INPUT_PIN_H_
#define PWM_INPUT_PIN_H_

struct pwm_in
{
    // The pins are assigned to ADC0 by default, there should
    // be code to change the pin assignment to ADC1 if ADC0 is
    // occupied.

    int ADC_PERIPHERAL;
    int GPIO_PORT_BASE;
    int INTERNAL_PIN_NUM;
    int ADC_BASE;

    // For now the ADC's always using SS3 because it's
    // easier, although it could be changed with a
    // find-replace of these values;
    int SEQUENCER_NUMBER;
    int FIFO_BUFFER_DEPTH;
    int SEQUENCER_STEPS;

    // Represents the ADC the pin is currently assigned to by the main program.
    // Defaults to 0 for ADC0
    int CURRENT_ASSIGNED_ADC;
};

typedef struct pwm_in pwm_input_pin;

pwm_input_pin dummy_pin_in = {

           .ADC_PERIPHERAL = -1,
           .GPIO_PORT_BASE = -1,
           .INTERNAL_PIN_NUM = -1,
           .ADC_BASE = -1,

           .SEQUENCER_NUMBER = -1,
           .FIFO_BUFFER_DEPTH = -1,
           .SEQUENCER_STEPS = -1,
           .CURRENT_ASSIGNED_ADC = -1

};

pwm_input_pin PE3_in = {

             .ADC_PERIPHERAL = SYSCTL_PERIPH_ADC0,
             .GPIO_PORT_BASE = GPIO_PORTE_BASE,
             .INTERNAL_PIN_NUM = GPIO_PIN_3,
             .ADC_BASE = ADC0_BASE,

             .SEQUENCER_NUMBER = 3,
             .FIFO_BUFFER_DEPTH = 1,
             .SEQUENCER_STEPS = 1,
             .CURRENT_ASSIGNED_ADC = 0
};

pwm_input_pin PE2_in = {

             .ADC_PERIPHERAL = SYSCTL_PERIPH_ADC0,
             .GPIO_PORT_BASE = GPIO_PORTE_BASE,
             .INTERNAL_PIN_NUM = GPIO_PIN_2,
             .ADC_BASE = ADC0_BASE,

             .SEQUENCER_NUMBER = 3,
             .FIFO_BUFFER_DEPTH = 1,
             .SEQUENCER_STEPS = 1,
             .CURRENT_ASSIGNED_ADC = 0
};

pwm_input_pin PE1_in = {

             .ADC_PERIPHERAL = SYSCTL_PERIPH_ADC0,
             .GPIO_PORT_BASE = GPIO_PORTE_BASE,
             .INTERNAL_PIN_NUM = GPIO_PIN_1,
             .ADC_BASE = ADC0_BASE,

             .SEQUENCER_NUMBER = 3,
             .FIFO_BUFFER_DEPTH = 1,
             .SEQUENCER_STEPS = 1,
             .CURRENT_ASSIGNED_ADC = 0
};

pwm_input_pin PE0_in = {

             .ADC_PERIPHERAL = SYSCTL_PERIPH_ADC0,
             .GPIO_PORT_BASE = GPIO_PORTE_BASE,
             .INTERNAL_PIN_NUM = GPIO_PIN_0,
             .ADC_BASE = ADC0_BASE,

             .SEQUENCER_NUMBER = 3,
             .FIFO_BUFFER_DEPTH = 1,
             .SEQUENCER_STEPS = 1,
             .CURRENT_ASSIGNED_ADC = 0
};

pwm_input_pin PD3_in = {

             .ADC_PERIPHERAL = SYSCTL_PERIPH_ADC0,
             .GPIO_PORT_BASE = GPIO_PORTD_BASE,
             .INTERNAL_PIN_NUM = GPIO_PIN_3,
             .ADC_BASE = ADC0_BASE,

             .SEQUENCER_NUMBER = 3,
             .FIFO_BUFFER_DEPTH = 1,
             .SEQUENCER_STEPS = 1,
             .CURRENT_ASSIGNED_ADC = 0
};

pwm_input_pin PD2_in = {

             .ADC_PERIPHERAL = SYSCTL_PERIPH_ADC0,
             .GPIO_PORT_BASE = GPIO_PORTD_BASE,
             .INTERNAL_PIN_NUM = GPIO_PIN_2,
             .ADC_BASE = ADC0_BASE,

             .SEQUENCER_NUMBER = 3,
             .FIFO_BUFFER_DEPTH = 1,
             .SEQUENCER_STEPS = 1,
             .CURRENT_ASSIGNED_ADC = 0
};

pwm_input_pin PD1_in = {

             .ADC_PERIPHERAL = SYSCTL_PERIPH_ADC0,
             .GPIO_PORT_BASE = GPIO_PORTD_BASE,
             .INTERNAL_PIN_NUM = GPIO_PIN_1,
             .ADC_BASE = ADC0_BASE,

             .SEQUENCER_NUMBER = 3,
             .FIFO_BUFFER_DEPTH = 1,
             .SEQUENCER_STEPS = 1,
             .CURRENT_ASSIGNED_ADC = 0
};

pwm_input_pin PD0_in = {

             .ADC_PERIPHERAL = SYSCTL_PERIPH_ADC0,
             .GPIO_PORT_BASE = GPIO_PORTD_BASE,
             .INTERNAL_PIN_NUM = GPIO_PIN_0,
             .ADC_BASE = ADC0_BASE,

             .SEQUENCER_NUMBER = 3,
             .FIFO_BUFFER_DEPTH = 1,
             .SEQUENCER_STEPS = 1,
             .CURRENT_ASSIGNED_ADC = 0
};

pwm_input_pin PE5_in = {

             .ADC_PERIPHERAL = SYSCTL_PERIPH_ADC0,
             .GPIO_PORT_BASE = GPIO_PORTE_BASE,
             .INTERNAL_PIN_NUM = GPIO_PIN_5,
             .ADC_BASE = ADC0_BASE,

             .SEQUENCER_NUMBER = 3,
             .FIFO_BUFFER_DEPTH = 1,
             .SEQUENCER_STEPS = 1,
             .CURRENT_ASSIGNED_ADC = 0
};

pwm_input_pin PE4_in = {

             .ADC_PERIPHERAL = SYSCTL_PERIPH_ADC0,
             .GPIO_PORT_BASE = GPIO_PORTE_BASE,
             .INTERNAL_PIN_NUM = GPIO_PIN_4,
             .ADC_BASE = ADC0_BASE,

             .SEQUENCER_NUMBER = 3,
             .FIFO_BUFFER_DEPTH = 1,
             .SEQUENCER_STEPS = 1,
             .CURRENT_ASSIGNED_ADC = 0
};

pwm_input_pin PB4_in = {

             .ADC_PERIPHERAL = SYSCTL_PERIPH_ADC0,
             .GPIO_PORT_BASE = GPIO_PORTB_BASE,
             .INTERNAL_PIN_NUM = GPIO_PIN_4,
             .ADC_BASE = ADC0_BASE,

             .SEQUENCER_NUMBER = 3,
             .FIFO_BUFFER_DEPTH = 1,
             .SEQUENCER_STEPS = 1,
             .CURRENT_ASSIGNED_ADC = 0
};

pwm_input_pin PB5_in = {

             .ADC_PERIPHERAL = SYSCTL_PERIPH_ADC0,
             .GPIO_PORT_BASE = GPIO_PORTB_BASE,
             .INTERNAL_PIN_NUM = GPIO_PIN_5,
             .ADC_BASE = ADC0_BASE,

             .SEQUENCER_NUMBER = 3,
             .FIFO_BUFFER_DEPTH = 1,
             .SEQUENCER_STEPS = 1,
             .CURRENT_ASSIGNED_ADC = 0
};

pwm_input_pin getInputPinFromNumber(int pinnum);
pwm_input_pin getInputPinFromNumber(int pinnum)
{
    switch(pinnum)
    {
    case 6:
        return PE3_in;
    case 7:
        return PE2_in;
    case 8:
        return PE1_in;
    case 9:
        return PE0_in;
    case 64:
        return PD3_in;
    case 63:
        return PD2_in;
    case 62:
        return PD1_in;
    case 61:
        return PD0_in;
    case 60:
        return PE5_in;
    case 59:
        return PE4_in;
    case 58:
        return PB4_in;
    case 57:
        return PB5_in;
    default:
        return dummy_pin_in;

    }
}

#endif /* PWM_INPUT_PIN_H_ */
