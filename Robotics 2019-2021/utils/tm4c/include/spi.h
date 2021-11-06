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
    // Store the base number (SSI0_BASE, SSI1_base, etc.)
    uint32_t spi_base;
public:
    SPIInterface(uint32_t portNumber = 0, SPIInterfaceParams* params = nullptr);

    /**
     * Writes the n least significant bits of `data` over SPI
     * where n is the dataWidth chosen during initialization.
     */
    void put(uint32_t data);

    /**
     * Reads and stores the n least significant bits from SPI to `out_data`
     * where n is the dataWidth chosen during initialization.
     */
    void get(uint32_t* out_data);
};
