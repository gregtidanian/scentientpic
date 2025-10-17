/*
 * File:   owire.c
 * Author: ALN
 * Description: 1-Wire EEprom source file for Scentient Collar Firmware
 * Comments: Copyright 2025 PD Studio 29 LTD
 * Created on 13 January 2025, 10:00
 */
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "owire.h"
#include "bluetooth.h"

// Global search state
uint8_t rom_no[8];
int last_discrepancy;
int last_fam_discrepancy;
int last_dev_flag;
uint8_t crc8;
static int last_pods_present = 0;
static bool last_all_pods = false;

PodStore pod_store;

static uint8_t dscrc_table[] = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

LocateMode mode = LOCATE_IDLE;

uint8_t docrc8(uint8_t value)
{
   // See Application Note 27
   
   // TEST BUILD
   crc8 = dscrc_table[crc8 ^ value];
   return crc8;
}

char ow_reset(void)
{
    TOWOUT;
    OWOUT;
    __delay_us(480);
    
    OWIN;
    __delay_us(70);
    
    TOWIN;
    char ret = (OWPINI == 0);
    __delay_us(410);
    
    return ret;
}

void ow_write_bit(char bit)
{
    TOWOUT;
    OWOUT;
    if (bit)
    {
        __delay_us(6);
        OWIN;
        __delay_us(64);
    }
    else
    {
        __delay_us(60);
        OWIN;
        __delay_us(10);
    }
}

char ow_read_bit(void)
{
    TOWOUT;
    OWOUT;
    __delay_us(6);
    
    OWIN;
    TOWIN;
    __delay_us(9);
    
    char bit = OWPINI;
    __delay_us(55);
    return bit;
}

char ow_read_byte(void)
{
    char c, r = 0;
    for (c = 0; c < 8; c++)
    {
        if (ow_read_bit())
            r |= 1 << c;
    }
    return r;
}

void ow_write_byte(char B)
{
    char c;
    for (c = 0; c < 8; c++)
        ow_write_bit((B >> c) & 1);
}

int ow_search_rom(uint8_t *sink)
{
/*     int bit_num, last_zero, rom_byte_num;
    int id_bit, cmp_id_bit;
    uint8_t rom_byte_mask, search_direction;
    int search_result;
    
    // Initialise search
    bit_num = 1;
    last_zero = 0;
    rom_byte_num = 0;
    rom_byte_mask = 1;
    search_result = 0;
    crc8 = 0;
    
    if (!last_dev_flag)
    {
        ow_reset();
        ow_write_byte(0xF0); // Search ROM command
        
        // Loop search
        do
        {
            id_bit = ow_read_bit();
            cmp_id_bit = ow_read_bit();
            if (id_bit && cmp_id_bit)   // Only one device?
                break;                  // Yes - Break out
            else                        // No - More than 1!
            {
                // All devices coupled have 0 or 1
                if (id_bit != cmp_id_bit)
                    search_direction = id_bit;
                else
                {
                    // If this discrepancy is before the last_discrepancy
                    // on previous next then pick the same as last time
                    if (bit_num < last_discrepancy)
                        search_direction = ((rom_no[rom_byte_num] & rom_byte_mask) > 0);
                    else
                        // If equal to last pick 1, if not then pick 0
                        search_direction = (bit_num == last_discrepancy);
                    
                    // if 0 was picked then record its position in last_zero
                    if (!search_direction)
                    {
                        last_zero = bit_num;
                        
                        // Check for last_fam_discrepancy
                        if (last_zero < 9)
                            last_fam_discrepancy = last_zero;
                    }
                }
                
                // Set or clear the bit in the ROM byte rom_byte_num
                // with mask rom_byte_mask
                if (search_direction)
                    rom_no[rom_byte_num] |= rom_byte_mask;
                else
                    rom_no[rom_byte_num] &= ~rom_byte_mask;
                
                // S/N search direction write
                ow_write_bit(search_direction);
                
                // increment the byte counter bit_num
                // and shift the mask rom_byte_mask
                bit_num++;
                rom_byte_mask <<= 1;
                
                // if mask is 0 then go to new S/N byte rom_byte_num and reset mask
                if (!rom_byte_mask)
                {
                    docrc8(rom_no[rom_byte_num]); // accumulate the CRC
                    rom_byte_num++;
                    rom_byte_mask = 1;
                }
            }
        }
        while (rom_byte_num < 8); // Loop until all 8 bytes are gathered
        
        // If search successful
        if (!((bit_num < 65) || (crc8 != 0)))
        {
            // search successful set last_discrepancy, last_dev_flag, search_result
            last_discrepancy = last_zero;
            
            // check for last device
            if (!last_discrepancy)
                last_dev_flag = 1;
            
            search_result = 1;
        }
    }
    
    // Reset search, if failed
    if (!search_result || !rom_no[0])
    {
        last_discrepancy = 0;
        last_dev_flag = 0;
        last_fam_discrepancy = 0;
        search_result = 0;
    }
    
    memcpy(sink, rom_no, 8);
    return search_result; */

    return 0;
}

uint8_t *rom_code_to_bits(uint8_t *rom_code)
{
    static uint8_t bits[64] = {0};
    int i = 0;
    
    for (; i < (8*8); i++)
        bits[i] = ((1 << (i % 8)) & (rom_code[i/8])) >> (i % 8);
    return bits;
}

uint8_t *ow_read_mem(uint8_t *rom_code)
{
    static uint8_t rx[51] = {0};
    int x = 0, y = 0;
    
    uint8_t *bits = rom_code_to_bits(rom_code);
    ow_reset();
    ow_write_byte(0x55);    // Match ROM command
    for (; y < 64; y++)
        ow_write_bit(bits[y]);
    //for (y=0; y<8; y++)
    //    ow_write_byte(rom_code[y]);
    ow_write_byte(0xF0);    // Read Memory command
    ow_write_byte(0x00);
    ow_write_byte(0x00);
    
    for (; x < 50; x++)
        rx[x] = ow_read_byte();
    //ow_reset();
    
    return rx;
}

uint8_t *ow_write_scratchpad(uint8_t *rom_code, uint16_t address, uint8_t *to_write, uint8_t length)
{
    static uint8_t buffer[32] = {0};
    uint8_t *bits = rom_code_to_bits(rom_code);
    uint8_t c1 = 0, c2 = 0;
    int i = 0, y = 0;
    if (length > 32)
        return NULL;
    
    memcpy(buffer, to_write, length);
    
    ow_reset();
    ow_write_byte(0x55);    // Match ROM command
    for (; y < 64; y++)
        ow_write_bit(bits[y]);
    ow_write_byte(0x0F);    // Write Scratchpad command
    ow_write_byte((uint8_t)((address & 0xFF00) >> 8));
    ow_write_byte((uint8_t)(address & 0x00FF));
    
    for (; i < 32; i++)
        ow_write_byte(buffer[i]);
    
    c1 = ow_read_byte();
    c2 = ow_read_byte();
    //ow_reset();
    
    return buffer;
}

uint8_t *ow_read_scratchpad(uint8_t *rom_code, uint16_t *address, uint8_t *es, uint16_t *crc)
{
    static uint8_t buffer[32] = {0};
    uint8_t *bits = rom_code_to_bits(rom_code);
    uint8_t a1 = 0, a2 = 0, e = 0, c1 = 0, c2 = 0;
    int i = 0, y = 0;
    
    ow_reset();
    ow_write_byte(0x55);    // Match ROM command
    for (; y < 64; y++)
        ow_write_bit(bits[y]);
    ow_write_byte(0xAA);    // Read Scratchpad command
    
    a1 = ow_read_byte();
    a2 = ow_read_byte();
    e = ow_read_byte();
    
    *address = ((a1 << 8) + a2);
    *es = e;
    
    for (; i < 32; i++)
        buffer[i] = ow_read_byte();
    
    c1 = ow_read_byte();
    c2 = ow_read_byte();
    *crc = ((c1 << 8) + c2);
    //ow_reset();
    
    return buffer;
}

uint8_t ow_copy_scratchpad(uint8_t *rom_code, uint16_t address, uint8_t es)
{
    uint8_t *bits = rom_code_to_bits(rom_code);
    int y = 0;
    
    ow_reset();
    ow_write_byte(0x55);    // Match ROM command
    for (; y < 64; y++)
        ow_write_bit(bits[y]);
    ow_write_byte(0x55);    // Copy Scratchpad command
    ow_write_byte((uint8_t)(address & 0xFF00) >> 8);
    ow_write_byte((uint8_t)(address & 0x00FF));
    ow_write_byte(es);
    return 1;
}

uint8_t write_save_eeprom(uint8_t *rom_code, uint16_t address, uint8_t *data, uint8_t length) 
{
    uint16_t read_addr, crc;
    uint8_t read_es;
    uint8_t *w_data, *r_data;
    
    /*do
    {*/
        w_data = ow_write_scratchpad(rom_code, address, data, length);
        __delay_ms(100);
        r_data = ow_read_scratchpad(rom_code, &read_addr, &read_es, &crc);
    /*}
    while (memcmp(r_data, w_data, 32));*/
    
    if (read_addr == address && !memcmp(w_data, r_data, 32))
    {
        uint8_t ret = ow_copy_scratchpad(rom_code, read_addr, read_es);
        __delay_ms(100);
        return ret;
    }
    return 0;
}

uint16_t dummy_ids[6] = { 12, 13, 22, 4, 6, 9 };
void read_eeprom(int ignore_pos)
{
    int x;
    for (x = 0; x < MAX_PODS /*pod_store.pods_present*/; x++)
    {
        //memset(&pod_store.pods[x], 0, sizeof(PodData));
        uint8_t *data = ow_read_mem(pod_store.pods[x].rom_code);
        if (data[0] == 0xFF || data[1] == 0xFF || data[2] == 0xFF || data[3] == 0xFF || data[4] == 0xFF || data[5] == 0xFF)
            return; //data = ow_read_mem(pod_store.pods[x].rom_code);
        pod_store.pods[x].pod_id = (data[0] << 8) + data[1];
        pod_store.pods[x].scent_id = (data[2] << 8) + data[3];
        pod_store.pods[x].scent_level = data[4];
        pod_store.pods[x].pod_position = data[5];
        
        sendPod(x, pod_store.pods[x].pod_id);
        __delay_ms(50);
        sendScentIDs(x, pod_store.pods[x].scent_id);
        __delay_ms(50);
        sendScentLevel(x, pod_store.pods[x].scent_level);
        __delay_ms(50);
    }
}

void read_pod_eeprom(int index, int data_type)
{
    //memset(&pod_store.pods[index], 0, sizeof(PodData));
    uint8_t *data = ow_read_mem(pod_store.pods[index].rom_code);
    if (data[0] == 0xFF || data[1] == 0xFF || data[2] == 0xFF || data[3] == 0xFF || data[4] == 0xFF || data[5] == 0xFF)
        return; //data = ow_read_mem(pod_store.pods[index].rom_code);
    pod_store.pods[index].pod_id = (data[0] << 8) + data[1];
    pod_store.pods[index].scent_id = (data[2] << 8) + data[3];
    pod_store.pods[index].scent_level = data[4];
    pod_store.pods[index].pod_position = data[5];

    if (0 == data_type)
        sendPod(index, pod_store.pods[index].pod_id);
    else if (1 == data_type)//__delay_ms(200);
        sendScentIDs(index, pod_store.pods[index].scent_id);
    else if (2 == data_type)//__delay_ms(200);
        sendScentLevel(index, pod_store.pods[index].scent_level);
    //__delay_ms(200);
}

void verify_eeprom(int index, int override_pos)
{
    uint8_t rom_code[8] = {0}, written = 0, data_rep[6] = {0};
    PodData temp_pod;
    uint16_t pod_id = (pod_store.pods[index].rom_code[1] << 8) + pod_store.pods[index].rom_code[2];
    pod_id += pod_store.pods[index].rom_code[7];
    do
    {
        __delay_ms(100);
        memcpy(rom_code, pod_store.pods[index].rom_code, 8);
        uint8_t *tdata = ow_read_mem(rom_code);
        memcpy(data_rep, tdata, 6);
        temp_pod.pod_id = (tdata[0] << 8) + tdata[1];
        temp_pod.scent_id = (tdata[2] << 8) + tdata[3];
        temp_pod.scent_level = tdata[4];
        temp_pod.pod_position = tdata[5];

        //if (temp_pod.pod_id != pod_id || pod_store.pods[index].pod_id != pod_id)
        if (!temp_pod.pod_id || 0xFFFF == temp_pod.pod_id)
            pod_store.pods[index].pod_id = pod_id;
        
        if (temp_pod.scent_id && temp_pod.scent_id <= DIESEL)
            pod_store.pods[index].scent_id = temp_pod.scent_id;
        
        if (temp_pod.scent_level && temp_pod.scent_level <= 100)
            pod_store.pods[index].scent_level = temp_pod.scent_level;

        if (override_pos)
        {
            pod_store.pods[index].scent_level = 100;
            uint8_t dummy[6] = {0};
            dummy[0] = (pod_store.pods[index].pod_id & 0xFF00) >> 8; 
            dummy[1] = (pod_store.pods[index].pod_id & 0x00FF);
            dummy[2] = (pod_store.pods[index].scent_id & 0xFF00) >> 8;
            dummy[3] = (pod_store.pods[index].scent_id & 0x00FF);
            dummy[4] = pod_store.pods[index].scent_level;
            dummy[5] = pod_store.pods[index].pod_position;
            write_save_eeprom(pod_store.pods[index].rom_code, 0x0000, dummy, 6);
            __delay_ms(1);
            written = 1;
        }
    }
    while ((pod_store.pods[index].pod_position != temp_pod.pod_position || 
            pod_store.pods[index].pod_id != temp_pod.pod_id) || 
            !override_pos);
    __delay_ms(100);
}

int find_channel_by_pod_code(uint8_t *code)
{
    for (int i = 0; i < pod_store.pods_present; i++)
    {
        if (!memcmp(pod_store.pods[i].rom_code, code, 8))
        {
            return i;
        }
    }
    return -1;
}

PodData find_by_pod_id(uint16_t id)
{
    PodData ret;
    
    for (int i = 0; i < pod_store.pods_present; i++)
    {
        if (pod_store.pods[i].pod_id == id)
        {
            memcpy(ret.rom_code, pod_store.pods[i].rom_code, 8);
            ret.pod_id = pod_store.pods[i].pod_id;
            ret.scent_id = pod_store.pods[i].scent_id;
            ret.pod_present = pod_store.pods[i].pod_present;
            ret.pod_position = pod_store.pods[i].pod_position;
        }
    }
    return ret;
}

PodData find_by_scent_id(uint16_t id)
{
    PodData ret;
    
    for (int i = 0; i < pod_store.pods_present; i++)
    {
        if (pod_store.pods[i].scent_id == id)
        {
            memcpy(ret.rom_code, pod_store.pods[i].rom_code, 8);
            ret.pod_id = pod_store.pods[i].pod_id;
            ret.scent_id = pod_store.pods[i].scent_id;
            ret.pod_present = pod_store.pods[i].pod_present;
            ret.pod_position = pod_store.pods[i].pod_position;
        }
    }
    return ret;
}

PodData find_by_channel_num(uint8_t channel)
{
    PodData ret;
    memcpy(ret.rom_code, pod_store.pods[channel].rom_code, 8);
    ret.pod_id = pod_store.pods[channel].pod_id;
    ret.scent_id = pod_store.pods[channel].scent_id;
    ret.pod_present = pod_store.pods[channel].pod_present;
    ret.pod_position = pod_store.pods[channel].pod_position;
    return ret;
}

uint8_t set_pod_id(PodData pod, uint16_t id)
{
    uint8_t buffer[6] = {0};
    buffer[0] = ((id & 0xFF00) >> 8);
    buffer[1] = (id & 0x00FF);
    buffer[2] = ((pod.scent_id & 0xFF00) >> 8);
    buffer[3] = (pod.scent_id & 0x00FF);
    buffer[4] = pod.scent_level;
    buffer[5] = pod.pod_position;
    
    return write_save_eeprom(pod.rom_code, 0x0000, buffer, 6);
}

uint8_t set_scent_id(PodData pod, uint16_t id)
{
    uint8_t buffer[6] = {0};
    buffer[0] = ((pod.pod_id & 0xFF00) >> 8);
    buffer[1] = (pod.pod_id & 0x00FF);
    buffer[2] = ((id & 0xFF00) >> 8);
    buffer[3] = (id & 0x00FF);
    buffer[4] = 100; //pod.scent_level;
    buffer[5] = pod.pod_position;
    
    return write_save_eeprom(pod.rom_code, 0x0000, buffer, 6); //res;    
}

uint8_t set_pod_position(PodData pod, uint8_t position)
{
    uint8_t buffer[6] = {0};
    buffer[0] = ((pod.pod_id & 0xFF00) >> 8);
    buffer[1] = (pod.pod_id & 0x00FF);
    buffer[2] = ((pod.scent_id & 0xFF00) >> 8);
    buffer[3] = (pod.scent_id & 0x00FF);
    buffer[4] = pod.scent_level;
    buffer[5] = position;
    
    return write_save_eeprom(pod.rom_code, 0x0000, buffer, 6);    
}

uint8_t set_scent_level(PodData pod, uint8_t level)
{
#if DEV_NO_OWIRE
    // DEV MODE: Update in-RAM pod_store only; skip 1-Wire EEPROM writes.
    int idx = pod.pod_position ? (pod.pod_position - 1) : -1;
    if (idx >= 0 && idx < MAX_PODS)
    {
        pod_store.pods[idx].scent_level = level;
        return 1;
    }
    return 0;
#else
    uint8_t buffer[6] = {0};
    buffer[0] = ((pod.pod_id & 0xFF00) >> 8);
    buffer[1] = (pod.pod_id & 0x00FF);
    buffer[2] = ((pod.scent_id & 0xFF00) >> 8);
    buffer[3] = (pod.scent_id & 0x00FF);
    buffer[4] = level;
    buffer[5] = pod.pod_position;
    
    return write_save_eeprom(pod.rom_code, 0x0000, buffer, 6);        
#endif
}

int is_present(uint8_t *code)
{
    int i = 0, ret = -1;
    uint8_t comp_code[8] = {0};
    memcpy(comp_code, code, 8);
    for (; i < MAX_PODS; i++)
    {
        if (!memcmp(pod_store.pods[i].rom_code, comp_code, 8))
            return i;
    }
    return ret;
}

void pod_added_led(void)
{
    // Removed LED indication
}

void pod_removed_led(void)
{
    // Removed LED indication
}

void pod_error_led(void)
{
    // Removed LED indication
}

uint8_t all_roms[6][8];
int is_present_all_roms(uint8_t *code)
{
    int i = 0, ret = -1;
    for (; i < 6; i++)
    {
        if (!memcmp(all_roms[i], code, 8))
            return i;
    }
    return ret;
}

int initial_search(int wait_until_all_found)
{
    uint8_t found_rom[8] = {0};
    bool no_more_found = false;
    int count = 0;
    memset(all_roms, 0, (6 * 8));
    
    while ((count < MAX_PODS && !no_more_found) || wait_until_all_found)
    {
        if (ow_search_rom(found_rom))
        {
            if (-1 == is_present_all_roms(found_rom))
                memcpy(all_roms[count++], found_rom, 8);
        }
        else
            no_more_found = true;
        
        if (count == wait_until_all_found)
            break;
    }
    
    return count;
}

int find_missing(void)
{
    int i, j;
    int present[6] = {0};
    
    for (i = 0; i < MAX_PODS; i++)
    {
        for (j = 0; j < MAX_PODS; j++)
        {
            if (!memcmp(pod_store.pods[i].rom_code, all_roms[j], 8))
                present[i] = 1;
        }
    }
        
    for (i = 0; i < MAX_PODS; i++)
    {
        if (!present[i])
            return i;
    }
    return -1;
}

void add_pod(bool all_pods)
{
    uint8_t found_rom[8] = {0}, new_rom[8] = {0};
    bool no_more_found = false;
    
    while (0 == new_rom[0] && !no_more_found)
    {
        // If no 1-Wire hardware is present, ow_search_rom will return 0.
        // Previously, the 'else' branches were commented out, causing an
        // infinite loop. Restore the early-exit behavior so we break cleanly
        // when nothing is found.
        if (ow_search_rom(found_rom))
        {
            if (-1 == is_present(found_rom))
                memcpy(new_rom, found_rom, 8);
            else
                no_more_found = true;
        }
        else
            no_more_found = true;
    }
    
    if (new_rom[0] != 0)
    {
        if (all_pods)
        {
            uint8_t *data = ow_read_mem(new_rom);
            uint8_t pos = data[5];
            if (!pos)
                pos = 6;
            else if (pos > 6)
            {
                pod_error_led();
                mode = LOCATE_REMOVE;
                return;
            }
            memcpy(pod_store.pods[pos-1].rom_code, new_rom, 8);
            pod_store.pods[pos-1].pod_present = 1;
            pod_store.pods[pos-1].pod_id = 0;
            pod_store.pods[pos-1].scent_id = 0;
            pod_store.pods[pos-1].pod_position = pos;
            last_all_pods = true;
            verify_eeprom(pos-1, 1);
        }
        else
        {
            int j = pod_store.pods_present;
            memcpy(pod_store.pods[j].rom_code, new_rom, 8);
            pod_store.pods[j].pod_present = 1;
            pod_store.pods[j].pod_id = 0;
            pod_store.pods[j].scent_id = 0;
            pod_store.pods[j].pod_position = j+1;
            verify_eeprom(j, 1);
        }
        pod_store.pods_present++;
        pod_added_led();
    }
}

int pod_index = 0, pod_data_type = 0;
void pods_idle(void)
{    
    if (MAX_PODS != pod_store.pods_present)
        return;
    
    //INTCON2bits.GIE = 0;
    //enableSerial(false);
    /*if (pod_index >= MAX_PODS)
    {
        pod_index = 0;
        pod_data_type++;
    }
    
    if (pod_data_type > 2)
        pod_data_type = 0;
    read_pod_eeprom(pod_index++, pod_data_type); */
    read_eeprom(1);
    //INTCON2bits.GIE = 1;
    //enableSerial(true);
    //__delay_ms(2000);
}

void remove_pod(void)
{
    memset(&pod_store, 0, sizeof(PodStore));
    
    if (!pod_store.pods_present)
    {
        __delay_ms(15000);
        pod_removed_led();
        //CLEAR_LEDS;
        //IDLE_PURPLE;
        mode = LOCATE_ADD;
    }
}

int locate_pods(void)
{
#if DEV_NO_OWIRE
    // DEV MODE: Mock six pods so piezo commands can run without 1-Wire devices.
    // This bypasses bus enumeration and EEPROM I/O.
    if (!pod_store.pods_present)
    {
        memset(&pod_store, 0, sizeof(PodStore));
        for (int i = 0; i < MAX_PODS; i++)
        {
            // Provide deterministic nonzero IDs and full scent levels
            pod_store.pods[i].pod_present = 1;
            pod_store.pods[i].pod_id = 100 + i;   // arbitrary nonzero
            pod_store.pods[i].scent_id = 1;
            pod_store.pods[i].scent_level = 100;
            pod_store.pods[i].pod_position = i + 1;
        }
        pod_store.pods_present = MAX_PODS;
        last_all_pods = true;
    }
    
    return pod_store.pods_present;
#else
    int pods_on_bus = initial_search(0);
    
    if (!last_pods_present && mode != LOCATE_REMOVE)
        mode = LOCATE_ADD;
    else if (pod_store.pods_present < MAX_PODS && mode != LOCATE_REMOVE)
        mode = LOCATE_ADD;
    else if ((last_pods_present == pod_store.pods_present && MAX_PODS == pod_store.pods_present) && mode != LOCATE_REMOVE)
        mode = LOCATE_IDLE;
    else if (mode != LOCATE_REMOVE)
        mode = LOCATE_IDLE;
    
    switch (mode)
    {
        default:
            return -1;
        case LOCATE_ADD:
            
            add_pod((MAX_PODS == pods_on_bus && !last_pods_present) || last_all_pods);
            last_pods_present = pod_store.pods_present;
            break;
        case LOCATE_IDLE:
            
            pods_idle();
            last_pods_present = pod_store.pods_present;
            break;
        case LOCATE_REMOVE:
            
            last_all_pods = false;
            remove_pod();
            last_pods_present = pod_store.pods_present;
            break;
    }
    
    return pod_store.pods_present;
#endif
}