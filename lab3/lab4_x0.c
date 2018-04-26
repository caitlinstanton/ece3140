#include "3140_concur.h"
#include "utils.h"
#include "lock.h"
#include "cond.h"

lock_t l;
cond_t cr;
cond_t cw; 

unsigned int nr= 0;
unsigned int nw= 0;

void p1 (){
	int i;
	for (i=0; i < 10; i++) {
		delay();
		LEDBlue_Toggle();
	}
}

void reader (void) {
	l_lock(&l);
	
	if(nw!=0){
		c_wait(&l,&cr);
	}
	nr++;
	if(c_waiting(&l,&cr)){
		c_signal(&l,&cr);
	}
	else{
		l_unlock(&l);
	}
	
	/*start reading*/
	p1();
	/*end reading*/
	
	l_lock(&l);
	
	nr--;
  	if(c_waiting(&l,&cw) && nr == 0){
  		c_signal(&l,&cw);
  	}
  	else if(c_waiting(&l,&cr)){
  		c_signal(&l,&cr);
  	}
  	else{
  		l_unlock(&l);			
  	}
}

void writer (void) {
	l_lock(&l);
	
	if(nw!=0 || nr!=0){
		c_wait(&l,&cw);
	}
	nw++;
	
	l_unlock(&l);
	
	/*start writing*/
	LEDRed_Toggle();
	
	delay();
	
	LEDRed_Toggle();
	delay();
	
	l_lock(&l);
	
	nw--;
  	if(c_waiting(&l,&cw) && nr == 0){
  		c_signal(&l,&cw);
  	}
  	else if(c_waiting(&l,&cr)){
  		c_signal(&l,&cr);
  	}
  	else{
  		l_unlock(&l);		
  	}
}

int main (void){
	LED_Initialize();   
	
	l_init (&l);
	c_init (&l,&cr);
	c_init (&l,&cw);
 
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
	
	while (1) ;

	return 0;
}
