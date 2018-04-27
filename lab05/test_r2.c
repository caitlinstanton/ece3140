///*************************************************************************
// * pNRT1: ^r_r_r_r v
// * pNRT2: ^_g_g_g_g v
// * pRT1:  ^__________b b b v
// *
// * We expected to see a sequence of processes as depicted above:
// *     - Non real-time processes pNRT1 and pNRT1 both start at time zero.
// *       They should be executed concurrently (blinking red and green 
// *       alternatively beginning with the red LED
// *       pRT1 has priority, but has a very late start time. 
// *     - After pNRT1 and pNRT2 complete, pRT1 begins and blinks the blue 
// *       LED 5x @ 5Hz.
// *     - pRT1 should not miss its deadline, therefor, the green LED should not
// *       flash at the end.
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
//#define NRT1_STACK 20
//#define NRT2_STACK 20
//#define RT_STACK  20
// 
///*--------------------------------------*/
///* Time structs for real-time processes */
///*--------------------------------------*/

///* Constants used for 'work' and 'deadline's */
//realtime_t t_1msec = {0, 1};
//realtime_t t_10sec = {10, 0};

///* Process start time */
//realtime_t t_pRT1 = {4, 0};

///*------------------*/
///* Helper functions */
///*------------------*/
//void shortDelay(){delay();}
//void mediumDelay() {delay(); delay();}
//void longDelay() {delay(); delay(); delay();}

///*-------------------------------------------------------------
// * Real-time process
// *Blinks red LED three times
//*Should happen after non real time processes have completed
// *------------------------------------------------------------*/

//void pRT1(void) {
//	int i;
//	for (i=0; i<3;i++){
//	LEDBlue_On();
//	mediumDelay();
//	LEDBlue_Toggle();
//	mediumDelay();
//	}
//}

///*----------------------------------------------------
// * Non real-time process
// *   Blinks red LED 4 times. 
// *----------------------------------------------------*/
// 
//void pNRT1(void) {
//	int i;
//	for (i=0; i<4;i++){
//		LEDRed_On();
//		shortDelay();
//		LEDRed_Toggle();
//		shortDelay();
//	}
//}

///*----------------------------------------------------
// * Non real-time process
// *   Blinks green LED 4 times. 
// *----------------------------------------------------*/
// 
//void pNRT2(void) {
//	int i;
//	for (i=0; i<4;i++){
//		LEDGreen_On();
//		shortDelay();
//		LEDGreen_Toggle();
//		shortDelay();
//	}
//}

///*--------------------------------------------*/
///* Main function - start concurrent execution */
///*--------------------------------------------*/
//int main(void) {	
//	 
//	LED_Initialize();

//    /* Create processes */ 

//    if (process_rt_create(pRT1, RT_STACK, &t_pRT1, &t_10sec) < 0) { return -1; } 
//		if (process_create(pNRT1, NRT1_STACK) < 0) { return -1; }
//    if (process_create(pNRT2, NRT2_STACK) < 0) { return -1; }
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
