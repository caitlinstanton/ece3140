#include "3140_concur.h"
#include "utils.h"
#include "lock.h"

/** Non-Trivial Test 1
Nested locks, careful to avoid deadlock
*/

lock_t l1;
lock_t l2;

void p1(void){
	int i= 0;
	while(i < 3){
		/*NCS*/
		delay();
		i++;
		
		/* CS1 - Toggles Red LED */
		l_lock( &l1 );
		LEDRed_Toggle();
		delay();
		LEDRed_Toggle();
		delay();
		
		l_lock( &l2 ); // A second, different lock
		/* CS2 - Toggles Blue LED*/
		LEDBlue_Toggle();
		delay();
		LEDBlue_Toggle();
		delay();
		l_unlock( &l2 ); // Unlock lock 2
		
		l_unlock( &l1 ); // Unlock lock 1
	}
}

int main(void){
	LED_Initialize();           /* Initialize the LEDs */	

	l_init ( &l1 );             // Initialize lock 1
	l_init ( &l2 );             // Initializa lock 2
	
	if (process_create (p1,20) < 0) {
	 	return -1;
	}
	if (process_create (p1,20) < 0) {
	 	return -1;
	}
	
	process_start();
	LEDGreen_On();

	while(1);
	return 0;	
}
