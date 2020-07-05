// lcd.h - Defines and Macros for the LCD.
// Copyright (c) 2020-2020 Le Minh Tai.  All rights reserved.

#ifndef __LCD_H__
#define __LCD_H__

// LCD setting.
#define UART_DEBUG
#define LCD_4_BIT_INTERFACE // Comment out this line if 8-bit interface is used (not recommended).

// Defines for the hardware resources used by the LCD.
#define LCD_RS_PIN      0x3 // Registers selects
#define LCD_RW_PIN      0x4 // Read/_Write
#define LCD_EN_PIN      0x5 // Enable (E)
#define LCD_D0_PIN      0x6 // DB0
#define LCD_D1_PIN      0x7 // DB1
#define LCD_D2_PIN      0x8 // DB2
#define LCD_D3_PIN      0x9 // DB3
#define LCD_D4_PIN      0xA // DB4
#define LCD_D5_PIN      0xB // DB5
#define LCD_D6_PIN      0xC // DB6
#define LCD_D7_PIN      0xD // DB7

typedef struct {
    uint8_t ui8Pin;
    uint32_t ui32Port;
} PinConfig; // To configure Port Pin correctly

// Functions exported from lcd.c
extern void LCDDelayms(uint32_t ms);
extern void LCDDelayus(uint32_t us);
extern void LCDPinAssign(uint32_t ui32Port, uint8_t ui8Pin, uint8_t LCDPin);
extern void LCDPinSet(uint8_t LCDPin, uint8_t Val);
extern uint8_t LCDPinGet(uint8_t LCDPin);
extern void LCDDataSet(uint8_t Data);
extern uint8_t LCDDataGet(void);
extern void LCDInit(void);
extern void LCDBusyWait(void);
extern void LCDInstruct(uint8_t Cmd);
extern void LCDCharPut(uint8_t Char);
extern uint8_t LCDCharGet(void);
extern void LCDStringWrite(const uint8_t *String);
extern void LCDAddressSet(uint8_t Add);
extern uint8_t LCDAddressGet(void);
extern void LCDReturn(void);
extern void LCDHome(void);
extern void LCDEnd(void);
extern void LCDBackspace(void);
extern void LCDCharEcho(uint8_t Char);
extern void LCDDisplayClear(void);
extern void LCDOnOffToggle(void);

#endif // __LCD_H__
