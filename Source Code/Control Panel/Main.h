
#define _XTAL_FREQ      8000000

#define ACK             1
#define NAK             0

#define ACCESS_CFG      0xAC
#define START_CONV      0xEE
#define READ_TEMP       0xAA
#define CONT_CONV       0x02


#define FAN_EN_LED      PORTDbits.RD7
#define FAN_EN          PORTDbits.RD3
#define FAN_PWM         PORTCbits.RC2
#define RTC_ALARM_NOT   PORTAbits.RA4

#define Setup_Time_Key        	0x0C
#define Setup_Alarm_Key       	0x0D
#define Setup_Fan_Temp_Key    	0x0E
#define Toggle_Fan_Key      	0x05
#define Prev            		0x03
#define Next            		0x04
#define Minus           		0x06
#define Plus            		0x07
#define Ch_Minus                0x00
#define Ch_Plus                 0x02
#define Star            		?
#define Pound					?

#define ALARMLED_RED    PORTDbits.RD0
#define ALARMLED_GREEN  PORTDbits.RD1
#define ALARMLED_BLUE   PORTDbits.RD2


