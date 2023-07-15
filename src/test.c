/* Kernel includes. */
#include "../FreeRTOS.h"
#include "../task.h"
#include <xc.h>

void LEDtask(void *pvParameters );
void KEYtask(void *pvParameters);
void MAINtask(void *pvParameters);

/********************************************************************************/
/*		test_init																*/
/********************************************************************************/
void test_init()
{

	xTaskCreate( LEDtask,					/* The function that implements the task. */
			"LEDtask",						/* The text name assigned to the task - for debug only as it is not used by the kernel. */
			configMINIMAL_STACK_SIZE + 100, /* The size of the stack to allocate to the task. */
			( void * ) 0,					/* The parameter passed to the task - just to check the functionality. */
			4, 								/* The priority assigned to the task. */
			NULL );							/* The task handle is not required, so NULL is passed. */

	xTaskCreate( KEYtask,					/* The function that implements the task. */
			"KEYtask",						/* The text name assigned to the task - for debug only as it is not used by the kernel. */
			configMINIMAL_STACK_SIZE + 100, /* The size of the stack to allocate to the task. */
			( void * ) 0,					/* The parameter passed to the task - just to check the functionality. */
			1, 								/* The priority assigned to the task. */
			NULL );							/* The task handle is not required, so NULL is passed. */

	xTaskCreate( MAINtask,					/* The function that implements the task. */
			"MAINtask",						/* The text name assigned to the task - for debug only as it is not used by the kernel. */
			configMINIMAL_STACK_SIZE + 4000, /* The size of the stack to allocate to the task. */
			( void * ) 0,					/* The parameter passed to the task - just to check the functionality. */
			2, 								/* The priority assigned to the task. */
			NULL );							/* The task handle is not required, so NULL is passed. */

	vTaskStartScheduler();					/* Start the tasks and timer running. */
}
/********************************************************************************/
/*		LEDtask																	*/
/********************************************************************************/
void LEDtask(void *pvParameters )
{
    
	for(;;){
        LATBbits.LATB4 = 1;
		vTaskDelay(500);
        LATBbits.LATB4 = 0;
		vTaskDelay(500);
	}
}
/********************************************************************************/
/*		KEYtask																	*/
/********************************************************************************/
void KEYtask(void *pvParameters)
{
	for(;;){
		vTaskDelay(1);
	}
}
