///*************************************************************************
// * This is a test to make sure that the ready queue is sorted in ascending
// * order of absolute deadline and that real time processes are interrupted
// * if another real time with an earlier deadline is added to the real time
// *  process queue.
// * 
// *   There shuld be two blinks of the LED at the end becaue pRT2 and PRT3 
// *   both miss their deadlines.
// * 
// ************************************************************************/
// 
//#include "utils.h"
//#include "3140_concur.h"
//#include "realtime.h"

///*--------------------------*/
///* Parameters for test case */
///*--------------------------*/

///* Stack space for processes */
//#define NRT_STACK  20
//#define RT1_STACK  20
//#define RT2_STACK  20
//#define RT3_STACK  20
// 
///*--------------------------------------*/
///* Time structs for real-time processes */
///*--------------------------------------*/

///* Constants used for 'work' and 'deadline's */
//realtime_t t_1msec = {0, 1};
//realtime_t t_3msec = {0, 3};
//realtime_t t_1sec = {1, 0};
//realtime_t t_10sec = {10, 0};

///* Process start time */
//realtime_t t_pRT1 = {1, 0}; 
//realtime_t t_pRT2 = {2, 0};
//realtime_t t_pRT3 = {9, 0}; 

// 
///*------------------*/
///* Helper functions */
///*------------------*/
//void shortDelay(){delay();}
//void mediumDelay() {delay(); delay();}
//void longDelay() {delay(); delay(); delay();} 



///*----------------------------------------------------
// * Non real-time process
// *   Blinks red LED 4 times. 
// *   Should be blocked by real-time process at first.
// *----------------------------------------------------*/
// 
//void pNRT(void) {
//	int i;
//	for (i=0; i<4;i++){
//	LEDRed_On();
//	shortDelay();
//	LED_Off();
//	shortDelay();
//	}
//}

///*-----------------------------------------------------
// * First Real-time process
// *Blinks blue LED 3 times
// *----------------------------------------------------*/

//void pRT1(void) {
//	int i;
//	for (i=0; i<3;i++){
//	LEDBlue_On();
//	mediumDelay();
//	LED_Off();
//	mediumDelay();
//	}
//}

///*-----------------------------------------------------
// * Second Real-time process
// *Blinks green LED 2 times
// *----------------------------------------------------*/

//void pRT2(void) {
//	int i;
//	for (i=0; i<2;i++){
//	LEDGreen_On();
//	longDelay();
//	LED_Off();
//	longDelay();
//	}
//}

///*-----------------------------------------------------
// * Second Real-time process
// *Blinks blue LED 5 times
// *----------------------------------------------------*/

//void pRT3(void) {
//	int i;
//	for (i=0; i<3;i++){
//	LEDBlue_On();
//	shortDelay();
//	LED_Off();
//	shortDelay();
//	}
//}
///*--------------------------------------------*/
///* Main function - start concurrent execution */
///*--------------------------------------------*/
//int main(void) {	
//	 
//	LED_Initialize();

//    /* Create processes */ 
//    if (process_create(pNRT, NRT_STACK) < 0) { return -1; }
//    if (process_rt_create(pRT1, RT1_STACK, &t_pRT1, &t_1msec) < 0) { return -1; } 
//    if (process_rt_create(pRT2, RT2_STACK, &t_pRT2, &t_3msec) < 0) { return -1; } 
//		if (process_rt_create(pRT3, RT3_STACK, &t_pRT3, &t_1msec) < 0) { return -1; } 
//    /* Launch concurrent execution */
//	process_start();

//  LED_Off();
//  while(process_deadline_miss>0) {
//		LEDGreen_On();
//		shortDelay();
//		LED_Off();
//		shortDelay();
//		process_deadline_miss--;
//	}
//	
//	/* Hang out in infinite loop (so we can inspect variables if we want) */ 
//	while (1);
//	return 0;
//}