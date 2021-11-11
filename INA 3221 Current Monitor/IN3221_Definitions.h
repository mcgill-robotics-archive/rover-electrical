#include "Arduino.h"
#include <Wire.h>

/*=============================================================
    I2C Adddress   
    I2C slave address for the monitor is GND 
    -----------------------------------------------------------*/
#define SLAVE_ADDRESS (0x40)
#define SLAVE_READ (0x01)
/*==============================================================
    Config register (R/W)
    -----------------------------------------------------------*/
#define SLAVE_CONFIG_REG (0x00)

#define SLAVE_CONGIF_BYTE1 (0x1D)
//configure register byte 1: xx011101; 588us conv time for shunt, continuos shunt only

#define SLAVE_CONFIG_BYTE2 (0xEE00)
//configure register byte 2: 1110111x; reset, chan 2 & 3 en, 1024 samples for avg 

/*=============================================================
    Shunt Voltage Register 
    -data stored in the shunt registers are represented in 2's complement form
    -----------------------------------------------------------*/
#define SLAVE_CHN1_SHNT_ADDR (0x01)
#define SLAVE_CHN2_SHNT_ADDR (0x03)

/*=============================================================
    Channel 1 Critical Alert Limit Register
    -7ff8 by default
    -load in value for critical current threshold 
    -if average values from shunt registers exceeds programmed value register is set to 0x00
    -when register set to 0x00, mask/enable register asserts bit psn for channel causing event
    -----------------------------------------------------------*/
#define SLAVE_CHN1_CRIT_ALRT (0x07)
/*channel 1 is 0.5mOhms*/
#define CHN1_RES 0.05
/*channel 2 is 1mOhm*/
#define CHN2_RES 0.1
/*channel 1 max voltage is 7.5mV*/
#define CRIT_ALRT_LIMIT_CHN1 (7.5)

/*=============================================================
    Channel 1 Critical Alert Limit Register
    -7ff8 by default
    -load in value for critical current threshold 
    -if average values from shunt registers exceeds programmed value register is set to 0x00
    -when register set to 0x00, mask/enable register asserts bit psn for channel causing event
    -----------------------------------------------------------*/
#define SLAVE_CHN2_CRIT_ALRT (0x09)
/*channel 2 is 1mOhm*/
#define CHN2_RES 0.1
/*channel 2 max voltage is 15mV*/
#define CRIT_ALRT_LIMIT_CHN2 (15)

class I2C_INA3221{
    public: 
        I2C_INA3221(uint8_t addr = SLAVE_ADDRESS, float shunt1resistor = CHN1_RES, float shunt2resistor = CHN2_RES);
        void begin(void); 
        float getShuntVoltage_mV(int channel); 
        float getCurrent_mA(int channel); 

    private: 
        float shunt1resistor; 
        float shunt2resistor; 
        uint8_t addr; 

        void wireWriteRegister(uint8_t reg, uint16_t value); 
        void wireReadRegister(uint8_t reg, uint16_t *value); 
        void configINA3221(void); 
        int16_t getShuntVoltage_raw(int channel);
}


