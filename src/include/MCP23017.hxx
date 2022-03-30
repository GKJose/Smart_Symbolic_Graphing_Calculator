#ifndef _MCP23017_H
#define _MCP23017_H

#include <calc_conf.h>
#if ENABLE_MCP_KEYPAD
#include <cstddef>
#include <cstdint>

extern "C" {
#include <i2c/smbus.h>
}

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>



class MCP23017
{
public:
    unsigned char kI2CBus ;         // I2C bus of the MCP23017
    int kI2CFileDescriptor ;        // File Descriptor to the MCP23017
    int kI2CAddress ;               // Address of MCP23017; defaults to 0x20
    int error ;
    MCP23017(int bus=1, int address=0x20);
    ~MCP23017() ;
    bool openI2C() ;
    void closeI2C();


    void pinMode(uint8_t p, uint8_t d);
    void digitalWrite(uint8_t p, uint8_t d);
    void pullUp(uint8_t p, uint8_t d);
    bool digitalRead(uint8_t p);

    void writeGPIOAB(uint16_t);
    uint16_t readGPIOAB();
    uint8_t readGPIO(uint8_t b);

    void setupInterrupts(uint8_t mirroring, uint8_t open, uint8_t polarity);
    void setupInterruptPin(uint8_t p, uint8_t mode);
    uint8_t getLastInterruptPin();
    uint8_t getLastInterruptPinValue();

private:
    uint8_t i2caddr;

    uint8_t bitForPin(uint8_t pin);
    uint8_t regForPin(uint8_t pin, uint8_t portAaddr, uint8_t portBaddr);

    uint8_t readRegister(uint8_t addr);
    uint8_t writeRegister(uint8_t addr, uint8_t value);
    uint8_t readByte();
    uint8_t writeByte(uint8_t value);

  /**
   * Utility private method to update a register associated with a pin (whether port A/B)
   * reads its value, updates the particular bit, and writes its value.
   */
    void updateRegisterBit(uint8_t p, uint8_t pValue, uint8_t portAaddr, uint8_t portBaddr);

    bool bitRead(uint8_t num, uint8_t index);
    void bitWrite(uint8_t &var, uint8_t index, uint8_t bit);


};

#define MCP23017_ADDRESS 0x20

// registers
#define MCP23017_IODIRA 0x00
#define MCP23017_IPOLA 0x02
#define MCP23017_GPINTENA 0x04
#define MCP23017_DEFVALA 0x06
#define MCP23017_INTCONA 0x08
#define MCP23017_IOCONA 0x0A
#define MCP23017_GPPUA 0x0C
#define MCP23017_INTFA 0x0E
#define MCP23017_INTCAPA 0x10
#define MCP23017_GPIOA 0x12
#define MCP23017_OLATA 0x14


#define MCP23017_IODIRB 0x01
#define MCP23017_IPOLB 0x03
#define MCP23017_GPINTENB 0x05
#define MCP23017_DEFVALB 0x07
#define MCP23017_INTCONB 0x09
#define MCP23017_IOCONB 0x0B
#define MCP23017_GPPUB 0x0D
#define MCP23017_INTFB 0x0F
#define MCP23017_INTCAPB 0x11
#define MCP23017_GPIOB 0x13
#define MCP23017_OLATB 0x15

#define MCP23017_INT_ERR 255

//constants set up to emulate Arduino pin parameters
#define HIGH 1
#define LOW 0

#define INPUT 1
#define OUTPUT 0

#define CHANGE 0
#define FALLING 1
#define RISING 2

#endif
#endif