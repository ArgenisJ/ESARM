#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

uint8_t ui8PinData=2;

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_10|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    while(1)
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8PinData);
        SysCtlDelay(2000000);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
        SysCtlDelay(2000000);
        // Green = 8, Blue = 4, Red = 2

        // R.. G.. B.. RG.. RB.. GB.. RGB..
        if(ui8PinData==14) {ui8PinData=2;}
        else if (ui8PinData==2) {ui8PinData=8;}
        else if (ui8PinData==8) {ui8PinData=4;}
        else if (ui8PinData==4) {ui8PinData=10;}
        else if (ui8PinData==10) {ui8PinData=6;}
        else if (ui8PinData==6) {ui8PinData=12;}
        else if (ui8PinData==12) {ui8PinData=14;}

        // Blue.. Green.. Red..
        //if (ui8PinData==2) {ui8PinData=4;}
        //else if (ui8PinData==4) {ui8PinData=8;}
        //else if (ui8PinData==8) {ui8PinData=2;}

        // Red.. Blue.. Green..
        //if(ui8PinData==8) {ui8PinData=2;} else {ui8PinData=ui8PinData*2;}

    }
}
