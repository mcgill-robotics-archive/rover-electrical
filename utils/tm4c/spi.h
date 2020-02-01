/*
 * spi.h
 *
 *  Created on: Feb 1, 2020
 *      Author: Roey
 */

#pragma once

#include "corona.h"

#include "inc/hw_ssi.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

void enableSPI0();

void putSPI0(uint32_t data);

void getSPI0(uint32_t* out_data);
