#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/delay.h>
int interr =0;
int state = 0;


// -----------ISRs------------------------

// Switch ISR
ISR(PORTF_PORT_vect){
	
	switch(PORTF.INTFLAGS){
		case 0x40:
		state = 2;
		TCA_SPLIT(127,254);
		break;
		
		case 0x20:
		state = 1;
		TCA_SPLIT(254,127);
		break;
	}
	
	//clear : interrupt flag
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS=intflags;


}


//PWM split ISR
ISR(TCA0_HUNF_vect){
	//C CLEAR THE INTERRUPT FLAG
	if (state == 1){
		PORTD.OUTCLR = PIN1_bm;
		PORTD.OUT |= PIN0_bm;
	}
	if (state == 2){
		PORTD.OUTTGL = PIN0_bm;
		PORTD.OUTTGL = PIN1_bm;
	}
	
	

	int intflags = TCA0.SPLIT.INTFLAGS;
	TCA0.SPLIT.INTFLAGS = intflags;

}

//PWM split ISR
ISR(TCA0_OVF_vect)
{
	if (state == 1 || state == 0){
		PORTD.OUTTGL = PIN0_bm;
		PORTD.OUTTGL = PIN1_bm;
	}
	
	if (state == 2){
		PORTD.OUTCLR = PIN0_bm;
		PORTD.OUT |= PIN1_bm;
	}
	
	int intflags = TCA0.SPLIT.INTFLAGS;
	TCA0.SPLIT.INTFLAGS = intflags;
}

//ADC ISR
ISR(ADC0_WCOMP_vect){
	int intflags  = ADC0.INTFLAGS;
	ADC0.INTFLAGS =intflags;
	
	PORTD.OUT |= PIN1_bm;
	PORTD.OUT |= PIN0_bm;
	PORTD.OUTCLR |= PIN2_bm;
	
}
// -------------------------------------------

// -----------FUNCTIONS------------------------

void PIN_INIT(void){
		PORTD.DIR |= PIN0_bm; //pin 0 right movement
		PORTD.OUTCLR= PIN0_bm; // enable right movement
		
		PORTD.DIR |= PIN1_bm; //pin 1 left
		PORTD.OUTCLR= PIN1_bm; // enable left movement
		
		PORTD.DIR |= PIN2_bm;
}

void ADC_INIT(void){
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; // 10 BIT RESOLUTION
	ADC0.CTRLA |= ADC_FREERUN_bm; // FREE RUNNING MODE ENABLED
	ADC0.CTRLA |=  ADC_ENABLE_bm; // ENABLE ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //THE BIT
	
	// ENABLE DEBUG MODE
	ADC0.DBGCTRL |= ADC_DBGRUN_bm;
	// WINDOW COMPARATOR MODE
	ADC0.WINLT |= 10; //SETJ THRESHOLD
	ADC0.INTCTRL |= ADC_WCMP_bm; // ENABLE INTERRRUPTS FOR WCM
	ADC0.CTRLE |= ADC_WINCM0_bm; // INTERRUPT WHEN RESULT <WINLT
}


void Switch_init(void){
	PORTF.PIN5CTRL |=PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	PORTF.PIN6CTRL |=PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
}


void TCA_SPLIT(int lper, int hper )
{
	
	PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc;
	
	//enable split mode
	
	TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm;
	TCA0.SPLIT.CTRLB =TCA_SPLIT_HCMP0EN_bm | TCA_SPLIT_LCMP0EN_bm;
	
	TCA0.SPLIT.LPER = lper;
	TCA0.SPLIT.LCMP0 = 254;
	TCA0.SPLIT.HPER = hper;
	TCA0.SPLIT.HCMP0 = 254;
	TCA0.SPLIT.INTCTRL = TCA_SPLIT_HUNF_bm;
	TCA0.SPLIT.INTCTRL |= TCA_SPLIT_LUNF_bm;
	
	TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV16_gc | TCA_SPLIT_ENABLE_bm;
	
}

void TCA0_hardReset(void){
	TCA0.SINGLE.CTRLA &= ~(TCA_SINGLE_ENABLE_bm);
	TCA0.SINGLE.CTRLESET = TCA_SINGLE_CMD_RESET_gc;
}

// -------------------------------------------

int main(){
	PIN_INIT();
	ADC_INIT();
	Switch_init();
	
	sei();
	ADC0.COMMAND |= ADC_STCONV_bm; // START CONVERSION
	
	while (1){
		TCA_SPLIT(254,254); while(1){  }
	}
}
