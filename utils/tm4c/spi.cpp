/*
 * spi.cpp
 *
 *  Created on: Feb 1, 2020
 *      Author: Roey
 */
#include "spi.h"

void enableSPI0()
{
    // Initialize Peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Initialize the GPIO pins
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2);

    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 10000, 8);
    SSIEnable(SSI0_BASE);
}

void putSPI0(uint32_t data)
{
    SSIDataPut(SSI0_BASE, data);
}

void getSPI0(uint32_t* out_data)
{
    SSIDataGet(SSI0_BASE, out_data);
}


