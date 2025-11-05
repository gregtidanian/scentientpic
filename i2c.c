#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define FCY 16000000UL        // Adjust if using different oscillator
#include <xc.h>
#include <libpic30.h>
#define EEPROM_ADDR  0x51     // 7-bit device address (A2..A0 = 100)


void I2C1_Init(void);
void I2C1_Start(void);
void I2C1_Restart(void);
void I2C1_Stop(void);
void I2C1_Write(uint8_t data);
uint8_t I2C1_Read(uint8_t ack);
void EEPROM_WriteByte(uint8_t memAddr, uint8_t data);
uint8_t EEPROM_ReadByte(uint8_t memAddr);
void EEPROM_WaitForWrite(void);

void I2C1_Init(void)
{
    // Ensure TRIS configured as inputs (I2C module will drive them)
    TRISGbits.TRISG2 = 1;  // SCL1
    TRISGbits.TRISG3 = 1;  // SDA1

    I2C1CONL = 0;
    //I2C1BRG  = 0x12; //((FCY / 100000) / 2) - 2;  // 100 kHz operation
    I2C1BRG = 0x4E;
    I2C1CONLbits.I2CEN = 1;               // Enable I2C module
    __delay_ms(10);
}

void I2C1_Start(void)
{
    I2C1CONLbits.SEN = 1;
    while (I2C1CONLbits.SEN);
}

void I2C1_Restart(void)
{
    I2C1CONLbits.RSEN = 1;
    while (I2C1CONLbits.RSEN);
}

void I2C1_Stop(void)
{
    I2C1CONLbits.PEN = 1;
    while (I2C1CONLbits.PEN);
}

void I2C1_Write(uint8_t data)
{
    I2C1TRN = data;
    while (I2C1STATbits.TBF);
    while (I2C1STATbits.TRSTAT);
    while (I2C1STATbits.ACKSTAT);  // Wait for ACK
}

uint8_t I2C1_Read(uint8_t ack)
{
    uint8_t data;
    I2C1CONLbits.RCEN = 1;
    while (!I2C1STATbits.RBF);
    data = I2C1RCV;
    I2C1CONLbits.ACKDT = ack;      // 0=ACK, 1=NACK
    I2C1CONLbits.ACKEN = 1;
    while (I2C1CONLbits.ACKEN);
    return data;
}

void EEPROM_WaitForWrite(void)
{
    // Poll device until it ACKs (write complete)
    do {
        I2C1_Start();
        I2C1_Write((EEPROM_ADDR << 1) | 0); // Write control byte
        I2C1_Stop();
    } while (I2C1STATbits.ACKSTAT);
}

void EEPROM_WriteByte(uint8_t memAddr, uint8_t data)
{
    I2C1_Start();
    I2C1_Write((EEPROM_ADDR << 1) | 0); // Write mode
    I2C1_Write(memAddr);
    I2C1_Write(data);
    I2C1_Stop();

    //EEPROM_WaitForWrite(); // Wait for internal write cycle
    __delay_ms(10);
}

uint8_t EEPROM_ReadByte(uint8_t memAddr)
{
    uint8_t data;

    I2C1_Start();
    I2C1_Write((EEPROM_ADDR << 1) | 0); // Write mode
    I2C1_Write(memAddr);
    I2C1_Restart();
    I2C1_Write((EEPROM_ADDR << 1) | 1); // Read mode
    data = I2C1_Read(1);                // NACK after last byte
    I2C1_Stop();

    return data;
}