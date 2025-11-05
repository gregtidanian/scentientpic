#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>

#ifndef FCY
#define FCY 16000000UL
#endif

// Device address example for M24C02 when A2..A0 = 0b001 -> 0x51
#ifndef EEPROM_ADDR
#define EEPROM_ADDR  0x51
#endif

void I2C1_Init(void);
void I2C1_Start(void);
void I2C1_Restart(void);
void I2C1_Stop(void);
// Returns true if slave ACKed, false if NACK
bool I2C1_Write(uint8_t data);
// ack=true sends ACK, ack=false sends NACK
uint8_t I2C1_Read(bool ack);

// Optional helpers for 24xx02
void EEPROM_WaitForWrite(void);
void EEPROM_WriteByte(uint8_t memAddr, uint8_t data);
uint8_t EEPROM_ReadByte(uint8_t memAddr);

#endif