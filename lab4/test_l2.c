#include "3140_concur.h"
#include "utils.h"
#include "lock.h"

/** Non-Trivial Test 2
All 3 processes share the same lock, but the 2nd middle process never unlocks
Thus, the 3rd process never gets to run.
*/

lock_t l;

/** Process 1
Turns a Green LED on 3 times
*/
void p1(void){
  /*CS*/
  l_lock(&l);
	int i = 0;
	while ( i < 3 ) {
    i++;
	  LEDGreen_Toggle();
	  delay();
	  LEDGreen_Toggle();
	  delay();
	}
	l_unlock(&l);
}

/** Process 2
Turns a Blue LED on 3 times
Locks the process, but never calls unlock!
*/
void p2( void ) {
	l_lock( &l );
	int i = 0;
	while ( i < 3 ) {
    i++;
	  LEDBlue_Toggle();
	  delay();
	  LEDBlue_Toggle();
	  delay();
	// Never calls l_unlock( &l )!
	}
}

/** Process 3
Turns a Red LED on 3 times
Since is queued up after process 2, never executed
*/
void p3( void ){
	l_lock( &l );
	int i = 0;
	while ( i < 3 ) {
    i++;
	  LEDRed_Toggle();
	  delay();
	  LEDRed_Toggle();
	  delay();
	  l_unlock( &l );
	}
}

int main(void){
	LED_Initialize();           /* Initialize the LEDs           */	

	l_init (&l);                // Initialize the lock
	
	if (process_create (p1,20) < 0) { // Toggles Greed LED 3x
	 	return -1;
	}
	if (process_create (p2,20) < 0) { // Toggled Red LED 3x, never unlocks
	 	return -1;
	}
	if (process_create (p3,20) < 0) { // Never gets to run
	 	return -1;
	}
	
	process_start();
	LEDGreen_On();

	while(1);
	return 0;	
}
