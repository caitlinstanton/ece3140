#include "3140_concur.h"
#include "utils.h"
#include "lock.h"

lock_t w;
lock_t r;

unsigned int nr= 0;

void p1 (){
	int i;
	for (i=0; i < 10; i++) {
		delay();
		LEDBlue_Toggle();
	}
}

void reader (void) {
  /* enter */
	l_lock(&r);
	
	nr++;
	if (nr == 1) {
	  l_lock (&w);	
	}
	
	l_unlock (&r);
	
	/*start reading*/
	p1();
	/*end reading*/

  /* exit */
	l_lock(&r);

	nr--;
	if (nr == 0) {
	   l_unlock (&w);
	}

	l_unlock (&r);
}

void writer (void) {
  /* enter */	
	l_lock(&w);
	
	/*start writing*/
	delay();

	LEDRed_Toggle();
	
	delay();
	
	LEDRed_Toggle();	
	/*end writing*/

  /* exit */			
	l_unlock(&w);
}



int main (void){		
	LED_Initialize();           /* Initialize the LEDs           */	
	l_init (&w);
	l_init (&r);
 
	if (process_create (writer,20) < 0) {
	 	return -1;
	}
	if (process_create (reader,20) < 0) {
	 	return -1;
	}
	if (process_create (reader,20) < 0) {
	 	return -1;
	}
	if (process_create (reader,20) < 0) {
	 	return -1;
	}
	if (process_create (writer,20) < 0) {
	 	return -1;
	}
	
	process_start();
 
	LEDGreen_On();

	while(1);
}

