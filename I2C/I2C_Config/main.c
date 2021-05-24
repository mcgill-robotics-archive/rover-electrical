
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

uint32_t data; 

//hardcoding slave address (SDA)
//#define ui8SlaveAddr 0x40

void intI2C_CONFIG(uint32_t ui32Peripheral, uint32_t ui32PeripheralGPIO, uint32_t ui32Base, bool bFast, uint32_t ui32PinConfigSCL, uint32_t ui32PinConfigSDA, uint32_t ui32port, uint8_t ui8pinSCL, uint8_t ui8pinSDA);
void write(uint8_t ui8SlaveAddr, uint32_t ui32Base, bool bReceive, uint8_t num_args, ...);
uint32_t read(uint8_t ui8SlaveAddr, uint8_t ui8reg, uint32_t ui32Base, bool bReceive); 

int main(void)
{
    //SYSCTL_PERIPH_I2C0, SYSCTL_PERIPH_GPIOB I2C0_BASE are macros
    //GPIO_PB2_I2C0SCL, GPIO_PB3_I2C0SDA are the pins corresponding to the SDA and SCL for I2C Module 0
    intI2C_CONFIG(SYSCTL_PERIPH_I2C0, SYSCTL_PERIPH_GPIOB, I2C0_BASE, true, GPIO_PB2_I2C0SCL, GPIO_PB3_I2C0SDA, GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_3);

    write(4, I2C0_BASE, false, 2, 0, 0x01);

    data = read(4, 0, I2C0_BASE, true);

    printf("%d\n", data);

}

/* Enable i2c peripherial
* I2CMasterInitExpClk
* set the slave address
* enable the communication config*/
void intI2C_CONFIG(uint32_t ui32Peripheral, uint32_t ui32PeripheralGPIO, uint32_t ui32Base, bool bFast, uint32_t ui32PinConfigSCL, uint32_t ui32PinConfigSDA, uint32_t ui32port, uint8_t ui8pinSCL, uint8_t ui8pinSDA){
    //
        // Enable the I2C0 Module
        //
        SysCtlPeripheralEnable(ui32Peripheral);
    
        //Reset the I2C0 Module
        SysCtlPeripheralReset(ui32Peripheral); 
        
        //Enable the GPIO Peripheral for I2C 0
        SysCtlPeripheralEnable(ui32PeripheralGPIO);
        
        //Configure pin muxing for I2C0 functions on port B2 and B3
        GPIOPinConfigure(ui32PinConfigSCL); 
        GPIOPinConfigure(ui32PinConfigSDA);
        
        //Select I2C functions for the pins
        GPIOPinTypeI2C(ui32port, ui8pinSDA); 
        GPIOPinTypeI2CSCL(ui32port, ui8pinSCL);
        // Wait for the I2C0 module to be ready.
        //
        while(!SysCtlPeripheralReady(ui32Peripheral)){
        }
        //
        // Initialize Master and Slave
        //
        I2CMasterInitExpClk(ui32Base, SysCtlClockGet(), bFast);
        //
        //Set Master to writing by default
        //I2CMasterSlaveAddrSet(ui32Base, ui8SlaveAddr, false);
        //commented out because I don't think our MCU has a FIFO for i2c
        //I2CRxFIFOConfigSet(ui32Base, ui32Config);

}

//Sends an I2C command to the specified slave 
//This function can be written with a variable agrument list
//Can access various registers in the slave device and write a value to that register by adding variables to the function's variable list
void write(uint8_t ui8SlaveAddr, uint32_t ui32Base, bool bReceive, uint8_t num_args, ...){
    I2CMasterSlaveAddrSet(ui32Base, ui8SlaveAddr, bReceive);
    
    //Stores list of data arguments
    va_list vargs; 
    
    //open va list 
    va_start(vargs, num_args);
    
    //Put Data to be sent into the master data register
    I2CMasterDataPut(ui32Base, va_arg(vargs, uint32_t)); 
    
    if(num_args == 1){
        //Initiate sending data from the MCU
        I2CMasterControl(ui32Base, I2C_MASTER_CMD_SINGLE_SEND);
        
        //Wait until MCU finished transferring data
        while(I2CMasterBusy(ui32Base));
        
        //Close va list
        va_end(vargs);
    }
    
    //to write to multiple registers on the slave device
    else{
        //initialize data transmission from MCU
        I2CMasterControl(ui32Base, I2C_MASTER_CMD_BURST_SEND_START);
        
        //wait until MCU finished transferring data
        while(I2CMasterBusy(ui32Base));
        
        for(uint8_t i = 1; i < (num_args - 1); i++){
            //load data into MCU register
            I2CMasterDataPut(ui32Base, va_args(vargs, uint8_t));
            
            //Send data that was just loaded into register
            I2CMasterControL(ui32Base, I2C_MASTER_CMD_BURST_SEND_CONT);
            
            //Wait until MCU is done transferring
            while(I2CMasterBusy(ui32Base));
        }
        //load last piece of data into I2C FIFO
        I2CMasterDataPut(ui32Base, va_args(vargs, uint8_t)); 
        
        //send last piece of data from MCU
        I2CMasterControl(ui32Base, I2C_MASTER_CMD_BURST_SEND_FINISH);
        
        //wait until MCU is done transferring
        while(I2CMasterBusy(ui32Base)); 
        
        //close va list
        va_end(vargs);
    }
}

//read from specified register on the slave device
uint32_t read(uint8_t ui8SlaveAddr, uint8_t ui8reg, uint32_t ui32Base, bool bReceive){
    //Write register address to slave 
    I2CMasterSlaveAddrSet(ui32Base, ui8SlaveAddr, false);
    
    //Specify register we want to read from
    I2CMasterDataPut(ui32Base, ui8reg); 
    
    //Send control byte and register address byte to slave 
    I2CMasterControl(ui32Base, I2C_MASTER_CMD_BURST_SEND_START);
    
    //Wait until MCU done transferring
    while(I2CMasterBusy(ui32base)); 
    
    //Tell slave we will be reading data from it
    I2CMasterSlaveAddrSet(ui32Base, ui8SlaveAddr, bReceive); 
    
    I2CMasterControl(ui32Base, I2C_MASTER_CMD_SINGLE_RECEIVE); 
    
    //Wait until MCU is done transferring 
    while(I2CMasterBusy(ui32Base)); 
    
    //return the data obtained from the register
    return I2CMasterDataGet(ui32Base); 
}



