#include <fsl_device_registers.h>
#include <system_MK64F12.h>

int flag = 0;
int sec = 21000000;

void setup() {
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK; //Enable the clock to port B
	// enable blue LED
	PORTB->PCR[21] = PORT_PCR_MUX(001); //Set up PTB21 as GPIO
	PTB->PDDR |= (1 << 21); //enable PTB21 as an output
	PTB->PSOR = (1 << 21); //Led off

	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK; //Enable the clock to port E
	// enable blue LED
	PORTE->PCR[26] = PORT_PCR_MUX(001); //Set up PTE26 as GPIO
	PTE->PDDR |= (1 << 26); //enable PTE26 as an output
	PTE->PSOR = (1 << 26); //Led off
}

void space() {
 int counter = 5200000; //1 sec with CPU frequency (~120MHz)
	while(counter > 0)
		counter--;
}

void blue_toggle() {
	for(int i=0; i < 1000000; i++) {
		PTB->PCOR = (1 << 21);
		space();
		PTB->PSOR = (1 << 21);
		space();
	}
}

void timer(){
	SIM->SCGC6 = SIM_SCGC6_PIT_MASK; // Enable clock to PIT module
	PIT->MCR = 0x00; //Enable PIT module

	PIT->CHANNEL[0].LDVAL = 21000000; // Set load value to 21Mhz
	PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK;
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;
}


/*
     Main program: entry point
*/
int main (void)
{
	NVIC_EnableIRQ(PIT0_IRQn); /* enable PIT0 Interrupts (for part 2) */
	setup();
	timer();
	blue_toggle();
}

/*
     PIT Interrupt Handler
*/
void PIT0_IRQHandler(void)
{
	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK; //clear
  //green_led toggle
	if(flag == 0) {
		flag = 1;
		PTE->PCOR = (1 << 26); // led on
		PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK;  //timer disable
		PIT->CHANNEL[0].LDVAL = 0.1 * sec;  //modify load val (0.1 secs)
		PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK; //timer enable
	}
  else {
		flag = 0;
		PTE->PSOR = (1 << 26); //Led off
		PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK; //timer disable
		PIT->CHANNEL[0].LDVAL = 0.9 * sec; //modify load val (0.9 secs)
		PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;  //timer enable
	}
}
