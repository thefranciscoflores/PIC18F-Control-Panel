#include <xc.h>
#include <p18f4620.h>
#include <stdio.h>
#include "Interrupt.h"

unsigned char bit_count;
unsigned int Time_Elapsed;

unsigned long long Nec_code;
unsigned char Nec_State = 0;

char Nec_Button;
extern short nec_ok;

extern char INT0_flag, INT1_flag, INT2_flag;

void init_INTERRUPT()
{
    TMR1H = 0;                              // Reset Timer1
    TMR1L = 0;                              //
	PIR1bits.TMR1IF = 0;                    // Clear timer 1 interrupt flag
    PIE1bits.TMR1IE = 1;                    // Enable Timer 1 interrupt

    INTCON2bits.INTEDG0=0 ; 				// INT0 EDGE falling
    INTCON2bits.INTEDG1=0; 					// INT1 EDGE falling
    INTCON2bits.INTEDG2=0; 					// INT2 EDGE falling
    INTCONbits.INT0IF = 0 ; 				// Clear INT0IF 
    INTCON3bits.INT1IF = 0; 				// Clear INT1IF 
    INTCON3bits.INT2IF =0; 					// Clear INT2IF 


    INTCONbits.INT0IE =1; 					// Set INT0 IE 
    INTCON3bits.INT1IE=1; 					// Set INT1 IE 
    INTCON3bits.INT2IE=1; 					// Set INT2 IE

    INTCONbits.PEIE = 1;                    // Enable Peripheral interrupt
    INTCONbits.GIE = 1;                     // Enable global interrupts
}

void interrupt high_priority chkisr() 
{
	if (PIR1bits.TMR1IF == 1) TIMER1_ISR();
    if (INTCONbits.INT0IF == 1) INT0_ISR(); 	//check if INT0 has occurred  
    if (INTCON3bits.INT1IF == 1) INT1_ISR(); 	//check if INT1 has occurred
    if (INTCON3bits.INT2IF == 1) INT2_ISR(); 	//check if INT2 has occurred
}


void INT1_ISR() 
{    
    INTCON3bits.INT1IF=0; 
    INT1_flag = 1; 								// set software INT1_flag
}

void INT2_ISR() 
{    
    INTCON3bits.INT2IF=0; 						// Clear the interrupt flag
    INT2_flag = 1; 								// set software INT2_flag
}

void Enable_INT_Interrupt()
{
    INTCONbits.INT0IE = 1;          			// Enable external interrupt
    INTCON2bits.INTEDG0 = 0;        			// Edge programming for INT0 falling edge
}    


void TIMER1_ISR(void)
{
    Nec_State = 0;                          	// Reset decoding process
    INTCON2bits.INTEDG0 = 0;                	// Edge programming for INT falling edge
    T1CONbits.TMR1ON = 0;                   	// Disable T1 Timer
    PIR1bits.TMR1IF = 0;                    	// Clear interrupt flag
}

void Reset_Nec_State()
{
    Nec_State=0;
    T1CONbits.TMR1ON = 0;
}

void INT0_ISR (void)
{
    INTCONbits.INT0IF = 0;                       // Clear external interrupt INT2IF
    if (Nec_State != 0)
    {
        Time_Elapsed = (TMR1H << 8) | TMR1L;	// Store Timer1 value
        TMR1H = 0;                          	// Reset Timer1
        TMR1L = 0;
    }
    
    switch(Nec_State)
    {
        case 0 :
        {
            // Clear Timer 1
            TMR1H = 0;                      // Reset Timer1
            TMR1L = 0;                      //
            PIR1bits.TMR1IF = 0;            //
            T1CON= 0x90;                    // Program Timer1 mode with count = 1usec using System clock running at 8Mhz
            T1CONbits.TMR1ON = 1;           // Enable Timer 1
            bit_count = 0;                  // Force bit count (bit_count) to 0
            Nec_code = 0;                   // Set Nec_code = 0
            Nec_State = 1;                  // Set Nec_State to state 1
            INTCON2bits.INTEDG0 = 1;        // Change Edge interrupt of INT 0 to Low to High            
            return;
        }
        
        case 1 :
        {
            
            if(Time_Elapsed > 8500 && Time_Elapsed < 9500)
                Nec_State = 2;              // Move to case 2
            else
                Reset_Nec_State();
            INTCON2bits.INTEDG0 = 0;       // Change edge from High to Low  
            return;
        }
        
        case 2 :                            
        {
            
            if(Time_Elapsed > 4000 && Time_Elapsed < 5000)
                Nec_State = 3;              //Move to case 3
            else
                Reset_Nec_State();
            INTCON2bits.INTEDG0 = 1;        //Change edge from Low to High
            return;
        }
        
        case 3 :                           
        {
            if(Time_Elapsed > 400 && Time_Elapsed < 700)
                Nec_State = 4;              //Move to case 4
            else
                Reset_Nec_State();
            INTCON2bits.INTEDG0 = 0;        //Change edge from High to Low
            return;
        }
        
        case 4 :                            
        {
            
            if(Time_Elapsed > 400 && Time_Elapsed < 1800)
            {
                Nec_code = Nec_code << 1;
                if(Time_Elapsed > 1000)
                {
                    Nec_code = Nec_code + 1;
                }
                    bit_count = bit_count + 1;
                    if(bit_count > 31)
                    {
                        nec_ok = 1;
                        INTCONbits.INT0IE = 0;      //Change edge from Low to High
                        Nec_Button = Nec_code >> 8;
                        Nec_State = 0;              //Reset State
                    }
                    else
                    {
                        Nec_State = 3;              //Go to previous State
                    }
                
            }
            else
            {
                Reset_Nec_State();                  //Reset State
            }
            INTCON2bits.INTEDG0 = 1;
            return;
        }
    }
}
