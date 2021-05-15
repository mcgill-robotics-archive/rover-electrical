
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
    //SYSCTL_PERIPH_I2C0, I2C0_BASE are macros and 0x00 is a placeholder for the slave address
    intI2C_CONFIG(SYSCTL_PERIPH_I2C0, I2C0_BASE,0x00, true, 0x000);
}

/* Enable i2c peripherial
* I2CMasterInitExpClk
* set the slave address
* enable the communication config*/
void intI2C_CONFIG(uint32_t ui32Peripheral, uint32_t ui32Base, uint8_t ui8SlaveAddr, bool bFast, uint32_t ui32Config){
    //
        // Enable the I2C0 peripheral
        //
        SysCtlPeripheralEnable(ui32Peripheral);
        //
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
        //
        I2CMasterSlaveAddrSet(ui32Base, ui8SlaveAddr, false);

        I2CRxFIFOConfigSet(ui32Base, ui32Config);

}



