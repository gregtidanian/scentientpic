/*
 * File:   bluetooth.c
 * Author: ALN
 * Description: Bluetooth communications for Scentient Collar
 * Comments: Copyright 2025 PD Studio 29 LTD
 * Created on 13 January 2025, 10:00
 */

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "xc.h"
#include "main.h"
#include "bluetooth.h"
#include "owire.h"
#include "piezos.h"

#define TIMEOUT 10000
#define SCHAR 0xFA
#define ECHAR 0xFF
#define CMD 1

//#define UART1RXPIN 21
//#define UART1TXPIN 26

#define UART1RXPIN 26
#define UART1TXPIN 21

uint8_t error_counter = 0;

typedef struct _MESSAGE_BUFFER {
    uint8_t pod;
    uint8_t intensity;
    uint16_t duration;
} message_buffer;

void setupBT(void)
{
    // Setup BT Pins
    TRISDbits.TRISD4 = 0;
    TRISDbits.TRISD5 = 1;
    
    ANSELDbits.ANSELD6 = 0;
    TRISDbits.TRISD6 = 0;
    
    // Directions must match: RG6 = TX (output), RG7 = RX (input)
    TRISGbits.TRISG6 = 0;   // RG6 output for U1TX
    TRISGbits.TRISG7 = 1;   // RG7 input  for U1RX

    // ADDED: Unlock PPS to allow RPINR/RPOR writes (clear IOLOCK)
    // Datasheet: OSCCON.IOLOCK must be cleared via unlock sequence
    __builtin_write_OSCCONL(OSCCON & 0xBF);

    // CHANGED: U1RXR old RP20 -> new RP21 (RG6)
    RPINR18bits.U1RXR = UART1RXPIN; // U1RX <- RP26 (RG7)
    // CHANGED: U1TX old RP25R -> new RP26R (RG7)
    //RPOR13bits.RP26R = 3;   // RG7 as UART1 TX
    RPOR10bits.RP21R = 3;

    // ADDED: Lock PPS after mapping (set IOLOCK)
    __builtin_write_OSCCONL(OSCCON | 0x40);

    
    //RPINR18bits.U1RXR = 25; // RP20 as UART1 RX
    //RPOR10bits.RP20R = 3; // RP25 as UART1 TX
    
    U1MODEbits.URXINV = 0;
    U1MODEbits.BRGH = 1; // high accuracy mode
    U1BRG = 34; // baud rate - 115200 bps
    U1MODEbits.UARTEN = 1;
    
    // ADDED: Ensure TX enabled (some silicon requires UTXEN set explicitly)
    U1STAbits.UTXEN = 1;
    
    // Setup BT Interrupt
    IFS0bits.U1RXIF = 0;
    // ADDED: Set nonzero RX interrupt priority (required for ISR to trigger)
    IPC2bits.U1RXIP = 5;
    IEC0bits.U1RXIE = 1;
}

void enableSerial(bool enable)
{
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = U1MODEbits.UARTEN = enable;
}

void BTTX(uint8_t *cmd, int length)
{
    int index = 0;
    U1STAbits.UTXEN = 1;
    for (; index < length; index++)
    {
        U1TXREG = cmd[index];
        while(U1STAbits.TRMT == 0);
    }
}

void sendLEName(uint8_t *name)
{
    char buffer[51] = {0};
    int length;
    if (name[0] == 0)
    {
        return;
    }
    length = sprintf(buffer, "AT+LENAME=%s\r\n", name);
    BTTX((uint8_t *)buffer, length);
}

void sendName(uint8_t *name)
{
    char buffer[51] = {0};
    int length;
    if (name[0] == 0)
    {
        return;
    }
    length = sprintf(buffer, "AT+NAME=%s\r\n", name);
    BTTX((uint8_t *)buffer, length);
}

void enableSSP(uint8_t en)
{
    char buffer[51] = {0};
    int length;
    length = sprintf(buffer, "AT+PIOCFG=%d,%d\r\n", en, en);
    BTTX((uint8_t *)buffer, length);
}

/*********Usage*********/
// enables and disables profiles based on bit value 
// Argument profiles - 11 bit number each bit representing a profile 
// Bit breakdown:
// BIT[0] - SPP (Serial Port Profile)
// BIT[1] - GATT Server (Generic Attribute Profile)
// BIT[2] - GATT Client (Generic Attribute Profile)
// BIT[3] - HFP Sink (Hands-Free Profile)
// BIT[4] - HFP Source (Hands-Free Profile)
// BIT[5] - A2DP Sink (Advanced Audio Distribution Profile)
// BIT[6] - A2DP Source (Advanced Audio Distribution Profile)
// BIT[7] - AVRCP Controller (Audio/Video Remote Control Profile)
// BIT[8] - AVRCP Target (Audio/Video Remote Control Profile)
// BIT[9] - HID Keyboard (Human Interface Device Profile)
// BIT[10] - PBAP Server (Phonebook Access Profile)
// Example - profiles = 97 - enables SPP, A2DP sink and source
void profileSelect(uint16_t profiles)
{
    if (profiles > 1)    // Value greater than "All enabled"?
        return;             // Yes - Bomb out!
    
    char buffer[51] = {0};
    int length;
    length = sprintf(buffer, "AT+ROLE=%d\r\n", profiles);
    BTTX((uint8_t *)buffer, length);
}

uint8_t testVar = 0;
void sendTest(void)
{
    U1TXREG = '[';
    while(U1STAbits.TRMT == 0);
    U1TXREG = 'B';
    while(U1STAbits.TRMT == 0);
    U1TXREG = 'T';
    while(U1STAbits.TRMT == 0);
    U1TXREG = 'N';
    while(U1STAbits.TRMT == 0);
    U1TXREG = ',';
    while(U1STAbits.TRMT == 0);
    U1TXREG = testVar = !testVar;
    while(U1STAbits.TRMT == 0);
    U1TXREG = ']';
    while(U1STAbits.TRMT == 0);
    U1TXREG = '\n';
    while(U1STAbits.TRMT == 0);
}

void sendData(uint8_t *data, int dlen)
{
    char buffer[51] = {0};
    int length = 0;
    if (data[0] == 0)
    {
        return;
    }
    //length += sprintf(&buffer[length], "AT+LESEND=%d,", dlen);
    for (int i = 0; i < dlen; i++)
        buffer[length++] = data[i];
    //length += sprintf(&buffer[length], "\r\n");
    BTTX((uint8_t *)buffer, length);
}

#define BTNSTR  ("[" BTNALIAS ",%d]\n")
void sendButtonState(uint8_t state)
{
    char buffer[251] = {0};
    int length = 0;
    length += sprintf(&buffer[length], BTNSTR, state);
    BTTX((uint8_t *)buffer, length);    
}

#define BATTSTR  ("[" BATTALIAS ",%d]\n")
void sendBattLevel(uint16_t level)
{
    char buffer[251] = {0};
    int length = 0;
    length += sprintf(&buffer[length], BATTSTR, level);
    BTTX((uint8_t *)buffer, length);   
}

void sendPod(uint8_t pod, uint16_t value)
{
    char buffer[251] = {0};
    int length = 0;
    length += sprintf(&buffer[length], "[%s,%d,%u]\n", PODALIAS, pod, value);
    BTTX((uint8_t *)buffer, length);   
}

void sendScentLevel(uint8_t num, uint16_t level)
{
    char buffer[251] = {0};
    int length = 0;
    length += sprintf(&buffer[length], "[%s,%d,%u]\n", SCENTALIAS, num, level);
    BTTX((uint8_t *)buffer, length);  
}

void sendScentIDs(uint8_t num, uint16_t id)
{
    char buffer[251] = {0};
    int length = 0;
    length += sprintf(&buffer[length], "[%s,%d,%u]\n", IDALIAS, num, id);
    BTTX((uint8_t *)buffer, length);  
}

#define MSGSTR  ("[" MSGALIAS ",%lu]\n")
void sendScentMsg(uint32_t message)
{
    char buffer[251] = {0};
    int length = 0;
    length += sprintf(&buffer[length], MSGSTR, message);
    BTTX((uint8_t *)buffer, length);  
}

void sendReboot(void)
{
    char buffer[51] = "AT+REBOOT\r\n";
    BTTX((uint8_t *)buffer, 11);
}

void sendDSCA(void)
{
    char buffer[51] = "AT+DSCA\r\n";
    BTTX((uint8_t *)buffer, 9);
}

void sendA2dpDisc(void)
{
    char buffer[51] = "AT+A2DPDISC\r\n";
    BTTX((uint8_t *)buffer, 13);
}

void sendRestore(void)
{
    char buffer[51] = "AT+RESTORE\r\n";
    BTTX((uint8_t *)buffer, 12);
}

void sendSPPData(uint8_t *data, int dlen)
{
    char buffer[51] = {0};
    int length = 0;
    if (data[0] == 0)
    {
        return;
    }
    length += sprintf(&buffer[length], "AT+SPPSEND=%d,", dlen);
    for (int i = 0; i < dlen; i++)
        buffer[length++] = data[i];
    length += sprintf(&buffer[length], "\r\n");
    BTTX((uint8_t *)buffer, length);
}

void sendPodIDsOLD(void)
{
    uint8_t txbuffer[401] = {0};
    int tx_index = 0;
    txbuffer[tx_index++] = 0xFA;
    txbuffer[tx_index++] = 0x16;
    for (int x = 0; x < 6; x++)
    {
        txbuffer[tx_index++] = (pod_store.pods[x].pod_id & 0xFF00) >> 8;
        txbuffer[tx_index++] = (pod_store.pods[x].pod_id & 0x00FF);
    }
    txbuffer[tx_index++] = 0xFF;
    sendData(txbuffer, tx_index);
}

void sendScentIDsOLD(void)
{
    uint8_t txbuffer[401] = {0};
    int tx_index = 0;
    txbuffer[tx_index++] = 0xFA;
    txbuffer[tx_index++] = 0x17;
    for (int x = 0; x < 6; x++)
    {
        txbuffer[tx_index++] = (pod_store.pods[x].scent_id & 0xFF00) >> 8;
        txbuffer[tx_index++] = (pod_store.pods[x].scent_id & 0x00FF);
    }
    txbuffer[tx_index++] = 0xFF;
    sendData(txbuffer, tx_index);
}

void sendScentLevelsOLD(void)
{
    uint8_t txbuffer[401] = {0};
    int tx_index = 0;
    txbuffer[tx_index++] = 0xFA;
    txbuffer[tx_index++] = 0x18;
    for (int x = 0; x < 6; x++)
        txbuffer[tx_index++] = pod_store.pods[x].scent_level;
    txbuffer[tx_index++] = 0xFF;
    sendData(txbuffer, tx_index);
}

void sendPowerStatsOLD(uint16_t charge_mV, uint16_t trans_mV, uint16_t battery_mV)
{
    uint8_t txbuffer[401] = {0};
    int tx_index = 0;
    txbuffer[tx_index++] = 0xFA;
    txbuffer[tx_index++] = 0x22;
    txbuffer[tx_index++] = (charge_mV & 0xFF00) >> 8;
    txbuffer[tx_index++] = (charge_mV & 0x00FF);
    txbuffer[tx_index++] = (trans_mV & 0xFF00) >> 8;
    txbuffer[tx_index++] = (trans_mV & 0x00FF);
    txbuffer[tx_index++] = (battery_mV & 0xFF00) >> 8;
    txbuffer[tx_index++] = (battery_mV & 0x00FF);
    txbuffer[tx_index++] = 0xFF;
    sendData(txbuffer, tx_index);    
}

void dump_OK(void)
{
    uint8_t c[10] = {0};
    uint32_t timeout = 0;
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 0;
    for (int i = 0; i < 6; i++)
    {
        while (!U1STAbits.URXDA)
        {
            timeout++;
            if (timeout == TIMEOUT)
                return;
        }
        timeout = 0;
        c[i] = U1RXREG;
    }
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 1;
}

#ifndef USE_CHARS
void BTRX(void)
{
    uint8_t rxbuffer[401] = {0};
    uint8_t txbuffer[401] = {0};
    uint8_t c = 0, success = 0;
    uint8_t i;
    uint16_t d, p_id, s_id;
    uint32_t timeout = 0;
    int index = 0, tx_index = 0;
    PodData search;
    
    do
    {
        uint8_t breakout = 0;
        while(U1STAbits.URXDA == 0)/*;*/
        {
            timeout++;
            if (timeout == TIMEOUT)
            {
                breakout = 1;
                break;
            }
        }
        
        if (breakout)
            break;
        
        rxbuffer[index++] = U1RXREG;
        timeout = 0;
    }
    while (1);
    
    Nop();
    while (U1STAbits.URXDA == 1) // look for any trailing garbage
    {
        c = U1RXREG;
        U1STAbits.OERR = 0; // Clear error flag
    }
    
    if (strstr((char *)rxbuffer, "ERR"))
    {
        Nop();
        Nop();
        Nop();
        Nop();
        return;
    }
    
    if (strstr((char *)rxbuffer, "OK"))
    {
        Nop();
        Nop();
        Nop();
        Nop();
        return;
    }
        
    switch (rxbuffer[CMD])
    {
        case 0x15:
            txbuffer[tx_index++] = 0xFA;
            txbuffer[tx_index++] = 0x15;
            
            i = rxbuffer[CMD+2];
            d = (rxbuffer[CMD+3] << 8) + rxbuffer[CMD+4];
            
            if (i <= 100 && d > 1000 && d <= 15000)
            {
                switch (rxbuffer[CMD+1])
                {
                    default:
                    case 1:
                        success = fire_piezo_R1(i, d);
                        break;
                    case 2:
                        success = fire_piezo_R2(i, d);
                        break;
                    case 3:
                        success = fire_piezo_R3(i, d);
                        break;
                    case 4:
                        success = fire_piezo_L1(i, d);
                        break;
                    case 5:
                        success = fire_piezo_L2(i, d);
                        break;
                    case 6:
                        success = fire_piezo_L3(i, d);
                        break;
                }
            }
            else
                success = 3;
            txbuffer[tx_index++] = rxbuffer[CMD+1];
            txbuffer[tx_index++] = rxbuffer[CMD+2];
            txbuffer[tx_index++] = rxbuffer[CMD+3];
            txbuffer[tx_index++] = rxbuffer[CMD+4];
            txbuffer[tx_index++] = success;
            txbuffer[tx_index++] = 0xFF;
            sendData(txbuffer, tx_index);
            break;
        case 0x19:  // set pod id
            p_id = rxbuffer[CMD+1];
            s_id = (rxbuffer[CMD+2] << 8) + rxbuffer[CMD+3];
            if (p_id < 1 || p_id > 6)
                success = 0;
            else
            {
                search = find_by_channel_num(p_id);
                success = set_pod_id(search, s_id);
            }
            
            txbuffer[tx_index++] = 0xFA;
            txbuffer[tx_index++] = 0x19;
            txbuffer[tx_index++] = rxbuffer[CMD+1];
            txbuffer[tx_index++] = rxbuffer[CMD+2];
            txbuffer[tx_index++] = rxbuffer[CMD+3];
            txbuffer[tx_index++] = !success;
            txbuffer[tx_index++] = 0xFF;
            sendData(txbuffer, tx_index);
            break;
        case 0x20:  // set scent id
            p_id = rxbuffer[CMD+1];
            s_id = (rxbuffer[CMD+2] << 8) + rxbuffer[CMD+3];
            if (p_id < 1 || p_id > 6)
                success = 0;
            else
            {
                search = find_by_channel_num(p_id);
                if (search.pod_id == 0x0000)
                    success = 0;
                else
                    success = set_scent_id(search, s_id);
            }
            
            txbuffer[tx_index++] = 0xFA;
            txbuffer[tx_index++] = 0x20;
            txbuffer[tx_index++] = rxbuffer[CMD+1];
            txbuffer[tx_index++] = rxbuffer[CMD+2];
            txbuffer[tx_index++] = rxbuffer[CMD+3];
            txbuffer[tx_index++] = !success;
            txbuffer[tx_index++] = 0xFF;
            sendData(txbuffer, tx_index);
            break;
        default:
            break;
    }
}
#else
void BTRX(void)
{
    uint8_t rxbuffer[401] = {0};
    uint8_t gattdata[301] = {0};
    uint8_t c = 0;
    uint32_t timeout = 0;
    int index = 0;
    PodData selected_pod;
    memset(&selected_pod, 0, sizeof(PodData));
    
    do
    {
        uint8_t breakout = 0;
        while(U1STAbits.URXDA == 0)/*;*/
        {
            timeout++;
            if (timeout == TIMEOUT)
            {
                breakout = 1;
                break;
            }
        }
        
        if (breakout)
            break;
        
        rxbuffer[index++] = U1RXREG;
        timeout = 0;
    }
    while (1);
    
    Nop();
    while (U1STAbits.URXDA == 1) // look for any trailing garbage
    {
        c = U1RXREG;
        U1STAbits.OERR = 0; // Clear error flag
    }
    
    if (rxbuffer[0] == 0)
        return;
    
    memcpy(gattdata, &rxbuffer[1], (index-1));
    if (!memcmp(gattdata, MSGALIAS, 4))
    {
        int startIndex = 5;
        uint32_t value = strtoul((char *)&gattdata[startIndex], NULL, 10);
        uint8_t flipped[4] = {0};
        flipped[0] = ((value & 0xFF000000) >> 24);
        flipped[1] = ((value & 0x00FF0000) >> 16);
        flipped[2] = ((value & 0x0000FF00) >> 8);
        flipped[3] = (value & 0x000000FF);

        PiezoFIFOItem buffer;
        //message_buffer buffer = {0};
        memcpy(&buffer, (PiezoFIFOItem *)&flipped, sizeof(PiezoFIFOItem));
        uint8_t x = buffer.intensity;
        buffer.intensity = x/2 - x/8 + x/64;
        if (buffer.channel <= 3)
            piezo_fifo_right_put(&buffer);
        else
            piezo_fifo_left_put(&buffer);
        piezo_interrupt = 1;

        /*switch (buffer.pod)
        {
            default:
                return;
            case 1:
                fire_piezo_R1(buffer.intensity, buffer.duration);
                break;
            case 2:
                fire_piezo_R2(buffer.intensity, buffer.duration);
                break;
            case 3:
                fire_piezo_R3(buffer.intensity, buffer.duration);
                break;
            case 4:
                fire_piezo_L1(buffer.intensity, buffer.duration);
                break;
            case 5:
                fire_piezo_L2(buffer.intensity, buffer.duration);
                break;
            case 6:
                fire_piezo_L3(buffer.intensity, buffer.duration);
                break;
        }*/
    }
    else if (!memcmp(gattdata, PODALIAS, 3))
    {
        int startIndex = 4;
        uint8_t pod = gattdata[startIndex] - '0';
        if (pod > 5)
            return;
        startIndex += 2;
        uint16_t id = (uint16_t)strtoul((char *)&gattdata[startIndex], NULL, 10);
        selected_pod = find_by_channel_num(pod);
        if (selected_pod.pod_id != 0)
            set_pod_id(selected_pod, id);
    }
    else if (!memcmp(gattdata, IDALIAS, 3))
    {
        int startIndex = 4;
        uint8_t pod = gattdata[startIndex] - '0';
        if (pod > 5)
            return;
        startIndex += 2;
        uint16_t id = (uint16_t)strtoul((char *)&gattdata[startIndex], NULL, 10);
        if (id > DIESEL)
            return;
        selected_pod = find_by_channel_num(pod);
        if (selected_pod.pod_id != 0)
            set_scent_id(selected_pod, id);
    }
    else if (!memcmp(gattdata, CLRALIAS, 4))
    {
        if ('V' == gattdata[5])
        {
            uint8_t val[101] = {0}, length;
            length = sprintf((char *)val, "[%s,%s,v%1.2f,%02d/%02d/%04d]\n", CLRALIAS,
                    VER_DESCRIPTION, VER_NUM, VER_BUILD_DAY, VER_BUILD_MON,
                    VER_BUILD_YEAR);
            BTTX(val, length);
        }
        // TODO add more later...
    }
}
#endif

void _ISR _U1RXInterrupt(void)
{
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 0;
    
    BTRX();
    Nop();
    
    //U1MODEbits.UARTEN = 0;
    //U1MODEbits.UARTEN = 1;
    //U2MODEbits.UARTEN = 0;
    //U2MODEbits.UARTEN = 1;
    
    IEC0bits.U1RXIE = 1;
    return;
}