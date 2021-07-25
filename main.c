#include <stdint.h>                         // Library of Standard Integer Types
#include <stdbool.h>                        // Library of Standard Boolean Types
#include "inc/tm4c123gh6pm.h"               // Definitions for interrupt and register assignments on Tiva C
#include "inc/hw_memmap.h"                  // Macros defining the memory map of the Tiva C Series device
#include "inc/hw_types.h"                   // Defines common types and macros
#include "inc/hw_gpio.h"                    // Defines Macros for GPIO hardware
#include "inc/hw_pwm.h"                     // Defines and Macros for Pulse Width Modulation (PWM) ports
#include "driverlib/debug.h"                // Macros for assisting debug of the driver library
#include "driverlib/pwm.h"                  // API function prototypes for PWM ports
#include "driverlib/sysctl.h"               // Defines and macros for System Control API of DriverLib
#include "driverlib/interrupt.h"            // Defines and macros for NVIC Controller API of DriverLib
#include "driverlib/gpio.h"                 // Defines and macros for GPIO API of DriverLib
#include "driverlib/pin_map.h"              // Mapping of peripherals to pins for all parts
#include "driverlib/rom.h"                  // Defines and macros for ROM API of driverLib
#include "driverlib/rom_map.h"
#include "pwm_output_pin.h"
#include "driverlib/adc.h"
#include "pwm_input_pin.h"
#define PWM_FREQUENCY 55
#define PIN_MAX_VOLTAGE 3.3

// TODO: make the following functions (the funct signatures might differ)
//void pwm_write(int gpio_bank, int pin_num, int value);
//void pwm_read(int GPIO_PORTF_BASE, int GPIO_PIN_4, int GPIO_PIN_0);
//void pwm_init_bank(int GPIO_PORTF_BASE, int GPIO_PORTD_BASE, int GPIO_PIN_4, int GPIO_PIN_0);
//void pwm_set_input(int GPIO_PORTF_BASE, int GPIO_PORTD_BASE, int GPIO_PIN_4, int GPIO_PIN_0);
//void pwm_set_output(int GPIO_PORTF_BASE, int GPIO_PORTD_BASE, int GPIO_PIN_4, int GPIO_PIN_0);

bool pwm_init_bank(){
    // run the CPU at 40MHz, set the PWM clock for the first time.
    // run the PWM clock at 40MHz / 64 = 625 kHz
    MAP_SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    MAP_SysCtlPWMClockSet(SYSCTL_PWMDIV_64);
    return true;
}

bool ADC0_IS_OCCUPIED = false;
bool ADC1_IS_OCCUPIED = false;

// ADC0, GPIO Port E, AIN0 - PE3
//int FIFO_BUFFER_DEPTH = 1;
//int ADC_PERIPHERAL = SYSCTL_PERIPH_ADC0;
//int GPIO_PORT_BASE = GPIO_PORTE_BASE;
//int INTERNAL_PIN_NUM = GPIO_PIN_3;
//int SEQUENCER_NUMBER = 3;
//int ADC_BASE = ADC0_BASE;
//int SEQUENCER_STEPS = 1;

bool pwm_set_input(int pinnum){

    pwm_input_pin TARGET_PIN = getInputPinFromNumber(pinnum);

    // If specified pin can't do ADC, fail to set it up and return
    if(TARGET_PIN.ADC_BASE == -1){
        return false;
    }

    // Making use of both onboard ADC modules.
    // If ADC0 is occupied, then use ADC1 instead. If they are
    // both occupied, overwrite ADC1 to use the new pin.

    // ADC0 occupied        ADC1 occupied       Module to use
    //      0                   0                   ADC0
    //      0                   1                   ADC0
    //      1                   0                   ADC1
    //      1                   1                   ADC1

    int ADC_PERIPHERAL, ADC_BASE;
    bool ASSIGN_TO_ADC0, ASSIGN_TO_ADC1;
    if(ADC0_IS_OCCUPIED){
        ADC_PERIPHERAL = SYSCTL_PERIPH_ADC1;
        ADC_BASE = ADC1_BASE;
        ASSIGN_TO_ADC1 = true;
        ASSIGN_TO_ADC0 = false;
    }else{
        // The pwm_input_pin struct defaults to ADC0
        ADC_PERIPHERAL = TARGET_PIN.ADC_PERIPHERAL;
        ADC_BASE = TARGET_PIN.ADC_BASE;
        ASSIGN_TO_ADC1 = false;
        ASSIGN_TO_ADC0 = true;
    }

    // Enable the ADC peripheral the pin is attached to
    SysCtlPeripheralEnable(ADC_PERIPHERAL);

    // Enable the GPIO port for this pin
    SysCtlPeripheralEnable(TARGET_PIN.GPIO_PORT_BASE);

    // Select the ADC function for this pin
    GPIOPinTypeADC(TARGET_PIN.GPIO_PORT_BASE, TARGET_PIN.INTERNAL_PIN_NUM);

    // Enable a sample sequence that is triggered by software
    ADCSequenceConfigure(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER, ADC_TRIGGER_PROCESSOR, 0);

    // Configure steps, every sequencer has a different number of steps. Tell them all
    // to sample from Channel 0
    int i;
    for(i = 0; i < TARGET_PIN.SEQUENCER_STEPS; i++){
        if(i == TARGET_PIN.SEQUENCER_STEPS - 1){
            // The last step should carry the "sample end" flag
            ADCSequenceStepConfigure(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER, i, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
        }else{
            ADCSequenceStepConfigure(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER, i, ADC_CTL_CH0);
        }


    }

    // Enable sequencer
    ADCSequenceEnable(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER);

    // Clear interrupt flag before sampling
    ADCIntClear(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER);

    // Set occupied flags and keep track of which ADC the target pin is assigned to
    if(ASSIGN_TO_ADC0){
        ADC0_IS_OCCUPIED = true;
        TARGET_PIN.CURRENT_ASSIGNED_ADC = 0;
    }
    if(ASSIGN_TO_ADC1){
        ADC1_IS_OCCUPIED = true;
        TARGET_PIN.CURRENT_ASSIGNED_ADC = 1;
    }

    return true;

}

double pwm_read(int pinnum){

    pwm_input_pin TARGET_PIN = getInputPinFromNumber(pinnum);

    // If specified pin can't do ADC, fail to read from it and return
    if(TARGET_PIN.ADC_BASE == -1){
        return false;
    }

    // Get which ADC module the current pin is assigned to
    int ADC_BASE;
    if(TARGET_PIN.CURRENT_ASSIGNED_ADC == 1){
        ADC_BASE = ADC1_BASE;
    }else{
        ADC_BASE = ADC0_BASE;
    }

    // An array to store the ADC output, must be the same size
    // as the FIFO buffer of the sequencer the pin is attached to
    uint32_t pui32ADCvalue[TARGET_PIN.FIFO_BUFFER_DEPTH];

    // Trigger conversion
    ADCProcessorTrigger(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER);

    // Wait for conversion to complete
    while(!ADCIntStatus(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER, false)){}

    // Clear ADC interrupt flag
    ADCIntClear(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER);

    // Read value from result register
    ADCSequenceDataGet(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER, pui32ADCvalue);

    // Potential extra feature: Use the hardware averager instead of using
    // the top of the array

    // Vref = 3.3V, 3.3 / 4096 = 0.8057 mV per increment

    double voltage = (0.8057 * pui32ADCvalue[0])/1000;
    return voltage;
}

bool pwm_write(int pinnum, int millivolts){

    pwm_output_pin TARGET_PIN = getPinFromNumber(pinnum);

    // If pin is invalid, fail to write and return
    if(TARGET_PIN.PWM_MODULE == -1){
        return false;
    }

    //  (pulse width / pwm period) = (given voltage / maximum pwm voltage)
    //  pulse width(ms) = (voltage(v) / maximum pwm voltage(v)) * pwm period(ms)
    //  pwm period = 1/55 Hz = 18.2 ms
    //  pulse width = (voltage / maximum pwm voltage) * 18.2 ms
    //  pulse width (in pwm clock ticks) = pulse width(ms) / (pulse resolution)
    //  where pulse resolution = pwm period / 1000 = 18.2 / 1000 (pulse resolution given in the tutorial example, could be changed to be more sensitive)
    //  pulse width (in pwm clock ticks) = pulse width(ms) / (18.2 / 1000)
    //                                   = 1000 * pulse width (ms) / 18.2
    //                                   = 1000 * [(voltage / maximum pwm voltage) * 18.2] / 18.2
    //                                   = 1000 * (voltage / maximum pwm voltage)

    int voltage = millivolts / 1000;

    int pulse_width = 1000 * (voltage / PIN_MAX_VOLTAGE);

    PWMPulseWidthSet(TARGET_PIN.PWM_BASE, TARGET_PIN.PWM_OUT, pulse_width);
    return true;
}

bool pwm_set_output(int pinnum){

    pwm_output_pin TARGET_PIN = getPinFromNumber(pinnum);

    if(TARGET_PIN.PWM_MODULE == -1){
        // If I'm reading the datasheet right, not every pin can do PWM.
        // If the pin specified doesn't have a designated PWM generator attached
        // to it, fail to set up the pin and return.
        // See TM4C datasheet page 1233: https://www.ti.com/lit/ds/spms376e/spms376e.pdf
        return false;
    }

    // Enable necessary PWM and GPIO modules
    MAP_SysCtlPeripheralEnable(TARGET_PIN.PWM_MODULE);
    MAP_SysCtlPeripheralEnable(TARGET_PIN.GPIO_MODULE);

    // If the pin to be enabled is PF0, we need to unlock the GPIO commit control register
    // before we can set the pin as something else than GPIO.

    if(TARGET_PIN.PORT_BASE == GPIO_PORTF_BASE && TARGET_PIN.INTERNAL_PIN_NUM == GPIO_PIN_0){
        HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
        HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
        HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
    }

    // Set the pin as the output pin of its designated PWM generator
    MAP_GPIOPinTypePWM(TARGET_PIN.PORT_BASE, TARGET_PIN.INTERNAL_PIN_NUM);
    MAP_GPIOPinConfigure(TARGET_PIN.PWM_SETUP_CODE);

    // Math for the down-counting register
    volatile uint32_t ui32Load;
    volatile uint32_t ui32PWMClock;
    volatile uint8_t ui8Adjust;
    ui8Adjust = 83;

    ui32PWMClock = SysCtlClockGet() / 64;
    ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
    PWMGenConfigure(TARGET_PIN.PWM_BASE, TARGET_PIN.PWM_GEN, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(TARGET_PIN.PWM_BASE, TARGET_PIN.PWM_GEN, ui32Load);

    // At this point the PWM output should be configured already. The PWM starts with a
    // 50% duty cycle by default but it could be changed.
    PWMPulseWidthSet(TARGET_PIN.PWM_BASE, TARGET_PIN.PWM_OUT, 500);
    PWMOutputState(TARGET_PIN.PWM_BASE, TARGET_PIN.PWM_OUT_BIT, true);
    PWMGenEnable(TARGET_PIN.PWM_BASE, TARGET_PIN.PWM_GEN);

    return true;

}

int main(void)
{
    // LED's:
    // PF1 - 29 - Red, PF2 - 30 - Blue, PF3 - 31 - Green

    // Initialize everything
    pwm_init_bank();
    
    pwm_set_input(6);
    
    double voltage; 
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){
    }
    
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
    
   while(1){
      voltage = pwm_read(6);
      if (voltage > 1. 65){
          GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
      }
      else
          GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);
   }
     
    
    
//     int pinnum;
//     for(pinnum = 29; pinnum <= 31; pinnum++){
//         // Set the pin as PWM output
//         pwm_set_output(pinnum);

//         // Sweep the duty cycle range from 0% to 100% and back
//         // a couple of times
//         int i,j;
//         for(j=0; j < 100; j++){

//             for(i=0; i <= 3300; i++){
//                 pwm_write(pinnum, i);
//                 }
//             for(i=3300; i >= 0; i--){
//                 pwm_write(pinnum, i);
//                 }
//         }
        // Wait for 5 seconds and do the same with other two LED's
        //delaySeconds(5);
//     }
}

void delaySeconds(int seconds){
    SysCtlDelay((SysCtlClockGet() * seconds) / 3);
}
