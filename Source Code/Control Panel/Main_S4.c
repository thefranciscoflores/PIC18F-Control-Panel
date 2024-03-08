#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include <p18f4620.h>
#include <usart.h>
#include <string.h>

#include "I2C.h"
#include "I2C_Support.h"
#include "Interrupt.h"
#include "Fan_Support.h"
#include "Main.h"
#include "ST7735_TFT.h"
#include "utils.h"
#include "Main_Screen.h"
#include "Setup_Alarm_Time.h"
#include "Setup_Fan_Temp.h"
#include "Setup_Time.h"

#pragma config OSC = INTIO67
#pragma config BOREN =OFF
#pragma config WDT=OFF
#pragma config LVP=OFF
//#pragma config CCP2MX = PORTBE

void test_alarm();
char second = 0x00;
char minute = 0x00;
char hour = 0x00;
char dow = 0x00;
char day = 0x00;
char month = 0x00;
char year = 0x00;

char found;
char tempSecond = 0xff; 
signed int DS1621_tempC, DS1621_tempF;
char setup_second, setup_minute, setup_hour, setup_day, setup_month, setup_year;
char alarm_second, alarm_minute, alarm_hour, alarm_date;
char setup_alarm_second, setup_alarm_minute, setup_alarm_hour;
unsigned char fan_set_temp = 75;
unsigned char Nec_state = 0;
float volt;
char INT1_flag, INT2_flag;

short nec_ok;
unsigned long long Nec_code;
char FAN;
char duty_cycle;
int rps;
int rpm;
int ALARMEN;
int alarm_mode = 0;
int MATCHED,color;
char Nec_Button;



char buffer[38]     = " ECE3301L F'23 Final S4\0";
char *nbr;
char *txt;
char tempC[]        = "+25";
char tempF[]        = "+77";
char time[]         = "00:00:00";
char date[]         = "00/00/00";
char alarm_time[]   = "00:00:00";
char Alarm_SW_Txt[] = "OFF";
char Fan_Set_Temp_Txt[] = "075F";
char Fan_SW_Txt[]   = "OFF";                // text storage for Heater Mode
char array1[21]={0xA2,0x62,0xE2,0x22,0x02,0xC2,0xE0,0xA8,0x90,0x68,0x98,0xB0,0x30,0x18,0x7A,0x10,0x38,0x5A,0x42,0x4A,0x52};		//Acquired hex values of each button in order						 

    
char DC_Txt[]       = "000";                // text storage for Duty Cycle value
char Volt_Txt[]     = "0.00V";
char RTC_ALARM_Txt[]= "0";                      //
char RPM_Txt[]      = "0000";               // text storage for RPM

char setup_time[]       = "00:00:00";
char setup_date[]       = "00/00/00";
char setup_alarm_time[] = "00:00:00"; 
char setup_fan_set_text[]   = "075F";


void putch (char c)
{   
    while (!TRMT);       
    TXREG = c;
}

void init_UART()
{
    OpenUSART (USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, 25);
    OSCCON = 0x70;
}



void Do_Init()                      // Initialize the ports 
{ 
    init_UART();                    // Initialize the uart
    Init_ADC();                     // Initialize ADC
    OSCCON=0x70;                    // Set oscillator to 8 MHz 
    
    ADCON1= 0x0D;                   //ADCON1 is 0000 1101 ... AN1 AN0 as Analog
    TRISA = 0x13;                   // 0001 0011 bit 4,1, and 0 as input
    TRISB = 0x07;                   // 0000 0111 bit 2,1, and 0 as input
    TRISC = 0x01;                   // 0000 0001 bit 0 as input
    TRISD = 0x00;                   // PORTD as outputs
    TRISE = 0x00;                   // PORTE as output
    PORTE = 0x00;                   // TURN OFF PORTE to 0

    FAN = 0;                        //Initialize Fan to 0   
    RBPU=0;                         

    TMR3L = 0x00;                   //
    T3CON = 0x03;
    I2C_Init(100000);               //Initialize I2C module

    DS1621_Init();                  //Initialize DS1621 Module
    init_INTERRUPT();               //Initialize Interrupts
    Turn_Off_Fan();                 //Turn off Fan
    fan_set_temp = 75;              //Set Heater temp to 75 F
    
    ALARMLED_RED = 0;               //ALARM LED 0
    ALARMLED_GREEN = 0;
    ALARMLED_BLUE = 0;
}


void main() 
{
    Do_Init();                                                  // Initialization  
    Initialize_Screen();                                        //Initialize Screen
    DS3231_Turn_Off_Alarm();                                    //Turn off Alarm
    DS3231_Read_Alarm_Time();                                   // Read alarm time for the first time
    DS3231_Setup_Time();                                        //Initialize Time Setup 
    tempSecond = 0xff;                                          //Initialize var tempSecond as 0xFF
    
    while (1)
    {
        DS3231_Read_Time();                                             //Read time on clock
        Set_ADCON0(1);                                            //Set ADCON0 to Channel 1.
        if(tempSecond != second)
        {
            tempSecond = second;
            rpm = get_RPM();                                         //Get RPM
            
            volt = read_volt();                                     //Read Voltage
            DS1621_tempC = DS1621_Read_Temp();                      //Read temp in Celsius
            DS1621_tempF = (DS1621_tempC * 9 / 5) + 32;                 //Convert temp from celsius to F
            duty_cycle = get_duty_cycle(DS1621_tempF, fan_set_temp);    //Get Duty Cycle
            
            printf ("%02x:%02x:%02x %02x/%02x/%02x",hour,minute,second,month,day,year);     //Print All info into TeraTerm
            printf (" Temp = %d C = %d F ", DS1621_tempC, DS1621_tempF);
            printf ("alarm = %d match = %d", RTC_ALARM_NOT, MATCHED);
            printf ("RPM = %d  dc = %d ", rpm, duty_cycle);
            printf ("Volt = %f \r\n", volt);
     
            Set_RPM_RGB(rpm);               //Set the LED to the RPM registered
            Monitor_Fan();                  //Monitor the fan state, speed and cycle
            test_alarm();                   //Test Alarm Module
            Update_Screen();                //Update Screen every second
        }
        
        if(nec_ok == 1)                     //If Nec_ok is 1
        {
            nec_ok = 0;                     //Reset nec_ok to 0
            
            INTCONbits.INT0IE = 1;          // Enable external interrupt
            INTCON2bits.INTEDG0 = 0;        // Edge programming for INT0 falling edge
            found = 0xff;
            
            for (int j=0; j< 21; j++)           //Mask remote button to IR value captured
            {
                if (Nec_Button == array1[j]) 
                {
                    found = j;                      //Found equals IR hex value
                    j = 21;
                }
            }

            if (found == 0xff)                          //if Found equals to 0xFF
            {
                printf ("Cannot find button \r\n");     
                //return (0);
            }
            else
            {
                // add code to turn on KEY PRESSED LEDs
                PORTCbits.RC4 = 1;
                // generate beep tone
                Activate_Buzzer();
                // wait 1 second
                Wait_One_Sec();
                // remove beep tone
                Deactivate_Buzzer();
                // turn off KEY PRESSED
                PORTCbits.RC4 = 0;


     
                switch (found)                          //Found is valid when:
                {
                    case Setup_Time_Key:
     
                        Do_Beep_Good();                 //Beep 
                        Do_Setup_Time();                //Go into setup time mode
                        break;

                    case Setup_Alarm_Key:           
                        Do_Beep_Good();                 //Beep
                        Do_Setup_Alarm_Time();          //Go into Alarm setup mode
                        break;

                    case Setup_Fan_Temp_Key:
                        Do_Beep_Good();                 //Beep
                        Setup_Temp_Fan();               //Go into Fan Setup Mode      
                        break;

                    case Toggle_Fan_Key:
                        Do_Beep_Good();                 //Beep
                        Toggle_Fan_Monitor();           //Toggle Fan On or Off
                         break;           

                    default:
                        Do_Beep_Bad();                  //Beep bad if none of the previous options is clicked
                        break;
                }
            } 
        }
        
        if (INT1_flag == 1)                             //If the Interrupt 1 Flag is 1
        {
            INT1_flag = 0;                              //Reset Interrupt Flag to 0
			Toggle_Fan_Monitor();                       //Turn Fan On or Off (Toggle)
        }
		
		if (INT2_flag == 1)                             //IF the interrupt 2 flag is 1
        {
            INT2_flag = 0;                              //Reset Interrupt Flag to 0
            if (ALARMEN == 0) ALARMEN = 1;              //Toggle Alarm
            else ALARMEN = 0;			
           
        }
        
    }
}

void test_alarm()
{
    //Possible Events
    
    if(alarm_mode == 0 && ALARMEN == 1) // Alarm is turned ON but match is off
    {
        DS3231_Turn_On_Alarm();         //Turn ON Alarm
        alarm_mode = 1;                 //Alarm Mode as 1 to move into next setting
    }
    
    if(alarm_mode == 1 && ALARMEN == 0) //If Alarm mode is 1 but Alarm is 0 
    {
        DS3231_Turn_Off_Alarm();            //Turn Off Alarm
        PORTD = (PORTD & 0xF8) | 0x00;      //Reset LED to OFF
        alarm_mode = 0;                     //Turn Alarm Mode to 0
        Deactivate_Buzzer();                //Deactivate Buzzer
    }
    
    
    if(alarm_mode == 1 && ALARMEN == 1)     //IF Alarm Mode and ALARMEN is 1
    {
        if(RTC_ALARM_NOT == 0)              //If ~ALARM_MATCH is ENABLED
        {
            MATCHED = 1;                    
            Activate_Buzzer_500Hz();        //ACTIVATE BUZZER
        }
        if (MATCHED == 1)                   //GO TO NEXT FUNCTION
        {
            Set_RGB_Color(color++);             //TOGGLE BETWEEN ALL LED COLORS
            if(color > 7) color = 0;    
            volt = read_volt();
            if(volt > 2.5)                    //ONCE PHOTORESISTOR VOLTAGE GOES ABOVE 2.5V
            {
                MATCHED = 0;                        //SET MATCH TO 0
                PORTD = (PORTD & 0xF8) | 0x00;      //TURN OFF LED
                Deactivate_Buzzer();                //DEACTIVATE BUZZER
                do_update_pwm(duty_cycle);          //UPDATE PWM
                alarm_mode = 0;                     //SET ALARM MODE TO 0 BACK TO INIT CONDITION
            }
        }        
    }          
}



