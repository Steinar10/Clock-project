/* 
 * File:   main.c
 * Author: steinar
 *
 * Created on 02 December 2023, 16:51
 */

#define F_CPU 2000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

inline void setLedChannel(uint8_t ledNum, uint8_t onOrOff)
{
	if((ledNum == 0) || (ledNum > 12)) {
		return;
	}
	
	uint8_t ledValue = !onOrOff; //PMOS high side switches mean that pin on = LED off
	
	if(ledNum > 8) {
		uint8_t portDPin = 12 - ledNum; // 3 - (ledNum - 9); //Function to translate led number to port D pin.
		PORTD.OUTCLR = (1 << portDPin);
		PORTD.OUTSET = (ledValue << portDPin);
		return;
	}
	//if ledNum <= 8:
	{
		uint8_t portAPin = 8 - ledNum; //7 - (ledNum - 1); //Done this way for clarity. Does this even contribute to clarity?
		PORTA.OUTCLR = (1 << portAPin);
		PORTA.OUTSET = (ledValue << portAPin);
	}
	return;
}

void clearAllLeds() //Set port A 0-7 and port D 0-3 to turn off all LED channels.
{
    PORTA.OUTSET = 0xFF;
    PORTD.OUTSET = 0x0F;
}

void setupPWM()
{
    PORTC.REMAP |= PORT_TC0A_bm
                |  PORT_TC0B_bm
                |  PORT_TC0C_bm
                |  PORT_TC0D_bm;
    TCC0.CTRLA = TC_CLKSEL_DIV2_gc;
    TCC0.CTRLB = TC0_CCBEN_bm
                |TC0_CCCEN_bm
                |TC0_CCDEN_bm
                |TC_WGMODE_SINGLESLOPE_gc;
    TCC0.PER = 0x0125;
    
    sei();
    PMIC.CTRL |= PMIC_LOLVLEN_bm;
    TCC0.INTCTRLB = (TC_CCAINTLVL_LO_gc << 0);
    
    TCC0.CCB = 0;
    TCC0.CCC = 0;
    TCC0.CCD = 0;
    
    TCC0.CCA = 0xFF;
}

void setColor(uint8_t red, uint8_t green, uint8_t blue)
{
    TCC0.CCB = red;
    TCC0.CCD = green;
    TCC0.CCC = blue;
}

volatile uint8_t colorNum = 0;
#define NUMCOLORS 3
#define RGB 3
#define R 0
#define G 1
#define B 2
const uint8_t colors[RGB][NUMCOLORS] = {{0x00, 0, 0}, {0x0F, 0, 0}, {0x00, 0, 0}};
const uint8_t port_a_per_color[NUMCOLORS] = {0xFF, 0xFF, 0x00};
const uint8_t port_d_per_color[NUMCOLORS] = {0x00, 0x00, 0x00};

ISR(TCC0_CCA_vect)
{
    if(++colorNum > NUMCOLORS) {
        colorNum = 0;
    }
    TCC0.CCB = colors[R][colorNum];
    TCC0.CCD = colors[G][colorNum];
    TCC0.CCC = colors[B][colorNum];
    PORTA.OUT = ~port_a_per_color[colorNum];
    PORTD.OUTSET = 0x0F;
    PORTD.OUTCLR = port_d_per_color[colorNum];
}



int main(void)
{
    /* Replace with your application code */
	
	PORTA.DIR = 0xFF;
	PORTA.OUT = 0xFC;
	PORTD.DIRSET = 0x0F;
	PORTD.OUTSET = 0x0F;
	PORTC.DIRSET = (1 << 5) | (1 << 6) | (1 << 7);
    
    setupPWM();
    setColor(0, 50, 100);
    
    while (1) 
    {
		for(int i = 5; i <= 7; ++i) { //Cycle through R, G and B on active LEDs
			/*switch(i) {
                case 5:{
                    setColor(1, 0, 0);
                    break;}
                case 6:{
                    setColor(0, 0, 0);
                    break;
                }
                case 7:{
                    setColor(00, 30, 10);
                    break;
                }
                    
            }*/
			for(int j = 0; j <= 12; ++j) {
                clearAllLeds();
				setLedChannel(j, 1);
				_delay_ms(500);
			}
		}
    }
}