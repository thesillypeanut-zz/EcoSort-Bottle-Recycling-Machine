/*
 * File:   pin_manager.c
 * Author: Maliha Islam
 *
 * Created on February 27, 2017, 10:31 PM
 */


 /*Description:
    This header file provides implementations for pin APIs for all pins selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB® Code Configurator - v2.25.2
        Device            :  PIC18F26K20
        Driver Version    :  1.02
    The generated drivers are tested against the following:
        Compiler          :  XC8 v1.34
        MPLAB             :  MPLAB X v2.35 or v3.00
 */


#include <xc.h>
#include "pin_manager.h"

void PIN_MANAGER_Initialize(void) {
    //LATA = 0x00;
    //TRISA = 0xFF;

    //LATB = 0x00;
    //TRISB = 0xFA;
    WPUB = 0x00;

    //LATC = 0x00;
    //TRISC = 0xFF;

    //ANSEL = 0x1F; //00011111 PORT A,E

    //ANSELH = 0x0A; //00001010 PORT B 

    INTCON2bits.nRBPU = 0x01;


}
/**
 End of File
 */