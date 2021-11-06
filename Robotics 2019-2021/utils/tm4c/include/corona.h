/*
 * corona.h
 *
 *  Created on: Feb 1, 2020
 *      Author: Roey
 */

#pragma once

#include "stdint.h"
#include "stdbool.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"


#define INITIALIZE()    \
int main()              \
{                       \
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);\
    setup();            \
    while (true) loop();\
}
