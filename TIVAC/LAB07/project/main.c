// Argenis Jimenez Aguirre
// CPE 403
// TIVAC Lab 07
// Task 02
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "driverlib/adc.h"

int main(void) {
    // Run 40MHz System Clock
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);


    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Enable ADC0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ADCHardwareOversampleConfigure(ADC0_BASE, 64);

    // Use highest priority and set ADC0 and SS1
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

    // Sample steps 0-2 on Sequencer 1
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);

    // Configure Interrupt flag and sample final step in Sequencer 1
    ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);

    // Enable Sequencer 1
    ADCSequenceEnable(ADC0_BASE, 1);

    // Enable RX and TX
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); //enable GPIO port for LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3); //enable pin for LED PF2

    // Set rate of data to 115200
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    //IntMasterEnable(); //enable processor interrupts
    IntEnable(INT_UART0); //enable the UART interrupt
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); //only enable RX and TX interrupts

    // Display starting commands
    UARTCharPut(UART0_BASE, 'E');
    UARTCharPut(UART0_BASE, 'n');
    UARTCharPut(UART0_BASE, 't');
    UARTCharPut(UART0_BASE, 'e');
    UARTCharPut(UART0_BASE, 'r');
    UARTCharPut(UART0_BASE, ' ');
    UARTCharPut(UART0_BASE, 'R');
    UARTCharPut(UART0_BASE, ',');
    UARTCharPut(UART0_BASE, ' ');
    UARTCharPut(UART0_BASE, 'G');
    UARTCharPut(UART0_BASE, ',');

    UARTCharPut(UART0_BASE, ' ');
    UARTCharPut(UART0_BASE, 'B');
    UARTCharPut(UART0_BASE, ' ');
    UARTCharPut(UART0_BASE, 'o');
    UARTCharPut(UART0_BASE, 'r');
    UARTCharPut(UART0_BASE, ' ');
    UARTCharPut(UART0_BASE, 'T');
    UARTCharPut(UART0_BASE, ':');
    UARTCharPut(UART0_BASE, '\r');

    while(1)
    {
        char cmd = UARTCharGet(UART0_BASE);

        if (cmd == 'T' || cmd =='t' )
        {
            // ADC FIFO data stored in array
            uint32_t ui32ADC0Value[4];

            // Variables for Average, Celsius and Fahrenheit Temperatures
            volatile uint32_t ui32TempAvg;
            volatile uint32_t ui32TempValueC;
            volatile uint32_t ui32TempValueF;
            // Variables used to convert to chars
            volatile uint32_t nF, nC;
            // Variables used to display the chars
            char tempF[2];
            char tempC[2];

            // ADC conversion complete when status flag is cleared
            ADCIntClear(ADC0_BASE, 1);
            // Trigger ADC conversion
            ADCProcessorTrigger(ADC0_BASE, 1);

            // Wait for end of conversion
            while(!ADCIntStatus(ADC0_BASE, 1, false))
            {
            }
            // Copy samples available in FIFO to buffer
            ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
            // Calculate Average, Celsius and Fahrenheit temperature
            ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
            ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
            ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;

            nF = ui32TempValueF; // Get temperature
            tempF[0] = nF/10 + 0x30; // Divide by 10 and add 48 to convert 1st int to char
            tempF[1] = ui32TempValueF%10 + 0x30; // Get remainder and add 48 to convert 2nd int to char

            nC = ui32TempValueC; // Get temperature
            tempC[0] = nC/10 + 0x30; // Divide by 10 and add 48 to convert 1st int to char
            tempC[1] = ui32TempValueC%10 + 0x30; // Get remainder and add 48 to convert 2nd int to char

            // Display Temperature in F and C
            UARTCharPut(UART0_BASE, 'T');
            UARTCharPut(UART0_BASE, 'e');
            UARTCharPut(UART0_BASE, 'm');
            UARTCharPut(UART0_BASE, 'p');
            UARTCharPut(UART0_BASE, ':');
            UARTCharPut(UART0_BASE, ' ');
            UARTCharPut(UART0_BASE, tempF[0]);
            UARTCharPut(UART0_BASE, tempF[1]);
            UARTCharPut(UART0_BASE, 'F');
            UARTCharPut(UART0_BASE, ' ');
            UARTCharPut(UART0_BASE, 'o');
            UARTCharPut(UART0_BASE, 'r');
            UARTCharPut(UART0_BASE, ' ');
            UARTCharPut(UART0_BASE, tempC[0]);
            UARTCharPut(UART0_BASE, tempC[1]);
            UARTCharPut(UART0_BASE, 'C');
        }
        // Turn ON Red LED with command 'R'
        else if (cmd == 'R')
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 2);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
            UARTCharPut(UART0_BASE, cmd);
        }
        // Turn ON Blue LED with command 'B'
        else if (cmd == 'B')
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
            UARTCharPut(UART0_BASE, cmd);
        }
        // Turn ON Green LED with command 'G'
        else if (cmd == 'G')
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 8);
            UARTCharPut(UART0_BASE, cmd);
        }
        // Turn OFF Red LED with command 'r'
        else if (cmd == 'r')
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
            UARTCharPut(UART0_BASE, cmd);
        }
        // Turn OFF Blue LED with command 'b'
        else if (cmd == 'b')
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
            UARTCharPut(UART0_BASE, cmd);
        }
        // Turn OFF Green LED with command 'g'
        else if (cmd == 'g')
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
            UARTCharPut(UART0_BASE, cmd);
        }

        // Display greeting after action
        UARTCharPut(UART0_BASE, '\r');
        UARTCharPut(UART0_BASE, '\n');
        UARTCharPut(UART0_BASE, 'E');
        UARTCharPut(UART0_BASE, 'n');
        UARTCharPut(UART0_BASE, 't');
        UARTCharPut(UART0_BASE, 'e');
        UARTCharPut(UART0_BASE, 'r');
        UARTCharPut(UART0_BASE, ' ');
        UARTCharPut(UART0_BASE, 'R');
        UARTCharPut(UART0_BASE, ',');
        UARTCharPut(UART0_BASE, ' ');
        UARTCharPut(UART0_BASE, 'G');
        UARTCharPut(UART0_BASE, ',');
        UARTCharPut(UART0_BASE, ' ');
        UARTCharPut(UART0_BASE, 'B');
        UARTCharPut(UART0_BASE, ' ');
        UARTCharPut(UART0_BASE, 'o');
        UARTCharPut(UART0_BASE, 'r');
        UARTCharPut(UART0_BASE, ' ');
        UARTCharPut(UART0_BASE, 'T');
        UARTCharPut(UART0_BASE, ':');
        UARTCharPut(UART0_BASE, '\r');
        UARTCharPut(UART0_BASE, '\n');
    }
}
