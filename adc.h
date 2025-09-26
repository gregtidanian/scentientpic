#define V_C_CHN 0x0C
#define V_T_CHN 0x0D
#define V_B_CHN 0x0E

extern void setup_adc(void);
extern uint16_t calc_adc(uint8_t channel);