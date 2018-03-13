#include <fsl_device_registers.h>
#include <system_MK64F12.h>

int red_led = 1;

void setup() {
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK; //Enable the clock to port B
	// enable red lED
	PORTB->PCR[22] = PORT_PCR_MUX(001); //Set up PTB22 as GPIO
	PTB->PDDR |= (1 << 22); //enable PTB22 as an output

	// // enable blue LED
	// PORTB->PCR[21] = PORT_PCR_MUX(001); //Set up PTB21 as GPIO
	// PTB->PDDR |= (1 << 21); //enable PTB21 as an output
}

void LEDRed_On() {
	red_led = 1;
	PTB->PCOR = (1 << 22);
}

void LEDRed_Off() {
	red_led = 0;
	PTB->PSOR = (1 << 22);
}

void timer(){
	SIM->SCGC6 = SIM_SCGC6_PIT_MASK; // Enable clock to PIT module
	PIT->MCR = 0x00; //Enable PIT module
	PIT->CHANNEL[0].LDVAL = 0x1406F40; // Set load value to 21Mhz
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;
}
/*
		 Main program: entry point
*/
int main (void)
{
	setup();
	timer();
	while(1) {
		if(PIT->CHANNEL[0].TFLG) {
			PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK; //clear
			if(red_led)
				LEDRed_Off();
			else
				LEDRed_On();
		}
	}
}
