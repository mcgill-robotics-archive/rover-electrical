
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

void intI2C_CONFIG(uint32_t ui32Peripheral, uint32_t ui32Base, uint8_t ui8SlaveAddr, bool bFast, uint32_t ui32Config);

int main(void)
{
    //SYSCTL_PERIPH_I2C0, SYSCTL_PERIPH_GPIOB I2C0_BASE are macros
    //GPIO_PB2_I2C0SCL, GPIO_PB3_I2C0SDA are the pins corresponding to the SDA and SCL for I2C Module 0
    intI2C_CONFIG(SYSCTL_PERIPH_I2C0, SYSCTL_PERIPH_GPIOB, I2C0_BASE, true, GPIO_PB2_I2C0SCL, GPIO_PB3_I2C0SDA, GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_3);
}

/* Enable i2c peripherial
* I2CMasterInitExpClk
* set the slave address
* enable the communication config*/
void intI2C_CONFIG(uint32_t ui32Peripheral, uint32_t ui32PeripheralGPIO, uint32_t ui32Base, bool bFast, uint32_t ui32PinConfigSCL, uint32_t ui32PinConfigSDA, uint32_t ui32port, uint8_t ui8pinSCL, uint8_t ui8pinSDA){
    //
        // Enable the I2C0 Module
        //
        SysCtlPeripheralEnable(ui32Peripheral);
    
        //Reset the I2C0 Module
        SysCtlPeripheralReset(ui32Peripheral); 
        
        //Enable the GPIO Peripheral for I2C 0
        SysCtlPeripheralEnable(ui32PeripheralGPIO);
        
        //Configure pin muxing for I2C0 functions on port B2 and B3
        GPIOPinConfigure(ui32PinConfigSCL); 
        GPIOPinConfigure(ui32PinConfigSDA);
        
        //Select I2C functions for the pins
        GPIOPinTypeI2C(ui32port, ui8pinSDA); 
        GPIOPinTypeI2CSCL(ui32port, ui8pinSCL);
        // Wait for the I2C0 module to be ready.
        //
        while(!SysCtlPeripheralReady(ui32Peripheral))
        {
        }
        //
        // Initialize Master and Slave
        //
        I2CMasterInitExpClk(ui32Base, SysCtlClockGet(), bFast);
        //
        // Specify slave address
        //Commented out because the false bool is only for writing...we should make this line in the read/write method later
        //I2CMasterSlaveAddrSet(ui32Base, ui8SlaveAddr, false);
        //commented out because I don't think our MCU has a FIFO for i2c
        //I2CRxFIFOConfigSet(ui32Base, ui32Config);

}



