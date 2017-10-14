/*
 * File:   main.c
 * Author: Maliha Islam
 * Description: Complete code for a bottle sorting machine that sorts four types
 *              bottles: YOP with/without cap, ESKA with/ without cap;
 *              machine interface and operation code
 * Reference Sources:
 *  DC motor actuation/PWM: https://electrosome.com/dc-motor-l293d-pic-microcontroller/
 *  Keypad + LCD interface: Sample code provided by Instructor 
 *  Servo PWM: https://electrosome.com/servo-motor-pic-microcontroller/:
 *  RTC: Sample code provided by Instructor
 *  EEPROM read/write: http://stackoverflow.com/questions/38436675/pic18f2520-mplab-x-xc8-eeprom
 *  Analog/Digital I/O: Sample code provided by Instructor
 *  Keypad Interrupts: Sample code provided by Instructor
 * Created on February 18, 2017, 12:11 PM
 */


// <editor-fold defaultstate="collapsed" desc=" VARIABLES, CONSTANTS AND DECLARATIONS ">
#include <xc.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "I2C.h"
#include "macros.h"

//void set_time(void);
void delay(int);
void standby_mode(unsigned char[]);
void update_time(unsigned char[]);
void print(char[], char[]);
void operation(void);
int time_diff(unsigned char[], unsigned char[]);
void elapsed_time(void);
int dec2hex(int);
void wait(void);
void key_release(void);
void opentoplid(void);
void closetoplid(void);
void openbotlid(void);
void closebotlid(void);
void readADC(char channel);
void lcdclear(void);
uint8_t read_eeprom(uint16_t address);
void write_eeprom(uint16_t address, uint8_t data);

const char keys[] = "123A456B789C*0#D"; 
const char curr_time[7] =    {0x00, //50 Seconds 
                        0x47, //13 Minutes
                        0x00, //24 hour mode, set to 21:00
                        0x02, //Wednesday 
                        0x11, //22nd
                        0x04, //February
                        0x17};//2017

unsigned char time[7];
unsigned char start_time[7];
unsigned char end_time[7];
int flag, flag1, flag2, flag3, flag4, flag5;
int yop_a = 0, yop_b = 0, eska_a = 0, eska_b = 0;
int totalbot = 0;
int run = 0;
unsigned char pressed;

//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" BOTTLE CATEGORIES ">
enum category {                        
    YOP,
    ESKA,
    CAP,
    NOCAP
};
enum category brand;
enum category type;

//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" STATES ">
enum state {                        
    STANDBY,
    OPERATION,
    END,
    LOG
};  
enum state curr_state = STANDBY;
//</editor-fold>

void main(void) {
    
    // <editor-fold defaultstate="collapsed" desc=" STARTUP SEQUENCE ">
    
    TRISA = 0b11011110; // Set Port A as all input, except RA5 (top lid) and RA0 (test LED)
    TRISB = 0b11111010; // Set Port B as all input, except RB0 and RB2 (DC motor)
    TRISC = 0b01011001; // Set Port C as all output, except RC3 and RC4 (I2C), RC0 (laser detector), RC6(IR detector)
    TRISD = 0x00; // Set Port D as all output
    TRISE = 0x00; // Set Port E as all output  

    LATA = 0x00;
    LATB = 0x00; 
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;
    
    ADCON0 = 0x00;  //Disable ADC
    ADCON1 = 0x0B;  //AN0 to AN3 used as analog input  
    CVRCON = 0x00; // Disable CCP reference voltage output
    CMCONbits.CIS = 0;
    ADFM = 1;
    
    RB0 = 1;        //RB0 is motor IN1
    RB2 = 1;        //RB2 is motor IN2, both high stops motor
    
    nRBPU = 0;
    
    I2C_Master_Init(10000); //Initialize I2C Master with 100KHz clock
    
    INT1IE = 1;
    ei(); // Enable all interrupts
    
    initLCD();
    // <editor-fold defaultstate="collapsed" desc=" WRITING TO EEPROM ">
                          
                    write_eeprom(0x30,0x0);
                    write_eeprom(0x31,0x0);
                    write_eeprom(0x32,0x0);
                    write_eeprom(0x33,0x0);
                
                
                    write_eeprom(0x20,0x0);
                    write_eeprom(0x21,0x0);
                    write_eeprom(0x22,0x0);
                    write_eeprom(0x23,0x0);
                
    
                    write_eeprom(0x10,0x0);
                    write_eeprom(0x11,0x0);
                    write_eeprom(0x12,0x0);
                    write_eeprom(0x13,0x0);                    
                

                    write_eeprom(0x00,0x0);
                    write_eeprom(0x01,0x0);
                    write_eeprom(0x02,0x0);
                    write_eeprom(0x03,0x0);                    
                
    //</editor-fold>
    //</editor-fold>

    while (1){
        //update_time(time);
        //set_time();
        
        switch(curr_state){
            case STANDBY:
                flag = 0, flag1 = 0, flag2 = 0, flag3 = 0, flag4 = 0, flag5 = 0;
                lcdclear();
                __lcd_home();
                while(keys[(PORTB & 0xF0)>>4] != '0'){ 
                    // RB1 is the interrupt pin, so if there is no key pressed, RB1 will be 0
                    // the PIC will be in standby mode until a key press is signaled
                    standby_mode(time);                                             //SCREEN 1: Time and Date
                }
                key_release();
                
                while (!flag){
                    lcdclear();
                    print("*: Start     |<C","1: Past Logs |D>");                   //SCREEN 2: Menu Part 1 (Start + Past Logs)
                    wait();
                    key_release();

                    if (pressed == '*'){     
                        // <editor-fold defaultstate="collapsed" desc=" OPERATION START SCREEN ">
                        yop_a = 0;
                        yop_b = 0;
                        eska_a = 0;
                        eska_b = 0;
                        totalbot = 0;
                        if (run == 5){
                            run = 1;
                        }
                        else{
                            run++;
                        }

                        lcdclear();
                        print("    Sorting    ","   has begun!   ");               //SCREEN 2A: Starting Operation 
                        delay(2);

                        lcdclear();
                        print("Sorting...        ","Press # to STOP   ");
                        update_time(start_time);
                        update_time(end_time);
                        curr_state = OPERATION;
                        //</editor-fold> 
                        break;
                    }
                    else if (pressed == '1'){ 
                        flag1=0;
                        while(!flag1){
                            lcdclear();
                            print("4: Run 1     |<C","5: Run 2     |D>");           //SCREEN 2B1: Past Logs Part 1 (Run 1 + Run 2)
                            wait();
                            key_release();
                            if (pressed == '4'){
                                // <editor-fold defaultstate="collapsed" desc=" RUN 1 SCREEN ">
                                lcdclear();
                                printf("YOP A: %i B: %i|<C",(int)read_eeprom(0x30),(int)read_eeprom(0x31));   //SCREEN 2B1i: Run 1 Bottle Count
                                __lcd_newline();
                                printf("ESK A: %i B: %i   ",(int)read_eeprom(0x32),(int)read_eeprom(0x33));
                                wait();
                                key_release();
                                //</editor-fold>   
                            }
                            else if (pressed == '5'){
                                // <editor-fold defaultstate="collapsed" desc=" RUN 2 SCREEN ">
                                lcdclear();
                                printf("YOP A: %i B: %i|<C",read_eeprom(0x20),read_eeprom(0x21));   //SCREEN 2B1ii: Run 2 Bottle Count
                                __lcd_newline();
                                printf("ESK A: %i B: %i   ",read_eeprom(0x22),read_eeprom(0x23));
                                wait();
                                key_release();
                                //</editor-fold>
                            }
                            else if (pressed == 'D'){
                                flag2= 0;
                                while(!flag2){
                                    lcdclear();
                                    print("6: Run 3     |<C","7: Run 4         ");           //SCREEN 2B2: Past Logs Part 2 (Run 3 + Run 4)
                                    wait();
                                    key_release();
                                    if (pressed == '6'){
                                        // <editor-fold defaultstate="collapsed" desc=" RUN 3 SCREEN ">
                                        lcdclear();
                                        printf("YOP A: %i B: %i|<C",(int)read_eeprom(0x10),(int)read_eeprom(0x11));   //SCREEN 2B2i: Run 3 Bottle Count
                                        __lcd_newline();
                                        printf("ESK A: %i B: %i   ",(int)read_eeprom(0x12),(int)read_eeprom(0x13));
                                        wait();
                                        key_release();
                                        //</editor-fold>  
                                    }
                                    else if (pressed == '7'){
                                        // <editor-fold defaultstate="collapsed" desc=" RUN 4 SCREEN ">
                                        lcdclear();
                                        printf("YOP A: %i B: %i|<C",(int)read_eeprom(0x00),(int)read_eeprom(0x01));   //SCREEN 2B2ii: Run 4 Bottle Count
                                        __lcd_newline();
                                        printf("ESK A: %i B: %i   ",(int)read_eeprom(0x02),(int)read_eeprom(0x03));
                                        wait();
                                        key_release();        
                                        //</editor-fold>  
                                    }
                                    else if (pressed == 'C'){
                                        flag2 = 1;
                                    }
                                }
                            }
                            else if (pressed == 'C'){
                                flag1 = 1;
                            }
                        }
                    }
                    else if (pressed == 'C'){
                        curr_state = STANDBY;
                        break;
                    }
                    else if (pressed == 'D'){
                        flag3=0;
                        while (!flag3){
                            lcdclear();
                            print("2: About     |<C","3: Credits         ");                     //SCREEN 3: MENU PART 2 (About + Credits)
                            wait();
                            key_release();  
                            if (pressed == '2'){
                                // <editor-fold defaultstate="collapsed" desc=" ABOUT SCREEN ">
                                flag4=0;
                                while (!flag4){                              
                                    lcdclear();
                                    print("EcoSort      |<C","Version: v1.0|D>");           //SCREEN 3A1: About Part 1
                                    wait();
                                    key_release();
                                    if (pressed == 'C'){
                                        flag4 = 1;
                                    }
                                    else if (pressed == 'D'){
                                        lcdclear();
                                        print("Copyright(C)2017","InnovaTech   |<C");        //SCREEN 3A2: About Part 2
                                        wait();
                                        key_release();
                                      
                                    }
                                }
                                //</editor-fold>
                            }
                            else if (pressed == '3'){
                                // <editor-fold defaultstate="collapsed" desc=" CREDITS SCREEN ">
                                flag4=0;
                                while(!flag4){
                                    lcdclear();
                                    print("Mechanics:   |<C","R. Jose      |D>");           //SCREEN 3B1: Mechanics Credits
                                    wait();
                                    key_release();
                                    if (pressed == 'C'){
                                        flag4 = 1;
                                    }
                                    else if (pressed == 'D'){
                                        flag5=0;
                                        while(!flag5){
                                            lcdclear();
                                            print("Circuitry:   |<C","J. Qiu       |D>");   //SCREEN 3B2: Circuitry Credits
                                            wait();
                                            key_release();
                                            if (pressed == 'D'){
                                                lcdclear();
                                                print("Programming: |<C","M. Islam        ");       //SCREEN 3B3: Programming Credits
                                                wait();
                                                key_release();
                                            }
                                            else if (pressed == 'C'){
                                                 flag5=1;
                                            }
                                        }
                                    }
                                }
                                //</editor-fold>
                            }   
                            else if (pressed == 'C'){
                                flag3 = 1;
                            }
                        }
                    }
                }                
                
            case OPERATION:
                //update_time(start_time);
                    
                while ((time_diff(start_time,end_time)<180 || totalbot < 10) && curr_state == OPERATION){ 
                    if (curr_state == OPERATION){
                        closebotlid();
                        opentoplid();
                    }
                    readADC(3); //reads analog input from RA3 (IR TCRT5000L) on delivery arm
                    
                    while ((ADRESH*256 + ADRESL > 0x19))
                    {    
                        readADC(3);
                        RC7 = 1;
                        __delay_us(2000);
                        RC7 = 0;
                        __delay_us(18000);
                    }
                                            
                    unsigned int counter = 0;
                    while (RC0 && curr_state==OPERATION){ 
                        
                        //wait here until laser detector (RC0) detects a bottle in the 
                        //testing chamber; 1 = no bottle, 0 = bottle
                        
                        RB0 = 0;        // motor start
                        for (unsigned int i = 0; i<31; i++){    //PWM for motor
                            PORTEbits.RE0 = 1;
                            __delay_us(18300);
                            PORTEbits.RE0 = 0;
                            __delay_us(1700);
                            
                        }
                        RB0 = 1;                 //motor stop
                        __delay_ms(3000);
                        counter++;               
                        if ((counter == 5) && RC0){      //foolproof for Eska not detected by the laser beam
                            openbotlid();
                            delay(1);
                            closebotlid();
                        }
                        if ((yop_a>=1) && (yop_b>=1) && (eska_a>=1) && (eska_b>=1) && (counter==10)){ //loading bin stops moving after 10 cycles if at least 4 bottles has been sorted
                            update_time(end_time);
                            curr_state = END;
                            counter = 0;
                            break;
                        }
                   
                        
                    }
                    
                    if (curr_state == OPERATION){
                        closetoplid();
                    }
                    
                    // <editor-fold defaultstate="collapsed" desc=" IDENTIFICATION ALGORITHM ">
                    
                    readADC(2); //reads analog input from RA2 (IR TCRT5000) on testing chamber
                    
                    if (RC6){ //RC6 is IR detector, input 1 for opaque, 0 for clear
                        brand = YOP;
                    }
                    else if (!RC6){
                        brand = ESKA;
                    }
                    
                    if (brand == YOP && RA4 == 0 && (ADRESL < 0x14)){
                        //RA4 is IR TCRT5000 module, RA2 is IR TCRT5000
                        type = CAP;
                    }
                    else if (brand == ESKA && (RA4 == 0 || (ADRESL < 0x14))){                   
                        type = CAP;
                    }
                    else{
                        type = NOCAP;
                    }
                    //</editor-fold>

                    // <editor-fold defaultstate="collapsed" desc=" DELIVERY CHUTE ROTATION ">
                    
                    //RC7 is the pin for continuous servo
                    
                    
                    if (brand == ESKA && type == CAP && curr_state == OPERATION){
                        openbotlid();
                    
                        eska_a++;                            //add to ESKA A count
                    }
                    else if (brand == YOP && type == CAP && curr_state == OPERATION){
                    // <editor-fold defaultstate="collapsed" desc=" GO TO 90 DEGREES ">
                        unsigned int i;
                        for(i=0;i<5;i++)                   //rotate chute to 90 degrees
                        {
                        
                          RC7 = 1;
                          __delay_us(2300);
                          RC7 = 0;
                          __delay_us(17700);
                        }
                        openbotlid();

                    //</editor-fold>
                        yop_a++;                            //add to YOP A count
                    }

                    else if (brand == YOP && type == NOCAP && curr_state == OPERATION){
                    // <editor-fold defaultstate="collapsed" desc=" GO TO 180 DEGREES ">
                        unsigned int i;
                        for(i=0;i<13;i++)                   //rotate chute to 180 degrees
                        {
                        
                          RC7 = 1;
                          __delay_us(2300);
                          RC7 = 0;
                          __delay_us(17700);
                        }
                        openbotlid();
 
                    //</editor-fold>
                        yop_b++;                           //add to YOP B count
                    }

                    else if (brand == ESKA && type == NOCAP && curr_state == OPERATION){
                    // <editor-fold defaultstate="collapsed" desc=" GO TO 270 DEGREES ">
                        unsigned int i;
                        for(i=0;i<21;i++)                   //rotate chute to 270 degrees
                        {
                        
                          RC7 = 1;
                          __delay_us(2300);
                          RC7 = 0;
                          __delay_us(17700);
                        }
                        openbotlid();

                    //</editor-fold>
                        eska_b++;                           //add to ESKA B count
                    }
                    //</editor-fold>
                    
                    totalbot++;
                    if (totalbot == 10){
                        update_time(end_time);
                        curr_state = END;
                        break;
                    }

                    update_time(end_time);
                }
                
                if (curr_state != OPERATION){ 
                    update_time(end_time);
                    break;
                }
                update_time(end_time);
                curr_state = END;
                break;
                
            case END:
                //update_time(end_time);
                RB0=1; //stop motor
                RC7=0; //stop cts servo
                opentoplid();
                closebotlid();

                lcdclear();
                print(" Operation has  ","   terminated   ");
                delay(2);
                RA0 = 1; //DEBUG
                curr_state = LOG;
                break; 
                
            case LOG:
                flag = 0, flag2 = 0;
                
                // <editor-fold defaultstate="collapsed" desc=" WRITING TO EEPROM ">
                if (run == 1){          
                    write_eeprom(0x30,(uint8_t)yop_a);
                    write_eeprom(0x31,(uint8_t)yop_b);
                    write_eeprom(0x32,(uint8_t)eska_a);
                    write_eeprom(0x33,(uint8_t)eska_b);
                }
                else if (run == 2){
                    write_eeprom(0x20,(uint8_t)yop_a);
                    write_eeprom(0x21,(uint8_t)yop_b);
                    write_eeprom(0x22,(uint8_t)eska_a);
                    write_eeprom(0x23,(uint8_t)eska_b);
                }
                else if (run == 3){
                    write_eeprom(0x10,(uint8_t)yop_a);
                    write_eeprom(0x11,(uint8_t)yop_b);
                    write_eeprom(0x12,(uint8_t)eska_a);
                    write_eeprom(0x13,(uint8_t)eska_b);                    
                }
                else if (run == 4) {
                    write_eeprom(0x00,(uint8_t)yop_a);
                    write_eeprom(0x01,(uint8_t)yop_b);
                    write_eeprom(0x02,(uint8_t)eska_a);
                    write_eeprom(0x03,(uint8_t)eska_b);                    
                }
                //</editor-fold>
                
                while(!flag && curr_state == LOG){
                    lcdclear();
                    print("  Collect Log?  ", "< B Home|Yes A >");

                    wait();
                    key_release();
                    if (pressed == 'A'){
                        flag2=0;
                        while (!flag2){
                            lcdclear();
                            printf("YOP A: %i B: %i|<C",yop_a,yop_b);   
                            __lcd_newline();
                            printf("ESK A: %i B: %i|D>",eska_a, eska_b);
                            wait();
                            key_release();
                            if (pressed == 'D'){
                                totalbot = eska_a + eska_b + yop_a + yop_b;
                                lcdclear();
                                printf("Total: %i    |<C",totalbot);
                                __lcd_newline();
                                printf("Time: %is       ",time_diff(start_time,end_time)-2);
                                wait();
                                key_release();
                            }
                            else if (pressed == 'C'){
                                flag2 = 1;
                            }
                        }
                    }
                }
                break;
        }
    }

    return;
}


// <editor-fold defaultstate="collapsed" desc=" lcdclear FUNCTION ">
void lcdclear(void){
    __lcd_clear();
    __delay_ms(2);
}
//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" set_time FUNCTION ">
void set_time(void){
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    for(char i=0; i<7; i++){
        I2C_Master_Write(curr_time[i]);
    }    
    I2C_Master_Stop(); //Stop condition   
}
//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" standby_mode FUNCTION ">
void standby_mode(unsigned char time[]){
    update_time(time);
    di();
    printf("%02x/%02x/%02x MENU:", time[6],time[5],time[4]);    //Print date in YY/MM/DD
    __lcd_newline();
    printf("%02x:%02x:%02x Press 0", time[2],time[1],time[0]);    //HH:MM:SS
    ei();
}    
//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" update_time FUNCTION ">
void update_time(unsigned char time[]){
    //Reset RTC memory pointer 
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    I2C_Master_Stop(); //Stop condition

    //Read Current Time
    I2C_Master_Start();
    I2C_Master_Write(0b11010001); //7 bit RTC address + Read
    for(unsigned char i=0;i<0x06;i++){
        time[i] = I2C_Master_Read(1);
    }
    time[6] = I2C_Master_Read(0);       //Final Read without ack
    I2C_Master_Stop();
}
//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" time_diff FUNCTION ">
int time_diff(unsigned char time1[], unsigned char time2[]) {
    int hr1, hr2, min1, min2, s1, s2;
    int d1, d2, d3;
    hr1 = time1[2]; hr2 = time2[2]; min1 = time1[1]; min2 = time2[1]; 
    s1 = time1[0]; s2 = time2[0];
    
    d1 = dec2hex(hr2) - dec2hex(hr1);
    d2 = dec2hex(min2) - dec2hex(min1);
    d3 = dec2hex(s2) - dec2hex(s1);
    
    return 3600*d1 + 60*d2 + d3;
}
//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" delay FUNCTION ">
void delay(int seconds) {
    for (int i = 0; i <= seconds; i ++) {
        __delay_1s();
    }
}
//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" dec2hex FUNCTION ">
int dec2hex(int num) {
    int i = 0, quotient = num, temp, hexnum = 0;
 
    
    while (quotient != 0) {
        temp = quotient % 16;
        
        hexnum += temp*pow(10, i);
        
        quotient = quotient / 16;
        i += 1;
    }
    return hexnum;
}
//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" wait FUNCTION ">
void wait(void){
    while(PORTBbits.RB1 == 0){
        // Wait until a key is pressed
    }
}
//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" key_release FUNCTION ">
void key_release(void){
   
    while(PORTBbits.RB1 == 1){
        // Wait until the key has been released
        pressed = keys[(PORTB & 0xF0)>>4];
    }
    Nop();  //Apply breakpoint here because of compiler optimizations
    Nop();
}
//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" print FUNCTION ">
void print(char line1[], char line2[]) {
    if (line2 == ""){
        printf(line1);
    }
    else{
        printf(line1);
        __lcd_newline();
        printf(line2);
    }
}
//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" readADC FUNCTION ">
void readADC(char channel){
    // Select A2D channel to read
    ADCON0 = ((channel <<2));
    ADON = 1;
    ADCON0bits.GO = 1;
   while(ADCON0bits.GO_NOT_DONE){__delay_ms(5);}   
}
//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" KEYPAD INTERRUPTS ">
void interrupt keypressed(void) {           
    if(INT1IF){
        switch(keys[(PORTB & 0xF0) >> 4]){
            case '#':
                update_time(end_time);
                RC7=0; //stop cts servo
                RB0=1; //stop motor
                closebotlid();
                opentoplid();
                lcdclear();
                print(" Operation has  ","   terminated   ");
                delay(1);
                RA0 = 1;
                curr_state = LOG;
                break; 

            case 'B':
                curr_state = STANDBY;
                break;
            
            default:
                break;
        }
        INT1IF = 0;     //Clear flag bit
    }
}
//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" 180 SERVO FUNCTIONS ">
void opentoplid(void) 
{
    unsigned int i;
        for(i=0;i<50;i++)
        {
          PORTAbits.RA5 = 1;
          __delay_us(1200);
          PORTAbits.RA5 = 0;
          __delay_us(18800);
        }
}

void closetoplid(void) 
{
    unsigned int i;
        for(i=0;i<50;i++)
        {
          PORTAbits.RA5 = 1;
          __delay_us(2700);
          PORTAbits.RA5 = 0;
          __delay_us(17300);
        }
}

void openbotlid(void) 
{
  unsigned int i;
  for(i=0;i<50;i++)
  {
    RC5 = 1;
    __delay_us(1250);
    RC5 = 0;
    __delay_us(18750);
  }
}

void closebotlid(void) 
{
  unsigned int i;
  for(i=0;i<50;i++)
  {
    RC5 = 1;
    __delay_us(2700);
    RC5 = 0;
    __delay_us(17300);
  }
}
//</editor-fold>

// <editor-fold defaultstate="collapsed" desc=" EEPROM R/W FUNCTIONS ">
uint8_t read_eeprom(uint16_t address)
{

    // Set address registers
    EEADRH = (uint8_t)(address >> 8);
    EEADR = (uint8_t)address;

    EECON1bits.EEPGD = 0;       // Select EEPROM Data Memory
    EECON1bits.CFGS = 0;        // Access flash/EEPROM NOT config. registers
    EECON1bits.RD = 1;          // Start a read cycle

    // A read should only take one cycle, and then the hardware will clear
    // the RD bit
    while(EECON1bits.RD == 1);

    return EEDATA;              // Return data

}

void write_eeprom(uint16_t address, uint8_t data)
{    
    // Set address registers
    EEADRH = (uint8_t)(address >> 8);
    EEADR = (uint8_t)address;

    EEDATA = data;          // Write data we want to write to SFR
    EECON1bits.EEPGD = 0;   // Select EEPROM data memory
    EECON1bits.CFGS = 0;    // Access flash/EEPROM NOT config. registers
    EECON1bits.WREN = 1;    // Enable writing of EEPROM (this is disabled again after the write completes)

    // The next three lines of code perform the required operations to
    // initiate a EEPROM write
    EECON2 = 0x55;          // Part of required sequence for write to internal EEPROM
    EECON2 = 0xAA;          // Part of required sequence for write to internal EEPROM
    EECON1bits.WR = 1;      // Part of required sequence for write to internal EEPROM

    // Loop until write operation is complete
    while(PIR2bits.EEIF == 0)
    {
        continue;   // Do nothing, are just waiting
    }

    PIR2bits.EEIF = 0;      //Clearing EEIF bit (this MUST be cleared in software after each write)
    EECON1bits.WREN = 0;    // Disable write (for safety, it is re-enabled next time a EEPROM write is performed)
}
//</editor-fold>