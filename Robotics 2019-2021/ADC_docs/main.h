#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"

bool initADC_CONFIG(int pinnum){

    input_pin TARGET_PIN = getInputPinFromNumber(pinnum);

    // If specified pin can't do ADC, fail to set it up and return
        if(TARGET_PIN.ADC_BASE == -1){
            return false;
        }

        // Making use of both onboard ADC modules.
           // If ADC0 is occupied, then use ADC1 instead. If they are
           // both occupied, overwrite ADC1 to use the new pin.
       ​
           // ADC0 occupied        ADC1 occupied       Module to use
           //      0                   0                   ADC0
           //      0                   1                   ADC0
           //      1                   0                   ADC1
           //      1                   1                   ADC1
       ​
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
       ​
           // Enable the ADC peripheral the pin is attached to
           SysCtlPeripheralEnable(ADC_PERIPHERAL);
       ​
           // Enable the GPIO port for this pin
           SysCtlPeripheralEnable(TARGET_PIN.GPIO_PORT_BASE);
       ​
           // Select the ADC function for this pin
           GPIOPinTypeADC(TARGET_PIN.GPIO_PORT_BASE, TARGET_PIN.INTERNAL_PIN_NUM);
       ​
           // Enable a sample sequence that is triggered by software
           ADCSequenceConfigure(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER, ADC_TRIGGER_PROCESSOR, 0);
       ​
           // Configure steps, every sequencer has a different number of steps. Tell them all
           // to sample from Channel 0
           for(int i = 0; i < TARGET_PIN.SEQUENCER_STEPS; i++){
               if(i == TARGET_PIN.SEQUENCER_STEPS - 1){
                   // The last step should carry the "sample end" flag
                   ADCSequenceStepConfigure(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER, i, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
               }else{
                   ADCSequenceStepConfigure(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER, i, ADC_CTL_CH0);
               }​
       ​
           }
       ​
           // Enable sequencer
           ADCSequenceEnable(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER);
       ​
           // Clear interrupt flag before sampling
           ADCIntClear(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER);
       ​
           // Set occupied flags and keep track of which ADC the target pin is assigned to
           if(ASSIGN_TO_ADC0){
               ADC0_IS_OCCUPIED = true;
               TARGET_PIN.CURRENT_ASSIGNED_ADC = 0;
           }
           if(ASSIGN_TO_ADC1){
               ADC1_IS_OCCUPIED = true;
               TARGET_PIN.CURRENT_ASSIGNED_ADC = 1;
           }
       ​
           return true;

}

double ADC_read(int pinnum){
        input_pin TARGET_PIN = getInputPinFromNumber(pinnum);
    ​
        // If specified pin can't do ADC, fail to read from it and return
        if(TARGET_PIN.ADC_BASE == -1){
            return false;
        }
    ​
        int ADC_BASE;
        if(TARGET_PIN.CURRENT_ASSIGNED_ADC == 1){
            ADC_BASE = ADC1_BASE;
        }else{
            ADC_BASE = ADC0_BASE;
        }
    ​
        // An array to store the ADC output, must be the same size
        // as the FIFO buffer of the sequencer the pin is attached to
        uint32_t pui32ADCvalue[TARGET_PIN.FIFO_BUFFER_DEPTH];
    ​
        // Trigger conversion
        ADCProcessorTrigger(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER);
    ​
        // Wait for conversion to complete
        while(!ADCIntStatus(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER, false)){}
    ​
        // Clear ADC interrupt flag
        ADCIntClear(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER);
    ​
        // Read value from result register
        ADCSequenceDataGet(ADC_BASE, TARGET_PIN.SEQUENCER_NUMBER, pui32ADCvalue);
}