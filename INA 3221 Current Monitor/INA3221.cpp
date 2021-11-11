#include <Arduino.h>
#include <Wire.h>
#include "IN3221_Definitions.h"

void I2C_INA3221::wireWriteRegister(uint8_t reg, uint16_t value){
        Wire.beginTransmission(addr); 
        wire.write(reg); 
        wire.Write((value>>8) & 0xFF);
        wire.Write(value & 0xFF); 
        wire.endTransmission(); 
    //-we can only write data in 1 byte packages
}

void I2C_INA3221::wireReadRegister(uint8_t reg, uint16_t *value){
        Wire.beginTransmission(addr); 
        wire.write(reg); 
        wire.endTransmission(); 

        delay(1); 

        wire.requestFrom(addr)
        *value = ((wire.read() << 8 | wire.read());
    //-read most significant byte first then least significant byte
}

void I2C_INA3221::configINA3221(void){
        uint16_t value = SLAVE_CONFIG_BYTE2 | SLAVE_CONFIG_BYTE1
        wireReadRegister(SLAVE_CONFIG_REG, value); 
}

//creating a new instance of the class
I2C_INA3221::I2C_INA3221(uint8_t slave_addr, float shunt1_res, float shunt2_res){
        addr = slave_addr; 
        shunt1resistor = shunt1_res; 
        shunt2resistor = shunt1_res; 
}

void I2C_INA3221::begin(){
       Wire.begin(); 
       configINA3221();
}

int16_t I2C_INA3221::getShuntVoltage_raw (int channel){ 
    uint16_t value; 
    if (channel == 1){
        wireReadRegister(SLAVE_CHN1_SHNT_ADDR, &value); 
    }
    else if (channel == 2){
        wireReadRegister(SLAVE_CHN2_SHNT_ADDR, &value); 
    }
    return (int16_t)value; 
}

float I2C_INA3221::getShuntVoltage_mV(int channel){
    int16_t value = getShuntVoltage_raw(channel);
    return value * 0.001; 
}

float I2C_INA3221::getCurrent_mA(int channel){
    float current = 0;
    if(channel = 1){
        current = getShuntVoltage_mV(channel)/CHN1_RES;
    }
    else if(channel = 2){
        current = getShuntVoltage_mV(channel)/CHN2_RES;
    }
    return current; 
}