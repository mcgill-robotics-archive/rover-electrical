/*
 * spi.cpp
 *
 *  Created on: Feb 1, 2020
 *      Author: Roey
 */
#include "include/spi.h"

#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

SPIInterface::SPIInterface(uint32_t portNumber, SPIInterfaceParams* params)
{
    switch (portNumber)
    {
    case 0:
        // Set up the peripherals
        SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
        // Set up the GPIO pins
        GPIOPinConfigure(GPIO_PA2_SSI0CLK);
        GPIOPinConfigure(GPIO_PA3_SSI0FSS);
        GPIOPinConfigure(GPIO_PA4_SSI0RX);
        GPIOPinConfigure(GPIO_PA5_SSI0TX);

        spi_base = SSI0_BASE;
        break;
    case 1:
        // Set up the peripherals
        SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
        // Set up the GPIO pins
        GPIOPinConfigure(GPIO_PF2_SSI1CLK);
        GPIOPinConfigure(GPIO_PF3_SSI1FSS);
        GPIOPinConfigure(GPIO_PF0_SSI1RX);
        GPIOPinConfigure(GPIO_PF1_SSI1TX);

        spi_base = SSI1_BASE;
        break;
    case 2:
        SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
        GPIOPinConfigure(GPIO_PB4_SSI2CLK);
        GPIOPinConfigure(GPIO_PB5_SSI2FSS);
        GPIOPinConfigure(GPIO_PB6_SSI2RX);
        GPIOPinConfigure(GPIO_PB7_SSI2TX);

        spi_base = SSI2_BASE;
        break;
    case 3:
        SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
        GPIOPinConfigure(GPIO_PD0_SSI3CLK);
        GPIOPinConfigure(GPIO_PD1_SSI3FSS);
        GPIOPinConfigure(GPIO_PD2_SSI3RX);
        GPIOPinConfigure(GPIO_PD3_SSI3TX);

        spi_base = SSI3_BASE;
        break;
    default:
        break;
    }

    if (params == nullptr)
        SSIConfigSetExpClk(spi_base, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 10000, 8);
    else
        SSIConfigSetExpClk(spi_base, SysCtlClockGet(), params->protocol, params->mode, params->bitrate, params->dataWidth);

    SSIEnable(spi_base);
}

void SPIInterface::put(uint32_t data)
{
    SSIDataPut(spi_base, data);
}

void SPIInterface::get(uint32_t* out_data)
{
    SSIDataGet(spi_base, out_data);
}
