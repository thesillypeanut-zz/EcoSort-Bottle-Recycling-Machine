
#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set IN1 aliases
#define IN1_TRIS               TRISB0
#define IN1_LAT                LATB0
#define IN1_PORT               PORTBbits.RB0
#define IN1_WPU                WPUB0
#define IN1_ANS                ANS12
#define IN1_SetHigh()    do { LATB0 = 1; } while(0)
#define IN1_SetLow()   do { LATB0 = 0; } while(0)
#define IN1_Toggle()   do { LATB0 = ~LATB0; } while(0)
#define IN1_GetValue()         PORTBbits.RB0
#define IN1_SetDigitalInput()    do { TRISB0 = 1; } while(0)
#define IN1_SetDigitalOutput()   do { TRISB0 = 0; } while(0)

#define IN1_SetPullup()    do { WPUB0 = 1; } while(0)
#define IN1_ResetPullup()   do { WPUB0 = 0; } while(0)
#define IN1_SetAnalogMode()   do { ANS12 = 1; } while(0)
#define IN1_SetDigitalMode()   do { ANS12 = 0; } while(0)
// get/set IN2 aliases
#define IN2_TRIS               TRISB2
#define IN2_LAT                LATB2
#define IN2_PORT               PORTBbits.RB2
#define IN2_WPU                WPUB2
#define IN2_ANS                ANS8
#define IN2_SetHigh()    do { LATB2 = 1; } while(0)
#define IN2_SetLow()   do { LATB2 = 0; } while(0)
#define IN2_Toggle()   do { LATB2 = ~LATB2; } while(0)
#define IN2_GetValue()         PORTBbits.RB2
#define IN2_SetDigitalInput()    do { TRISB2 = 1; } while(0)
#define IN2_SetDigitalOutput()   do { TRISB2 = 0; } while(0)

#define IN2_SetPullup()    do { WPUB2 = 1; } while(0)
#define IN2_ResetPullup()   do { WPUB2 = 0; } while(0)
#define IN2_SetAnalogMode()   do { ANS8 = 1; } while(0)
#define IN2_SetDigitalMode()   do { ANS8 = 0; } while(0)

/**
 * @Param
    none
 * @Returns
    none
 * @Description
    GPIO and peripheral I/O initialization
 * @Example
    PIN_MANAGER_Initialize();
 */
void PIN_MANAGER_Initialize(void);

/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handling routine
 * @Example
    PIN_MANAGER_IOC();
 */
void PIN_MANAGER_IOC(void);

#endif // PIN_MANAGER_H
/**
 End of File
 */