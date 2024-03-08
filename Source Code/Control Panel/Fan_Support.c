#include <p18f4620.h>
#include "Main.h"
#include "Fan_Support.h"
#include "stdio.h"
#include "utils.h"
char FAN;
extern char duty_cycle;


int get_RPM()
{
    int RPS = TMR3L / 2;        //read the count. Since there are 2 pulses per rev
    
    TMR3L = 0;                  // clear out the count
 
    return (RPS * 60);          //return RPM = 60*RPS
}

void Toggle_Fan()
{
    FAN ^= 1;
    if(FAN == 1) Turn_On_Fan();
    else Turn_Off_Fan();
}

void Turn_Off_Fan()
{
    FAN = 0;
    FAN_EN = 0;
    FAN_EN_LED = 0;

}

void Turn_On_Fan()
{
    FAN = 1;
    //do_update_pwm(100);
    FAN_EN = 1;
    FAN_EN_LED = 1;

}

void Increase_Speed()
{
    if(duty_cycle == 100)
    {
        Do_Beep();
        Do_Beep();
        do_update_pwm(100);
    }else
    {
        duty_cycle = duty_cycle + 5;
        do_update_pwm(duty_cycle);
    }
}

void Decrease_Speed()
{
    if(duty_cycle == 0)
    {
        Do_Beep();
        Do_Beep();
        do_update_pwm(0);
    }else
    {
        duty_cycle = duty_cycle - 5;
        do_update_pwm(duty_cycle);
    }
 
}

/*void do_update_pwm(char duty_cycle) 
{ 
    float dc_f;
    int dc_I;
    PR2 = 0b00000100 ; // set the frequency for 25KHZ
    T2CON = 0b00000111 ; //
    dc_f = ( 4.0 * duty_cycle / 20.0) ; // calculate factor of duty cycle versus a 25Khz
    // signal
    dc_I = (int) dc_f; // get the integer part
    if (dc_I > duty_cycle) dc_I++; // round up function
    CCP1CON = ((dc_I & 0x03) << 4) | 0b00001100;
    CCPR1L = (dc_I) >> 2;

} OLD FUNCTION */

void Set_DC_RGB(int duty_cycle)
{
      char color = (duty_cycle / 10) ;
    if (color >=7){
        color = 7;
    }
    
    PORTD = (PORTD & 0xF8) | color;
}

void Set_RPM_RGB(int rpm)
{
    if(rpm == 0)
        PORTE = (PORTE & 0xF8) | 0x00;
    else if (rpm < 500)
        PORTE = (PORTE & 0xF8) | 0x01;
    else{
 
     char color = (rpm / 500)+1 ;
    // Ensure color is within the valid range
    PORTE = (PORTE & 0xF8) | color;
    }
}


void Toggle_Fan_Monitor()
{
    Toggle_Fan();
    
}

void Monitor_Fan()
{
    if(FAN == 0)
    {
        Turn_Off_Fan();
        duty_cycle = 0;
               
    }
    else Turn_On_Fan();
    do_update_pwm(duty_cycle);
    
}