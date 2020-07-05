// lcd.c - Source code for the LCD.
// Copyright (c) 2020-2020 Le Minh Tai.  All rights reserved.

#include <stdbool.h>
#include <stdint.h>

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h" // needed for GPIO.
#include "inc/hw_memmap.h" // needed for GPIO.

#include "drivers/lcd.h"

#ifdef UART_DEBUG
#include "driverlib/uart.h" // needed for UART, UARTstdio.
#include "utils/uartstdio.h" // needed for UARTStdioConfig(), UARTprintf(), etc.
#endif

static PinConfig PinTable[16];
static uint8_t LCDState = 1;

void LCDDelayms(uint32_t ms)
{
    SysCtlDelay((SysCtlClockGet() / 3000) * ms);
}

void LCDDelayus(uint32_t us)
{
    SysCtlDelay((SysCtlClockGet() / 3000000) * us);
}

void LCDPinAssign(uint32_t ui32Port, uint8_t ui8Pin, uint8_t LCDPin)
{
    PinTable[LCDPin].ui32Port = ui32Port;
    PinTable[LCDPin].ui8Pin = ui8Pin;
    uint32_t sysctl;
    switch (ui32Port)
    {
    case GPIO_PORTA_BASE:
        sysctl = SYSCTL_PERIPH_GPIOA;
        break;
    case GPIO_PORTB_BASE:
        sysctl = SYSCTL_PERIPH_GPIOB;
        break;
    case GPIO_PORTC_BASE:
        sysctl = SYSCTL_PERIPH_GPIOC;
        break;
    case GPIO_PORTD_BASE:
        sysctl = SYSCTL_PERIPH_GPIOD;
        break;
    case GPIO_PORTE_BASE:
        sysctl = SYSCTL_PERIPH_GPIOE;
        break;
    case GPIO_PORTF_BASE:
        sysctl = SYSCTL_PERIPH_GPIOF;
        break;
    }
    SysCtlPeripheralEnable(sysctl);
    GPIOPinTypeGPIOOutput(ui32Port, ui8Pin);
}

void LCDPinSet(uint8_t LCDPin, uint8_t Val)
{
    if (Val)
        GPIOPinWrite(PinTable[LCDPin].ui32Port, PinTable[LCDPin].ui8Pin, PinTable[LCDPin].ui8Pin);
    else
        GPIOPinWrite(PinTable[LCDPin].ui32Port, PinTable[LCDPin].ui8Pin, 0);
}

uint8_t LCDPinGet(uint8_t LCDPin)
{
    GPIOPinTypeGPIOInput(PinTable[LCDPin].ui32Port, PinTable[LCDPin].ui8Pin);
    uint8_t Val = GPIOPinRead(PinTable[LCDPin].ui32Port, PinTable[LCDPin].ui8Pin);
    GPIOPinTypeGPIOOutput(PinTable[LCDPin].ui32Port, PinTable[LCDPin].ui8Pin);
    if (Val)
        return 0x1;
    else
        return 0x0;
}

void LCDDataSet(uint8_t Data)
{
#ifndef LCD_4_BIT_INTERFACE
    LCDPinSet(LCD_D0_PIN, Data & 0x01);
    LCDPinSet(LCD_D1_PIN, (Data >> 1) & 0x01);
    LCDPinSet(LCD_D2_PIN, (Data >> 2) & 0x01);
    LCDPinSet(LCD_D3_PIN, (Data >> 3) & 0x01);
#endif
    LCDPinSet(LCD_D4_PIN, (Data >> 4) & 0x01);
    LCDPinSet(LCD_D5_PIN, (Data >> 5) & 0x01);
    LCDPinSet(LCD_D6_PIN, (Data >> 6) & 0x01);
    LCDPinSet(LCD_D7_PIN, (Data >> 7) & 0x01);
}

#define LCDDataGet(x) LCDCharGet(x)

void LCDInit(void)
{
#ifdef LCD_4_BIT_INTERFACE
    LCDInstruct(0x28); // Format: 0 0 1 DL  N F _ _ : 8/4 bits, 2/1 lines, 5x10/5x8 dots
#else
    LCDInstruct(0x38); // Format: 0 0 1 DL  N F _ _ : 8/4 bits, 2/1 lines, 5x10/5x8 dots
#endif
    LCDInstruct(0x01); // Format: 0 0 0 0  0 0 0 1 : Clear entire display.
    LCDInstruct(0x06); // Format: 0 0 0 0  0 1 I/D S : Increase/Decrease cursor move direction, display shift allowed on/off.
    LCDInstruct(0x0E); // Format: 0 0 0 0  1 D C B : display on/off, cursor on/off, shift/not_shift display.
}

void LCDBusyWait(void)
{
    LCDPinSet(LCD_RS_PIN, 0);
    LCDPinSet(LCD_RW_PIN, 1);
    uint32_t BusyFlag;
    do
    {
        LCDPinSet(LCD_EN_PIN, 1);
        BusyFlag = LCDPinGet(LCD_D7_PIN);
        LCDPinSet(LCD_EN_PIN, 0);
#ifdef LCD_4_BIT_INTERFACE
        LCDPinSet(LCD_EN_PIN, 1);
        LCDPinSet(LCD_EN_PIN, 0);
#endif
    }
    while (BusyFlag);
}

void LCDInstruct(uint8_t Cmd)
{
    LCDBusyWait();
    LCDPinSet(LCD_RS_PIN, 0);
    LCDPinSet(LCD_RW_PIN, 0);
    LCDDataSet(Cmd);
    LCDPinSet(LCD_EN_PIN, 1);
    LCDPinSet(LCD_EN_PIN, 0);
#ifdef LCD_4_BIT_INTERFACE
    LCDDataSet(Cmd << 4);
    LCDPinSet(LCD_EN_PIN, 1);
    LCDPinSet(LCD_EN_PIN, 0);
#endif
}

void LCDCharPut(uint8_t Char)
{
    LCDBusyWait();
    LCDPinSet(LCD_RS_PIN, 1);
    LCDPinSet(LCD_RW_PIN, 0);
    LCDDataSet(Char);
    LCDPinSet(LCD_EN_PIN, 1);
    LCDPinSet(LCD_EN_PIN, 0);
#ifdef LCD_4_BIT_INTERFACE
    LCDDataSet(Char << 4);
    LCDPinSet(LCD_EN_PIN, 1);
    LCDPinSet(LCD_EN_PIN, 0);
#endif
}

uint8_t LCDCharGet(void)
{
    LCDPinSet(LCD_RS_PIN, 1);
    LCDPinSet(LCD_RW_PIN, 1);
    LCDPinSet(LCD_EN_PIN, 1);
    uint8_t Char = (LCDPinGet(LCD_D7_PIN) << 7) | (LCDPinGet(LCD_D6_PIN) << 6) |
                   (LCDPinGet(LCD_D5_PIN) << 5) | (LCDPinGet(LCD_D4_PIN) << 4)
#ifndef LCD_4_BIT_INTERFACE
                   | (LCDPinGet(LCD_D3_PIN) << 3) | (LCDPinGet(LCD_D2_PIN) << 2) |
                   (LCDPinGet(LCD_D1_PIN) << 1) | (LCDPinGet(LCD_D0_PIN) << 0)
#endif
    ;
    LCDPinSet(LCD_EN_PIN, 0);
#ifdef LCD_4_BIT_INTERFACE
    LCDPinSet(LCD_EN_PIN, 1);
    Char = Char | (LCDPinGet(LCD_D7_PIN) << 3) | (LCDPinGet(LCD_D6_PIN) << 2) |
           (LCDPinGet(LCD_D5_PIN) << 1) | (LCDPinGet(LCD_D4_PIN) << 0);
    LCDPinSet(LCD_EN_PIN, 0);
#endif
    return Char;
}

void LCDStringWrite(const uint8_t *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        LCDCharPut(String[i]);
    }
}

void LCDAddressSet(uint8_t Add)
{
    LCDInstruct(Add | 0x80);
}

uint8_t LCDAddressGet(void)
{
    LCDPinSet(LCD_RS_PIN, 0);
    LCDPinSet(LCD_RW_PIN, 1);
    LCDPinSet(LCD_EN_PIN, 1);
    uint8_t Add = (LCDPinGet(LCD_D6_PIN) << 6) |
                  (LCDPinGet(LCD_D5_PIN) << 5) | (LCDPinGet(LCD_D4_PIN) << 4)
#ifndef LCD_4_BIT_INTERFACE
                  | (LCDPinGet(LCD_D3_PIN) << 3) | (LCDPinGet(LCD_D2_PIN) << 2) |
                  (LCDPinGet(LCD_D1_PIN) << 1) | (LCDPinGet(LCD_D0_PIN) << 0)
#endif
    ;
    LCDPinSet(LCD_EN_PIN, 0);
#ifdef LCD_4_BIT_INTERFACE
    LCDPinSet(LCD_EN_PIN, 1);
    Add = Add | (LCDPinGet(LCD_D7_PIN) << 3) | (LCDPinGet(LCD_D6_PIN) << 2) |
          (LCDPinGet(LCD_D5_PIN) << 1) | (LCDPinGet(LCD_D4_PIN) << 0);
    LCDPinSet(LCD_EN_PIN, 0);
#endif
    return Add;
}

void LCDReturn(void)
{
    uint8_t add = LCDAddressGet();
    LCDInstruct(0x02);
    if ((add >> 6) == 0)
        LCDAddressSet(0x40);
}

void LCDHome(void)
{
    uint8_t add = LCDAddressGet();
    LCDInstruct(0x02);
    if (add >> 6)
        LCDAddressSet(0x40);
}

void LCDEnd(void)
{
    uint8_t add = LCDAddressGet();
    if (add >> 6)
        LCDAddressSet(0x67);
    else
        LCDAddressSet(0x40);

    LCDInstruct(0x4);
    while (LCDCharGet() == ' ');
    LCDInstruct(0x06);
    LCDInstruct(0x14); // Move cursor right
}

void LCDBackspace(void)
{
    uint8_t add = LCDAddressGet();
    if ((add != 0) && (add != 0x40))
    {
        LCDInstruct(0x10); // Move cursor left
        LCDCharPut(' ');
        LCDInstruct(0x10); // Move cursor left
    }
}

void LCDDisplayClear(void)
{
    LCDInstruct(0x01);
}

void LCDOnOffToggle(void)
{
    if (LCDState)
        LCDInstruct(0x08);
    else
        LCDInstruct(0x0E);
    LCDState = !LCDState;
#ifdef UART_DEBUG
    UARTprintf("LCDState = %d\n", LCDState);
#endif
}
void LCDCharEcho(uint8_t Char)
{
    switch (Char)
    {
    case '/':
        LCDReturn();
        break;
    case '\\':
        LCDBackspace();
        break;
    case '[':
        LCDInstruct(0x10);
        break;
    case ']':
        LCDInstruct(0x14);
        break;
    case ',':
        LCDHome();
        break;
    case '.':
        LCDEnd();
        break;
    default:
        LCDCharPut((uint8_t) Char);
    }
}
