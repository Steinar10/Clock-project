/* 
 * File:   main.c
 * Author: steinar
 *
 * Created on 02 December 2023, 16:51
 */

#pragma config BOOTRST = 0

#define F_CPU 2000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void setupPWM()
{
    PORTC.REMAP |= PORT_TC0A_bm //Better to accidentally send bogus into unused pin than into I2C data line
                |  PORT_TC0B_bm
                |  PORT_TC0C_bm
                |  PORT_TC0D_bm;
    TCC0.CTRLA = TC_CLKSEL_DIV1_gc;
    TCC0.CTRLB = TC0_CCBEN_bm
                |TC0_CCCEN_bm
                |TC0_CCDEN_bm
                |TC_WGMODE_SINGLESLOPE_gc;
    TCC0.PER = 0x0265;
    
    sei();
    PMIC.CTRL |= PMIC_LOLVLEN_bm;
    TCC0.INTCTRLB = TC_CCAINTLVL_LO_gc;
    
    TCC0.CCB = 0;
    TCC0.CCC = 0;
    TCC0.CCD = 0;
    
    TCC0.CCA = 0x100;
}


volatile uint8_t colorNum = 0;
#define NUM_MAX_COLORS 12
#define RGB 3
#define R 0
#define G 1
#define B 2

uint8_t numColors = 12;
uint8_t colors[NUM_MAX_COLORS][RGB];// = {{25, 0, 0}, {0, 25, 0}, {0, 0, 25}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
uint8_t port_a_per_color[NUM_MAX_COLORS] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0, 0, 0, 0};
uint8_t port_d_per_color[NUM_MAX_COLORS] = {0, 0, 0, 0, 0, 0, 0, 0, 0x08, 0x04, 0x02, 0x01};
ISR(TCC0_CCA_vect) //Instead of this, do DMA(?) or interrupt into CCx buffer, triggered by said CCx event!
{
    if(++colorNum >= numColors) { //Fixed this. God damn.
        colorNum = 0;
    }
    TCC0.CCB = colors[colorNum][R];
    TCC0.CCD = colors[colorNum][G];
    TCC0.CCC = colors[colorNum][B];
    PORTA.OUT = port_a_per_color[colorNum];
    PORTD.OUTCLR = 0x0F;
    PORTD.OUTSET = port_d_per_color[colorNum];
}



void setupPeriodicInterrupt()
{
    TCC1.CTRLA = TC_CLKSEL_DIV1024_gc;
    TCC1.INTCTRLA = TC_OVFINTLVL_LO_gc;
    
    TCC1.PER = 0x02;
}

uint8_t rainbowR = 0x00;
uint8_t rainbowG = 0x55;
uint8_t rainbowB = 0x99;
int8_t rdir = 1;
int8_t gdir = 1;
int8_t bdir = 1;

ISR(TCC1_OVF_vect)
{
        rainbowR += rdir;
        if(rainbowR == 0x00) {
            rdir = -rdir;
            rainbowR += rdir;
        }
        rainbowG += gdir;
        if(rainbowG == 0x00) {
            gdir = -gdir;
            rainbowG += gdir;
        }
        rainbowB += bdir;
        if(rainbowB == 0x00) {
            bdir = -bdir;
            rainbowR += bdir;
        }
        setColor(12, rainbowR, rainbowG, rainbowB);
}

void setColor(uint8_t color, uint8_t redVal, uint8_t greenVal, uint8_t blueVal)
{
    if(--color > 11) {
        return;
    }
    
    colors[color][R] = redVal;
    colors[color][G] = greenVal;
    colors[color][B] = blueVal;
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
    
    setupPeriodicInterrupt();
    
    setColor(1, 25, 0, 0);
    setColor(2, 0, 50, 0);
    setColor(3, 0, 100, 25);
    setColor(5, 100, 1, 0);
    setColor(6, 0, 0, 100);
    setColor(7, 100, 20, 0);
    setColor(8, 50, 70, 0);
    setColor(9, 80, 0, 20);
    setColor(10, 20, 0, 80);

    while (1) 
    {
    }
}