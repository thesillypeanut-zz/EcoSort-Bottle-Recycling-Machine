/*
 * File:   mcc.c
 * Author: Maliha Islam
 *
 * Created on February 27, 2017, 10:26 PM
 */


#include <xc.h>
#include "mcc.h"
#include "configBits.h"

void SYSTEM_Initialize(void) {
    OSCILLATOR_Initialize();
    PIN_MANAGER_Initialize();
}

void OSCILLATOR_Initialize(void) {
    // SCS INTOSC; IRCF 8MHz_HFINTOSC/2; IDLEN disabled; 
    OSCCON = 0x62;
    // INTSRC disabled; PLLEN disabled; TUN 0x00; 
    OSCTUNE = 0x00;
    // Set the secondary oscillator
}