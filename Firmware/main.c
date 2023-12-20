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

/*//Again, function obsolete(?)
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
}*/

/*void clearAllLeds() //Set port A 0-7 and port D 0-3 to turn off all LED channels.
{
    PORTA.OUTCLR = 0xFF;
    PORTD.OUTCLR = 0x0F;
}*/

void setupPWM()
{
    PORTC.REMAP |= PORT_TC0A_bm //Better to accidentally send bogus into unused pin than into I2C data line
                |  PORT_TC0B_bm
                |  PORT_TC0C_bm
                |  PORT_TC0D_bm;
    TCC0.CTRLA = TC_CLKSEL_DIV4_gc;
    TCC0.CTRLB = TC0_CCBEN_bm
                |TC0_CCCEN_bm
                |TC0_CCDEN_bm
                |TC_WGMODE_SINGLESLOPE_gc;
    TCC0.PER = 0x0115;
    
    sei();
    PMIC.CTRL |= PMIC_LOLVLEN_bm;
    TCC0.INTCTRLB = (TC_CCAINTLVL_LO_gc << 0);
    
    TCC0.CCB = 0;
    TCC0.CCC = 0;
    TCC0.CCD = 0;
    
    TCC0.CCA = 0x100;
}

/* Function obsolete by new color system(?)
void setColor(uint8_t red, uint8_t green, uint8_t blue)
{
    TCC0.CCB = red;
    TCC0.CCD = green;
    TCC0.CCC = blue;
}*/

volatile uint8_t colorNum = 0;
#define NUMCOLORS 3
#define RGB 3
#define R 0
#define G 1
#define B 2
uint8_t colors[RGB][NUMCOLORS] = {{255, 255, 0}, {0, 255, 0}, {0, 0, 255}}; //{{R1, R2, R3}, {G1, ...
uint8_t port_a_per_color[NUMCOLORS] = {0b00100000, 0b01000000, 0b10000000};
uint8_t port_d_per_color[NUMCOLORS] = {0x0, 0x0, 0x0}; //From this I expect red on 8 & 9, green on 7 & 10, blue on 6 & 11.

ISR(TCC0_CCA_vect) //Instead of this, do DMA(?) or interrupt into CCx buffer, triggered by said CCx event!
{
    if(++colorNum >= NUMCOLORS) { //Fixed this. God damn.
        colorNum = 0;
    }
    TCC0.CCB = colors[R][colorNum];
    TCC0.CCD = colors[G][colorNum];
    TCC0.CCC = colors[B][colorNum];
    PORTA.OUT = port_a_per_color[colorNum];
    PORTD.OUTCLR = 0x0F;
    PORTD.OUTSET = port_d_per_color[colorNum];
}



int main(void)
{	
	PORTA.DIR = 0xFF;
    PORTCFG.MPCMASK = 0xFF; //Seems like this works :D
    PORTA.PIN0CTRL |= PORT_INVEN_bm //These outputs are connected to PMOS gates.
                   | (PORT_ISC_INPUT_DISABLE_gc << PORT_ISC_gp);//Inputs not needed here
    PORTA.OUTCLR = 0xFF; //Turn off all LEDs
    
	PORTD.DIRSET = 0x0F;
    PORTCFG.MPCMASK = 0x0F;
    PORTD.PIN0CTRL |= PORT_INVEN_bm //These outputs are connected to PMOS gates.
                   | (PORT_ISC_INPUT_DISABLE_gc << PORT_ISC_gp);
	PORTD.OUTCLR = 0x0F;
	PORTC.DIRSET = (1 << 5) | (1 << 6) | (1 << 7); //PortC[5..7] are R, B and G low-side NMOSes, respectively.
    
    setupPWM();
    
    
    while (1) 
    {
        /* Below code obsolete because of new colour system.
        
		for(int i = 5; i <= 7; ++i) { //Cycle through R, G and B on active LEDs
			switch(i) {
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
                    
            }
			for(int j = 0; j <= 12; ++j) {
                clearAllLeds();
				setLedChannel(j, 1);
				_delay_ms(500);
			}
		}*/
    }
}