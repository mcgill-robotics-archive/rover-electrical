#include "include/corona.h"

#include "include/spi.h"

#define NOP 0x00
#define RD_POS 0x00000010
#define ZR_POS 0x0000000e

// Initialize SPI interface with default params on port 0
SPIInterface s = SPIInterface(0);

uint32_t resp;
uint16_t pos;
float angle;

void setup()
{

}

void loop()
{
    s.put(RD_POS);

    while (resp != RD_POS)
    {
        s.get(&resp);
        s.put(NOP);
    }

    s.put(NOP);
    s.get(&resp);

    pos = (resp & 0x0f) << 8;

    s.put(NOP);
    s.get(&resp);

    pos += (resp & 0xff);
    angle = (float)pos*360.0/4096.0;
}

INITIALIZE()

