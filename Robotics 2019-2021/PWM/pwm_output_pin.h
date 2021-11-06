/*
 * pwm_output_pin.h
 *
 *  Created on: 31 May 2021
 *      Author: Eren
 */

#ifndef PWM_OUTPUT_PIN_H_
#define PWM_OUTPUT_PIN_H_

struct pwm_out
{
    int PWM_MODULE;
    int PWM_GEN;
    int PWM_OUT;
    int PWM_OUT_BIT;
    int GPIO_MODULE;
    int PORT_BASE;
    int INTERNAL_PIN_NUM;
    int PWM_BASE;
    int PWM_SETUP_CODE;

};

typedef struct pwm_out pwm_output_pin;

pwm_output_pin output_pin_numbers[63];

pwm_output_pin PB6 = {
    .PWM_MODULE = SYSCTL_PERIPH_PWM0,
    .PWM_GEN = PWM_GEN_0,
    .PWM_OUT = PWM_OUT_0,
    .PWM_OUT_BIT = PWM_OUT_0_BIT,
    .GPIO_MODULE = SYSCTL_PERIPH_GPIOB,
    .PORT_BASE = GPIO_PORTB_BASE,
    .INTERNAL_PIN_NUM = GPIO_PIN_6,
    .PWM_BASE = PWM0_BASE,
    .PWM_SETUP_CODE = GPIO_PB6_M0PWM0
};

pwm_output_pin PB7 = {
    .PWM_MODULE = SYSCTL_PERIPH_PWM0,
    .PWM_GEN = PWM_GEN_0,
    .PWM_OUT = PWM_OUT_0,
    .PWM_OUT_BIT = PWM_OUT_0_BIT,
    .GPIO_MODULE = SYSCTL_PERIPH_GPIOB,
    .PORT_BASE = GPIO_PORTB_BASE,
    .INTERNAL_PIN_NUM = GPIO_PIN_7,
    .PWM_BASE = PWM0_BASE,
    .PWM_SETUP_CODE = GPIO_PB7_M0PWM1
};

pwm_output_pin PB4 = {
    .PWM_MODULE = SYSCTL_PERIPH_PWM0,
    .PWM_GEN = PWM_GEN_1,
    .PWM_OUT = PWM_OUT_1,
    .PWM_OUT_BIT = PWM_OUT_1_BIT,
    .GPIO_MODULE = SYSCTL_PERIPH_GPIOB,
    .PORT_BASE = GPIO_PORTB_BASE,
    .INTERNAL_PIN_NUM = GPIO_PIN_4,
    .PWM_BASE = PWM0_BASE,
    .PWM_SETUP_CODE = GPIO_PB4_M0PWM2
};

pwm_output_pin PB5 = {
    .PWM_MODULE = SYSCTL_PERIPH_PWM0,
    .PWM_GEN = PWM_GEN_1,
    .PWM_OUT = PWM_OUT_1,
    .PWM_OUT_BIT = PWM_OUT_1_BIT,
    .GPIO_MODULE = SYSCTL_PERIPH_GPIOB,
    .PORT_BASE = GPIO_PORTB_BASE,
    .INTERNAL_PIN_NUM = GPIO_PIN_5,
    .PWM_BASE = PWM0_BASE,
    .PWM_SETUP_CODE = GPIO_PB5_M0PWM3
};

pwm_output_pin PE4 = {
      .PWM_MODULE = SYSCTL_PERIPH_PWM0,
      .PWM_GEN = PWM_GEN_2,
      .PWM_OUT = PWM_OUT_2,
      .PWM_OUT_BIT = PWM_OUT_2_BIT,
      .GPIO_MODULE = SYSCTL_PERIPH_GPIOE,
      .PORT_BASE = GPIO_PORTE_BASE,
      .INTERNAL_PIN_NUM = GPIO_PIN_4,
      .PWM_BASE = PWM0_BASE,
      .PWM_SETUP_CODE = GPIO_PE4_M0PWM4
};

pwm_output_pin PE5 = {
      .PWM_MODULE = SYSCTL_PERIPH_PWM0,
      .PWM_GEN = PWM_GEN_2,
      .PWM_OUT = PWM_OUT_2,
      .PWM_OUT_BIT = PWM_OUT_2_BIT,
      .GPIO_MODULE = SYSCTL_PERIPH_GPIOE,
      .PORT_BASE = GPIO_PORTE_BASE,
      .INTERNAL_PIN_NUM = GPIO_PIN_5,
      .PWM_BASE = PWM0_BASE,
      .PWM_SETUP_CODE = GPIO_PE5_M0PWM5
};

pwm_output_pin PC4 = {
      .PWM_MODULE = SYSCTL_PERIPH_PWM0,
      .PWM_GEN = PWM_GEN_3,
      .PWM_OUT = PWM_OUT_3,
      .PWM_OUT_BIT = PWM_OUT_3_BIT,
      .GPIO_MODULE = SYSCTL_PERIPH_GPIOC,
      .PORT_BASE = GPIO_PORTC_BASE,
      .INTERNAL_PIN_NUM = GPIO_PIN_4,
      .PWM_BASE = PWM0_BASE,
      .PWM_SETUP_CODE = GPIO_PC4_M0PWM6
};

pwm_output_pin PD0 = {
      .PWM_MODULE = SYSCTL_PERIPH_PWM1,
      .PWM_GEN = PWM_GEN_0,
      .PWM_OUT = PWM_OUT_0,
      .PWM_OUT_BIT = PWM_OUT_0_BIT,
      .GPIO_MODULE = SYSCTL_PERIPH_GPIOD,
      .PORT_BASE = GPIO_PORTD_BASE,
      .INTERNAL_PIN_NUM = GPIO_PIN_0,
      .PWM_BASE = PWM1_BASE,
      .PWM_SETUP_CODE = GPIO_PD0_M1PWM0
};

pwm_output_pin PC5 = {
      .PWM_MODULE = SYSCTL_PERIPH_PWM0,
      .PWM_GEN = PWM_GEN_3,
      .PWM_OUT = PWM_OUT_3,
      .PWM_OUT_BIT = PWM_OUT_3_BIT,
      .GPIO_MODULE = SYSCTL_PERIPH_GPIOC,
      .PORT_BASE = GPIO_PORTC_BASE,
      .INTERNAL_PIN_NUM = GPIO_PIN_5,
      .PWM_BASE = PWM0_BASE,
      .PWM_SETUP_CODE = GPIO_PC5_M0PWM7
};

pwm_output_pin PD1 = {
      .PWM_MODULE = SYSCTL_PERIPH_PWM1,
      .PWM_GEN = PWM_GEN_0,
      .PWM_OUT = PWM_OUT_0,
      .PWM_OUT_BIT = PWM_OUT_0_BIT,
      .GPIO_MODULE = SYSCTL_PERIPH_GPIOD,
      .PORT_BASE = GPIO_PORTD_BASE,
      .INTERNAL_PIN_NUM = GPIO_PIN_1,
      .PWM_BASE = PWM1_BASE,
      .PWM_SETUP_CODE = GPIO_PD1_M1PWM1
};

pwm_output_pin PA6 = {
      .PWM_MODULE = SYSCTL_PERIPH_PWM1,
      .PWM_GEN = PWM_GEN_1,
      .PWM_OUT = PWM_OUT_1,
      .PWM_OUT_BIT = PWM_OUT_1_BIT,
      .GPIO_MODULE = SYSCTL_PERIPH_GPIOA,
      .PORT_BASE = GPIO_PORTA_BASE,
      .INTERNAL_PIN_NUM = GPIO_PIN_6,
      .PWM_BASE = PWM1_BASE,
      .PWM_SETUP_CODE = GPIO_PA6_M1PWM2
};

pwm_output_pin PA7 = {
      .PWM_MODULE = SYSCTL_PERIPH_PWM1,
      .PWM_GEN = PWM_GEN_1,
      .PWM_OUT = PWM_OUT_1,
      .PWM_OUT_BIT = PWM_OUT_1_BIT,
      .GPIO_MODULE = SYSCTL_PERIPH_GPIOA,
      .PORT_BASE = GPIO_PORTA_BASE,
      .INTERNAL_PIN_NUM = GPIO_PIN_7,
      .PWM_BASE = PWM1_BASE,
      .PWM_SETUP_CODE = GPIO_PA7_M1PWM3
};

pwm_output_pin PF0 = {
      .PWM_MODULE = SYSCTL_PERIPH_PWM1,
      .PWM_GEN = PWM_GEN_2,
      .PWM_OUT = PWM_OUT_2,
      .PWM_OUT_BIT = PWM_OUT_2_BIT,
      .GPIO_MODULE = SYSCTL_PERIPH_GPIOF,
      .PORT_BASE = GPIO_PORTF_BASE,
      .INTERNAL_PIN_NUM = GPIO_PIN_0,
      .PWM_BASE = PWM1_BASE,
      .PWM_SETUP_CODE = GPIO_PF0_M1PWM4
};

pwm_output_pin PF1 = {
      .PWM_MODULE = SYSCTL_PERIPH_PWM1,
      .PWM_GEN = PWM_GEN_2,
      .PWM_OUT = PWM_OUT_2,
      .PWM_OUT_BIT = PWM_OUT_2_BIT,
      .GPIO_MODULE = SYSCTL_PERIPH_GPIOF,
      .PORT_BASE = GPIO_PORTF_BASE,
      .INTERNAL_PIN_NUM = GPIO_PIN_1,
      .PWM_BASE = PWM1_BASE,
      .PWM_SETUP_CODE = GPIO_PF1_M1PWM5
};

pwm_output_pin PF2 = {
      .PWM_MODULE = SYSCTL_PERIPH_PWM1,
      .PWM_GEN = PWM_GEN_3,
      .PWM_OUT = PWM_OUT_3,
      .PWM_OUT_BIT = PWM_OUT_3_BIT,
      .GPIO_MODULE = SYSCTL_PERIPH_GPIOF,
      .PORT_BASE = GPIO_PORTF_BASE,
      .INTERNAL_PIN_NUM = GPIO_PIN_2,
      .PWM_BASE = PWM1_BASE,
      .PWM_SETUP_CODE = GPIO_PF2_M1PWM6
};

pwm_output_pin PF3 = {
      .PWM_MODULE = SYSCTL_PERIPH_PWM1,
      .PWM_GEN = PWM_GEN_3,
      .PWM_OUT = PWM_OUT_3,
      .PWM_OUT_BIT = PWM_OUT_3_BIT,
      .GPIO_MODULE = SYSCTL_PERIPH_GPIOF,
      .PORT_BASE = GPIO_PORTF_BASE,
      .INTERNAL_PIN_NUM = GPIO_PIN_3,
      .PWM_BASE = PWM1_BASE,
      .PWM_SETUP_CODE = GPIO_PF3_M1PWM7
};

pwm_output_pin dummy_pin = {
      .PWM_MODULE = -1,
      .PWM_GEN = -1,
      .PWM_OUT = -1,
      .PWM_OUT_BIT = -1,
      .GPIO_MODULE = -1,
      .PORT_BASE = -1,
      .INTERNAL_PIN_NUM = -1,
      .PWM_BASE = -1,
      .PWM_SETUP_CODE = -1
};

pwm_output_pin getPinFromNumber(int pinnum);

pwm_output_pin getPinFromNumber(int pinnum)
{
    // Switch to map pin numbers to GPIO pins and define generators to be enabled.
    // See TM4C datasheet page 1233: https://www.ti.com/lit/ds/spms376e/spms376e.pdf

    switch(pinnum)
    {
        case 1:
            // Port B, GPIO Pin 6 - Module 0, Generator 0
            return PB6;

        case 4:
            // PB7 - Module 0, Generator 0
            return PB7;

        case 58:
            // PB4 - Module 0, Generator 1
            return PB4;

        case 57:
            // PB5 - Module 0, Generator 1
            return PB5;

        case 59:
            // PE4 - Module 0, Generator 2
            return PE4;

        case 60:
            // PE5 - Module 0, Generator 2
            return PE5;

        case 16:
            // PC4 - Module 0, Generator 3
            return PC4;

        case 61:
            // PD0 - Module 1, Generator 0
            return PD0;

        case 15:
            // PC5 - Module 0, Generator 3
            return PC5;

        case 62:
            // PD1 - Module 1, Generator 0
            return PD1;

        case 23:
            // PA6 - Module 1, Generator 1
            return PA6;

        case 24:
            // PA7 - Module 1, Generator 1
            return PA7;

        case 28:
            // PF0 - Module 1, Generator 2
            // NMI afflicted
            return PF0;

        case 29:
            // PF1 - Module 1, Generator 2
            return PF1;

        case 30:
            // PF2 - Module 1, Generator 3
            return PF2;

        case 31:
            // PF3 - Module 1, Generator 3
            return PF3;

        default:
            // If I'm reading the datasheet right, not every pin can do PWM.
            // If the pin specified doesn't have a designated PWM generator attached
            // to it, fail to set up the pin and return.
            return dummy_pin;
    }
}


#endif /* PWM_OUTPUT_PIN_H_ */
