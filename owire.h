/*
 * File:   owire.h
 * Author: ALN
 * Description: 1-Wire EEprom header file for Scentient Collar Firmware
 * Comments: Copyright 2025 PD Studio 29 LTD
 * Created on 13 January 2025, 10:00
 */

#ifdef BOARD_VER_A
#define OWPINI  PORTFbits.RF6
#define OWPINO  LATFbits.LATF6
#define TOW     TRISFbits.TRISF6
#else
#define OWPINI  PORTGbits.RG3
#define OWPINO  LATGbits.LATG3
#define TOW     TRISGbits.TRISG3
#endif
#define OWIN    OWPINO = 1
#define OWOUT   OWPINO = 0
#define TOWIN   TOW = 1
#define TOWOUT  TOW = 0;

#define MAX_PODS    6  // 6 true max

typedef struct _POD_DATA {
    uint8_t rom_code[8];    // 8 byte rom code for eeprom.
    uint16_t pod_id;
    uint16_t scent_id;
    uint8_t scent_level;
    uint8_t pod_present;
    uint8_t pod_position;
} PodData;

typedef struct _POD_STORE {
    PodData pods[6];
    
    int pods_present;
    int current_pod;
} PodStore;

typedef enum _LOCATE_MODE {
    LOCATE_IDLE = 0,
    LOCATE_ADD,
    LOCATE_REMOVE
} LocateMode;

typedef enum _CURRENT_SCENTS {
    NOTHING = 0,
    SMOKE,
    GAS,
    CAR,
    CHOCOLATE,
    LAVENDER,
    ORANGE,
    RAINFOREST,
    STIR_FRY,
    VANILLA,
    WATERFALL,
    WAVES,
    PEPPERMINT,
    COFFEE,
    BAKERY,
    LAUNDRY,
    GREEN_FIG,
    FLORAL_BLEND,
    OZONE,
    MEDITERRANEAN,
    ANTISEPTIC,
    RUBBER,
    BLOOD,
    SMOKE_DUPE,
    VOMIT,
    SWEAT,
    DIESEL,
} CurrentScents;

extern PodStore pod_store;
extern LocateMode mode;

extern int ow_search_rom(uint8_t *sink);
extern uint8_t *ow_read_mem(uint8_t *rom_code);
extern uint8_t *ow_write_scratchpad(uint8_t *rom_code, uint16_t address, uint8_t *to_write, uint8_t length);
extern uint8_t *ow_read_scratchpad(uint8_t *rom_code, uint16_t *address, uint8_t *es, uint16_t *crc);
extern uint8_t ow_copy_scratchpad(uint8_t *rom_code, uint16_t address, uint8_t es);
extern void read_eeprom(int ignore_pos);
extern void read_pod_eeprom(int index, int data_type);
extern uint8_t write_save_eeprom(uint8_t *rom_code, uint16_t address, uint8_t *data, uint8_t length) ;

extern int find_channel_by_pod_code(uint8_t *code);
extern PodData find_by_pod_id(uint16_t id);
extern PodData find_by_scent_id(uint16_t id);
extern PodData find_by_channel_num(uint8_t channel);
extern uint8_t set_pod_id(PodData pod, uint16_t id);
extern uint8_t set_scent_id(PodData pod, uint16_t id);
extern uint8_t set_pod_position(PodData pod, uint8_t position);
extern uint8_t set_scent_level(PodData pod, uint8_t level);
extern int locate_pods(void);