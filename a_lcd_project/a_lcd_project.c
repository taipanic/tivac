// Main program for testing the LCD.
// Copyright (c) 2020-2020 Le Minh Tai.  All rights reserved.

#include <stdbool.h>
#include <stdint.h>

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"     // Needed for GPIO.
#include "inc/hw_memmap.h"      // Needed for GPIO.

#include "driverlib/pin_map.h"  // Needed for GPIOPinConfigure() (e.g., in UART).
#include "driverlib/uart.h"     // Needed for UART, UARTstdio.
#include "utils/uartstdio.h"    // Needed for UARTStdioConfig(), UARTprintf(), etc.

#include "drivers/lcd.h"        // Needed for LCD.

// Some prototypes for function used in main program.
void ConfigureGPIO(void);
void ConfigureLCD(void);
void ConfigureUART(void);
void UARTIntHandler(void);
void ConfigureUARTInt(void);

// Main program.
int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                   SYSCTL_OSC_MAIN);
    ConfigureGPIO();
    ConfigureUART();
    ConfigureUARTInt();

    ConfigureLCD();
    LCDInit();
    const uint8_t *String = "Hello my friend"; // Test string.
    LCDStringWrite(String);

    while (1)
    {
        if (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0) == 0) // SW2.
        {
            while (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0) == 0);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // Turn on red LED.
            UARTprintf("SW2 pressed\n");
            LCDDelayms(50);
            LCDDisplayClear();
        } else
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); // Turn off red LED.
        if (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == 0) // SW1.
        {
            while (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == 0);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); //  Turn on green LED.
            UARTprintf("SW1 pressed\n");
            LCDDelayms(50);
            LCDOnOffToggle(); // Toggle On/Off the text on the display.
        } else
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); //  Turn off green LED.
    }
}

// Connect LCD to J1 on board, from PB1 to PA7
void ConfigureLCD(void)
{
    LCDPinAssign(GPIO_PORTB_BASE, GPIO_PIN_1, LCD_RS_PIN); // RS.
    LCDPinAssign(GPIO_PORTE_BASE, GPIO_PIN_4, LCD_RW_PIN); // RW.
    LCDPinAssign(GPIO_PORTE_BASE, GPIO_PIN_5, LCD_EN_PIN); // EN.
#ifndef LCD_4_BIT_INTERFACE // Change x if 8-bit interface is used. View lcd.h to learnmore.
    LCDPinAssign(GPIO_PORTx_BASE, GPIO_PIN_x, LCD_D0_PIN); // D0
    LCDPinAssign(GPIO_PORTx_BASE, GPIO_PIN_x, LCD_D1_PIN); // D1
    LCDPinAssign(GPIO_PORTx_BASE, GPIO_PIN_x, LCD_D2_PIN); // D2
    LCDPinAssign(GPIO_PORTx_BASE, GPIO_PIN_x, LCD_D3_PIN); // D3
#endif
    LCDPinAssign(GPIO_PORTB_BASE, GPIO_PIN_4, LCD_D4_PIN); // D4
    LCDPinAssign(GPIO_PORTA_BASE, GPIO_PIN_5, LCD_D5_PIN); // D5
    LCDPinAssign(GPIO_PORTA_BASE, GPIO_PIN_6, LCD_D6_PIN); // D6
    LCDPinAssign(GPIO_PORTA_BASE, GPIO_PIN_7, LCD_D7_PIN); // D7
}

void ConfigureGPIO(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOUnlockPin(GPIO_PORTF_BASE, GPIO_PIN_0);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
}

void ConfigureUART(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, SysCtlClockGet());
}

void ConfigureUARTInt(void)
{
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    UARTIntRegister(UART0_BASE, UARTIntHandler); // When using this, no need to edit startup file.
}

void UARTIntHandler(void)
{
    uint32_t ui32Status = UARTIntStatus(UART0_BASE, true); // Get UART status.
    UARTIntClear(UART0_BASE, ui32Status); // Clear it instantly.

    uint32_t received_char; // To store received char.
    while(UARTCharsAvail(UART0_BASE)) // Loop if chars is still available.
    {
        received_char = UARTCharGetNonBlocking(UART0_BASE);
        LCDCharEcho((uint8_t) received_char); // Echo it to LCD, too.
        UARTCharPutNonBlocking(UART0_BASE, received_char); // Echo it to Terminal (COM5 115200).
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); // Light up LED shortly.
        SysCtlDelay(SysCtlClockGet() / 3000);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
    }
}
