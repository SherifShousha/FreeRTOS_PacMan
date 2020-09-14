/*
 * buttons.c
 *
 *  Created on: Jul 4, 2019
 *      Author: lab_espl_stud16
 */

#include "ESPL_functions.h"
#include "buttons.h"

QueueHandle_t ButtonQueue;

/**
 * This task polls the joystick value every 20 ticks
 */
void checkButtons(void * params) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	struct buttons buttonStatus = { { 0 } };
	const TickType_t PollingRate = 20;

	while (1) {
		// Remember last joystick values
		buttonStatus.joystick.x = (uint8_t)(
				ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
		buttonStatus.joystick.y = (uint8_t) 255
				- (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);

		// Buttons not debounced, delaying does not count as debouncing
		buttonStatus.A = !GPIO_ReadInputDataBit(ESPL_Register_Button_A,
		ESPL_Pin_Button_A);
		buttonStatus.B = !GPIO_ReadInputDataBit(ESPL_Register_Button_B,
		ESPL_Pin_Button_B);
		buttonStatus.C = !GPIO_ReadInputDataBit(ESPL_Register_Button_C,
		ESPL_Pin_Button_C);
		buttonStatus.D = !GPIO_ReadInputDataBit(ESPL_Register_Button_D,
		ESPL_Pin_Button_D);
		buttonStatus.E = !GPIO_ReadInputDataBit(ESPL_Register_Button_E,
		ESPL_Pin_Button_E);
		buttonStatus.K = !GPIO_ReadInputDataBit(ESPL_Register_Button_K,
		ESPL_Pin_Button_K);

		xQueueSend(ButtonQueue, &buttonStatus, 100);

		// Execute every 20 Ticks
		vTaskDelayUntil(&xLastWakeTime, PollingRate);
	}
}


