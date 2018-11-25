//*****************************************************************************
//  SENSOR CONTROLLER STUDIO EXAMPLE: ADC WINDOW MONITOR FOR LAUNCHPAD
//  Operating system: None
//
//  The Sensor Controller is used to sample a single ADC channel and monitor
//  the value. The Sensor Controller updates a bit-vector that indicates
//  whether the ADC value is:
//  - Below a configurable low threshold
//  - Above a configurable high threshold
//
//  The Sensor Controller notifies the application when the bit-vector changes
//  (triggering scTaskAlertCallback()), and the application sets the LEDs as
//  follows:
//  - Green LED is set whenever the ADC value is below the low threshold
//  - Red LED is set whenever the ADC value is above the high threshold
//
//  This example application does not implement power management.
//
//
//  Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************/
#include "ex_include.h"
#include "scif.h"


// Display error message if the SCIF driver has been generated with incorrect operating system setting
#ifndef SCIF_OSAL_NONE_H
    #error "SCIF driver has incorrect operating system configuration for this example. Please change to 'None' in the Sensor Controller Studio project panel and re-generate the driver."
#endif

// Display error message if the SCIF driver has been generated with incorrect target chip package
#ifndef SCIF_TARGET_CHIP_PACKAGE_QFN48_7X7_RGZ
    #error "SCIF driver has incorrect target chip package configuration for this example. Please change to 'QFN48 7x7 RGZ' in the Sensor Controller Studio project panel and re-generate the driver."
#endif


#define BV(n)           (1 << (n))
#define IOC_O_IOCFG(n)  (IOC_O_IOCFG0 + (sizeof(uint32_t) * n))


// Pin mapping for LaunchPad
#define IOID_GLED          IOID_7
#define IOID_RLED          IOID_6




void ctrlReadyCallback(void) {

} // ctrlReadyCallback




void taskAlertCallback(void) {

    // Clear the ALERT interrupt source
    scifClearAlertIntSource();

    // Indicate on LEDs whether the current ADC value is high and/or low
    if (scifTaskData.adcWindowMonitor.output.bvWindowState & SCIF_ADC_WINDOW_MONITOR_BV_ADC_WINDOW_LOW) {
        HWREG(GPIO_BASE + GPIO_O_DOUTSET31_0) = BV(IOID_GLED);
    } else {
        HWREG(GPIO_BASE + GPIO_O_DOUTCLR31_0) = BV(IOID_GLED);
    }
    if (scifTaskData.adcWindowMonitor.output.bvWindowState & SCIF_ADC_WINDOW_MONITOR_BV_ADC_WINDOW_HIGH) {
        HWREG(GPIO_BASE + GPIO_O_DOUTSET31_0) = BV(IOID_RLED);
    } else {
        HWREG(GPIO_BASE + GPIO_O_DOUTCLR31_0) = BV(IOID_RLED);
    }

    // Acknowledge the alert event
    scifAckAlertEvents();

} // taskAlertCallback




void main(void) {

    // Enable power domains
    PRCMPowerDomainOn(PRCM_DOMAIN_PERIPH);
    PRCMLoadSet();
    while (PRCMPowerDomainStatus(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON);

    // Enable the GPIO module
    PRCMPeripheralRunEnable(PRCM_PERIPH_GPIO);
    PRCMPeripheralSleepEnable(PRCM_PERIPH_GPIO);
    PRCMPeripheralDeepSleepEnable(PRCM_PERIPH_GPIO);
    PRCMLoadSet();
    while (!PRCMLoadGet());

    // Setup I/O for LEDs
    HWREG(GPIO_BASE + GPIO_O_DOE31_0) |= BV(IOID_GLED) | BV(IOID_RLED);
    HWREG(IOC_BASE + IOC_O_IOCFG(IOID_GLED)) = IOC_STD_OUTPUT | IOC_PORT_GPIO;
    HWREG(IOC_BASE + IOC_O_IOCFG(IOID_RLED)) = IOC_STD_OUTPUT | IOC_PORT_GPIO;

#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC13X2_V1) && !defined(DeviceFamily_CC13X2_V2) && !defined(DeviceFamily_CC26X2) && !defined(DeviceFamily_CC26X2_V1) && !defined(DeviceFamily_CC26X2_V2)
    // In this example, we keep the AUX domain access permanently enabled
    scifOsalEnableAuxDomainAccess();
#endif

    // Initialize and start the Sensor Controller
    scifOsalRegisterCtrlReadyCallback(ctrlReadyCallback);
    scifOsalRegisterTaskAlertCallback(taskAlertCallback);
    scifInit(&scifDriverSetup);
    scifStartRtcTicksNow(0x00010000 / 8);
    IntMasterEnable();
    AONRTCEnable();

    // Configure and start the ADC window monitor task
    scifTaskData.adcWindowMonitor.cfg.adcWindowHigh = 800;
    scifTaskData.adcWindowMonitor.cfg.adcWindowLow  = 400;
    scifStartTasksNbl(BV(SCIF_ADC_WINDOW_MONITOR_TASK_ID));

    // Main loop
    while (1) {
    }

} // main
