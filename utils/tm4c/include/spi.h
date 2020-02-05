/*
 * spi.h
 *
 *  Created on: Feb 1, 2020
 *      Author: Roey
 */

#pragma once

#include "stdint.h"
#include "driverlib/ssi.h"

struct SPIInterfaceParams
{
    uint32_t protocol = SSI_FRF_MOTO_MODE_0;
    uint32_t mode = SSI_MODE_MASTER;
    uint32_t bitrate = 10000;
    uint32_t dataWidth = 8;
};

/**
 * A simple SPI interface which allows the user to use default options or set their own.
 */
class SPIInterface
{
private:
    // Store the base number
    uint32_t spi_base;
public:
    SPIInterface(uint32_t portNumber = 0, SPIInterfaceParams* params = nullptr);

    void put(uint32_t data);

    void get(uint32_t* out_data);
};
