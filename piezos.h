

// CHANGED: PIEZO_Rx old G6/E7/E6 -> new B7/B8/B9
#define PIEZO_R1    LATBbits.LATB7
#define PIEZO_R2    LATBbits.LATB8
#define PIEZO_R3    LATBbits.LATB9

// CHANGED: PIEZO_Lx old D10/D9/D8 -> new E1/E0/F1
#define PIEZO_L1    LATEbits.LATE1
#define PIEZO_L2    LATEbits.LATE0
#define PIEZO_L3    LATFbits.LATF1

typedef struct _PIEZO_FIFO {
    uint8_t channel;
    uint8_t intensity;
    uint16_t duration;
} PiezoFIFOItem;

extern uint8_t piezo_interrupt;

extern int fire_piezo_R1(uint8_t intensity, uint16_t duration);
extern int fire_piezo_R2(uint8_t intensity, uint16_t duration);
extern int fire_piezo_R3(uint8_t intensity, uint16_t duration);
extern int fire_piezo_L1(uint8_t intensity, uint16_t duration);
extern int fire_piezo_L2(uint8_t intensity, uint16_t duration);
extern int fire_piezo_L3(uint8_t intensity, uint16_t duration);
extern void piezo_handler(void);
extern uint8_t adjust_intensity(uint8_t percentage);
// CHANGED: adjust_voltage not used -> commented out
// extern void adjust_voltage(uint8_t state, uint8_t steps);

extern void piezo_fifo_init(void);
extern void piezo_fifo_right_get(PiezoFIFOItem *item);
extern void piezo_fifo_right_put(PiezoFIFOItem *item);
extern int piezo_fifo_right_items(void);
extern void piezo_fifo_left_get(PiezoFIFOItem *item);
extern void piezo_fifo_left_put(PiezoFIFOItem *item);
extern int piezo_fifo_left_items(void);