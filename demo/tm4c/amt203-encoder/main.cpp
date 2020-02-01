#include "corona/corona.h"

#include "corona/spi.h"
#include "corona/uart.h"

#define NOP 0x00
#define RD_POS 0x00000010
#define ZR_POS 0x0000000e

void setup()
{
    enableSPI0();
}

uint32_t resp;
uint16_t pos;
float angle;

void loop()
{
    putSPI0(RD_POS);

    while (resp != RD_POS)
    {
        getSPI0(&resp);
        putSPI0(NOP);
    }

    putSPI0(NOP);
    getSPI0(&resp);

    pos = (resp & 0x0f) << 8;

    putSPI0(NOP);
    getSPI0(&resp);

    pos += (resp & 0xff);
    angle = (float)pos*360.0/4096.0;
}

INITIALIZE()
