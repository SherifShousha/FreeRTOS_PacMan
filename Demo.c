/**
 * This is the main file of the ESPLaboratory Demo project.
 * It implements simple sample functions for the usage of UART,
 * writing to the display and processing user inputs.
 *
 * @author: Alex Hoffman alex.hoffman@tum.de (RCS, TUM)
 * 			Jonathan M��ller-Boruttau,
 * 			Tobias Fuchs tobias.fuchs@tum.de
 * 			Nadja Peters nadja.peters@tum.de (RCS, TUM)
 *
 */
#include "includes.h"
#include <math.h>
#include <stdbool.h>
#include "string.h"
#include "stdlib.h"
#include "Demo.h"
#include "FieldOne.h"
#include "buttons.h"
#include "moveLogic.h"
#include "inky.h"
#include "clyde.h"


#define DISPLAY_SIZE_X  320
#define DISPLAY_SIZE_Y  240

#define PAUSED_TEXT_X(TEXT)     DISPLAY_SIZE_X / 2 - (gdispGetStringWidth(TEXT, font1) / 2)
#define PAUSED_TEXT_Y(LINE)     DISPLAY_SIZE_Y / 2 - (gdispGetFontMetric(font1, fontHeight) * -(LINE + 0.5))

#define BUTTON_QUEUE_LENGTH 20
#define STATE_QUEUE_LENGTH 1

#define STATE_COUNT 3

#define STATE_ONE   1
#define STATE_TWO   2
#define STATE_THREE 3
#define STATE_FOURTH 4
#define STATE_FIFTHE 5
#define STATE_SIXTH 6
#define STATE_SEVENTH 7

#define NEXT_TASK   1
#define PREV_TASK   2
#define SETTING 3
#define MENU 4
#define LOSE 5
#define FIELD 6
#define PAUSED 7
#define CONTINUE 8

// Read only, there should be no globals being written to that are not locked via semaphores etc
// Start and stop bytes for the UART protocol
static const uint8_t startByte = 0xAA, stopByte = 0x55;
font_t font1; // Load font for ugfx
font_t font; // Load font for ugfx

//Function prototypes
void frameSwapTask(void * params);
void basicStateMachine(void * params);
void checkButtons(void * params);
void menu (void * params);
void setting (void * params);
void draw (void * params);
void drawTask1(void * params);
void drawTask2(void * params);
void lose(void * params);
void pause(void * params);

QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t ESPL_DisplayReady;
SemaphoreHandle_t DrawReady; // After swapping buffer calll drawing
//SemaphoreHandle_t DrawLock;
SemaphoreHandle_t RefreshScreen;
/*
 * All variables handled by multiple tasks should be sent in SAFE ways, ie. using queues.
 * Global variables should only be used for managing handles to the RTOS components used
 * to perform thread safe, multi-threaded programming.
 */
// Stores lines to be drawn
QueueHandle_t StateQueue;

// Task handles, used for task control
TaskHandle_t frameSwapHandle;
TaskHandle_t drawTask1Handle;
TaskHandle_t drawTask2Handle;
TaskHandle_t stateMachineHandle;
TaskHandle_t drawHandle;
TaskHandle_t menuHandle;
TaskHandle_t settingHandle;
TaskHandle_t loseHandle;
TaskHandle_t pausedHandle;

static int live = 3, score = 0, power = 10 , level = 1, Pspeed =1 , Ghostspeed =1 , highScore1=0, highScore2 =0;
static bool restField = false;

 int main(void) {
	// Initialize Board functions and graphics
	ESPL_SystemInit();

	font1 = gdispOpenFont("DejaVuSans24*");
	font = gdispOpenFont("fixed_5x8*");

	// Initializes Draw Queue with 100 lines buffer
	ButtonQueue = xQueueCreate(BUTTON_QUEUE_LENGTH, sizeof(struct buttons));
	StateQueue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(unsigned char));

	ESPL_DisplayReady = xSemaphoreCreateBinary();
	DrawReady = xSemaphoreCreateBinary();
	//DrawLock = xSemaphoreCreateBinary();
	RefreshScreen = xSemaphoreCreateBinary();
	//xSemaphoreGive(DrawLock);

	// Initializes Tasks with their respective priority
	// Core tasks
	xTaskCreate(frameSwapTask, "frameSwapper", 1000, NULL, 4, &frameSwapHandle);
	xTaskCreate(basicStateMachine, "StateMachine", 1000, NULL, 3,
			stateMachineHandle);
	xTaskCreate(checkButtons, "checkButtons", 1000, NULL, 3, NULL);

	// Drawing tasks for various things
	xTaskCreate(menu, "menu", 1000, NULL, 2, &menuHandle);
	xTaskCreate(setting, "setting", 1000, NULL, 2, &settingHandle);
	xTaskCreate(lose, "lose", 1000, NULL, 2, &loseHandle);
	xTaskCreate(pause, "pause", 1000, NULL, 2, &pausedHandle);
	xTaskCreate(draw, "draw", 1000, NULL, 2, &drawHandle);
	xTaskCreate(drawTask1, "drawTask1", 1000, NULL, 2, &drawTask1Handle);

	xTaskCreate(drawTask2, "drawTask2", 1000, NULL, 2, &drawTask2Handle);


	vTaskSuspend(menuHandle);
	vTaskSuspend(settingHandle);
	vTaskSuspend(loseHandle);
	vTaskSuspend(pausedHandle);
	vTaskSuspend(drawHandle);
	vTaskSuspend(drawTask1Handle);
	vTaskSuspend(drawTask2Handle);

	// Start FreeRTOS Scheduler
	vTaskStartScheduler();
}

/*
 * Frame swapping happens in the background, seperate to all other system tasks.
 * This way it can be guarenteed that the 50fps requirement of the system
 * can be met.
 */
void frameSwapTask(void * params) {
	TickType_t xLastWakeTime = 0;
	TickType_t xLLastWakeTime = 0;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t frameratePeriod = 20;

	while (1) {

		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// Swap buffers
		ESPL_DrawLayer();
		// Clear background
//		gdispClear(White);
		//TODO WRITE A CLEAR FUNCTION TO CLEAR SCORES AND OTHER LITTLE BITS LIKE PACMAN
		// Draw next frame
		xSemaphoreGive(DrawReady);

		xLLastWakeTime = xLastWakeTime;

		vTaskDelayUntil(&xLastWakeTime, frameratePeriod);



	}
}

/*
 * Changes the state, either forwards of backwards
 */
void changeState(volatile unsigned char *state, unsigned char forwards) {

	switch (forwards) {
	case 0:
		if (*state == 0)
			*state = STATE_COUNT;
		else
			(*state)--;
		break;
	case 1:
		if (*state == STATE_COUNT)
			*state = 0;
		else
			(*state)++;
		break;
	default:
		break;
	}
}

/*
 * Example basic state machine with sequential states
 */
void basicStateMachine(void * params) {
	unsigned char current_state = 1; // Default state
	unsigned char state_changed = 1; // Only re-evaluate state if it has changed
	unsigned char input = 0;

	while (1) {

		if (state_changed)
			goto initial_state;

		// Handle state machine input
		if (xQueueReceive(StateQueue, &input, portMAX_DELAY) == pdTRUE) {
			if (input == NEXT_TASK) {
				changeState(&current_state, 1);
				state_changed = 1;
			} else if (input == PREV_TASK) {
				changeState(&current_state, 0);
				state_changed = 1;
			}else if(input == SETTING){
				current_state = 4;
				state_changed = 1;
			}else if(input == MENU){
				current_state = 1;
				state_changed = 1;
			}else if(input == LOSE){
				current_state = 5;
				state_changed = 1;

			}else if(input == FIELD){
				current_state = 2;
				state_changed = 1;

			}else if(input == PAUSED){
				current_state = 6;
				state_changed = 1;

			}else if(input == CONTINUE){
				current_state = 7;
				state_changed = 1;
			}
		}

		const TickType_t xDelay = 2000 / portTICK_PERIOD_MS;

		initial_state:
		// Handle current state
		if (state_changed) {
			switch (current_state) {
			case STATE_ONE:
				vTaskSuspend(settingHandle);
				vTaskSuspend(drawHandle);
				vTaskSuspend(drawTask2Handle);
				vTaskSuspend(drawTask1Handle);
				vTaskSuspend(loseHandle);
				vTaskResume(menuHandle);
				state_changed = 0;
				break;
			case STATE_TWO:
				vTaskSuspend(loseHandle);
				vTaskSuspend(menuHandle);
				vTaskSuspend(drawTask2Handle);
//				if (uxSemaphoreGetCount(DrawLock) == 0) {
//					xSemaphoreGive(DrawLock);
//				}
//				vTaskResume(drawHandle);
//				vTaskDelay(xDelay);
//				vTaskSuspend(loseHandle);
//				vTaskSuspend(drawHandle);
				xSemaphoreGive(RefreshScreen);
				xTaskCreate(drawTask1, "drawTask1", 1000, NULL, 4, &drawTask1Handle);
				state_changed = 0;
				break;
			case STATE_THREE:
				vTaskSuspend(drawHandle);
				vTaskSuspend(menuHandle);
				vTaskSuspend(drawTask1Handle);
				vTaskResume(drawTask2Handle);
				state_changed = 0;
				break;
			case STATE_FOURTH:
				vTaskSuspend(drawHandle);
				vTaskSuspend(drawTask2Handle);
				vTaskSuspend(drawTask1Handle);
				vTaskSuspend(menuHandle);
//				if (uxSemaphoreGetCount(DrawLock) == 0) {
//					xSemaphoreGive(DrawLock);
//					}

				vTaskResume(settingHandle);
				state_changed = 0;
				break;
			case STATE_FIFTHE:
				vTaskSuspend(settingHandle);
				vTaskSuspend(drawHandle);
				vTaskSuspend(drawTask1Handle);
//				if (uxSemaphoreGetCount(DrawLock) == 0) {
//					xSemaphoreGive(DrawLock);
//				}
				vTaskResume(loseHandle);
				state_changed = 0;
				break;
			case STATE_SIXTH:

				vTaskSuspend(drawTask1Handle);
				vTaskResume(pausedHandle);
				state_changed = 0;
				break;
			case STATE_SEVENTH:
				vTaskSuspend(pausedHandle);
	//			xSemaphoreGive(DrawLock);
				vTaskResume(drawTask1Handle);
				state_changed = 0;
				break;

			default:
				break;
			}
		}
	}
}

/**
 * Draw the menu
 */

void menu(void * params) {
	char str[100]; // buffer for messages to draw to display
	struct buttons buttonStatus; // joystick queue input buffer

	static int selected = 1;
	int pressed = 1;
	int debounceFlagA = 1;
	int debounceFlagC = 1;
	int debounceFlagD = 1;
	int debounceFlagE = 1;
	int Color = Red;
	int backgroundColor = Blue;

	while (1) {

		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready
//			xSemaphoreTake(DrawLock, portMAX_DELAY);


			while (xQueueReceive(ButtonQueue, &buttonStatus, 0) == pdTRUE)
				;

			// Draw the layout for menu
			gdispFillArea(0, 0, 320, 240, Black);

			// reset the field coins every time in menu
			restField = true;

			// if A pressed go up
			if (selected > 0) {
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_A,
						ESPL_Pin_Button_A) == 0) {
					debounceFlagA = 0;
				}
				if (debounceFlagA == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_A,
								ESPL_Pin_Button_A) == 1) {
					debounceFlagA = 1;
					selected--;
					if (selected <= 0) {
						selected = 1;
					}
				}
			}	// if C pressed go down
			if (selected < 4) {
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_C,
						ESPL_Pin_Button_C) == 0) {
					debounceFlagC = 0;
				}
				if (debounceFlagC == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_C,
								ESPL_Pin_Button_C) == 1) {
					debounceFlagC = 1;
					selected++;
					if (selected >= 4) {
						selected = 3;
					}
				}
			}

			// find the selected item
			switch (selected) {
			case STATE_ONE:
				pressed = 1;
				// Draw high score places Player1
				gdispFillArea(65, 15 , 80, 42, Yellow);
				gdispFillArc(65, 35, 20, 90, 270, Yellow);
				gdispFillArc(145, 35, 20, 270, 90, Yellow);
				sprintf(str, "High score 1: %d", highScore1);
				gdispDrawString(60, 30, str, font1, Red);

				//Draw high score places Player2
				gdispFillArea(190, 15 , 80, 42, Yellow);
				gdispFillArc(190, 35, 20, 90, 270, Yellow);
				gdispFillArc(270, 35, 20, 270, 90, Yellow);
				sprintf(str, "High score 2: %d", highScore2);
				gdispDrawString(190, 30, str, font1, Red);

				// Draw the first button (Selected)
				gdispFillArea(80, 60, 160, 40, backgroundColor);
				gdispFillArc(80, 80, 20, 90, 270, backgroundColor);
				gdispFillArc(240, 80, 20, 270, 90, backgroundColor);
				gdispDrawArc(240, 80, 20, 270, 90, Color);
				gdispDrawLine(80, 60, 240, 60, Color);
				gdispDrawLine(80, 100, 240, 100, Color);
				gdispDrawArc(80, 80, 20, 90, 270, Color);
				sprintf(str, "Single Player");
				gdispDrawString(120, 75, str, font1, Color);

				// Draw the second button

				gdispFillArea(80, 105, 160, 40, Cyan);
				gdispFillArc(80, 125, 20, 90, 270, Cyan);
				gdispFillArc(240, 125, 20, 270, 90, Cyan);
				gdispDrawArc(240, 125, 20, 270, 90, Magenta);
				gdispDrawLine(80, 105, 240, 105, Magenta);
				gdispDrawLine(80, 145, 240, 145, Magenta);
				gdispDrawArc(80, 125, 20, 90, 270, Magenta);
				sprintf(str, "Double player");
				gdispDrawString(120, 120, str, font1, Magenta);

				// Draw the Third button
				gdispFillArea(80, 150, 160, 40, Cyan);
				gdispFillArc(80, 170, 20, 90, 270, Cyan);
				gdispFillArc(240, 170, 20, 270, 90, Cyan);
				gdispDrawArc(240, 170, 20, 270, 90, Magenta);
				gdispDrawLine(80, 150, 240, 150, Magenta);
				gdispDrawLine(80, 190, 240, 190, Magenta);
				gdispDrawArc(80, 170, 20, 90, 270, Magenta);
				sprintf(str, "Setting");
				gdispDrawString(140, 165, str, font1, Magenta);
				break;
			case STATE_TWO:
				pressed = 2;

				// Draw high score places Player1
				gdispFillArea(65, 15 , 80, 42, Yellow);
				gdispFillArc(65, 35, 20, 90, 270, Yellow);
				gdispFillArc(145, 35, 20, 270, 90, Yellow);
				sprintf(str, "High score 1: %d", highScore1);
				gdispDrawString(60, 30, str, font1, Red);

				//Draw high score places Player2
				gdispFillArea(190, 15 , 80, 42, Yellow);
				gdispFillArc(190, 35, 20, 90, 270, Yellow);
				gdispFillArc(270, 35, 20, 270, 90, Yellow);
				sprintf(str, "High score 2: %d", highScore2);
				gdispDrawString(190, 30, str, font1, Red);

				// Draw the first button
				gdispFillArea(80, 60, 160, 40, Cyan);
				gdispFillArc(80, 80, 20, 90, 270, Cyan);
				gdispFillArc(240, 80, 20, 270, 90, Cyan);
				gdispDrawArc(240, 80, 20, 270, 90, Magenta);
				gdispDrawLine(80, 60, 240, 60, Magenta);
				gdispDrawLine(80, 100, 240, 100, Magenta);
				gdispDrawArc(80, 80, 20, 90, 270, Magenta);
				sprintf(str, "Single Player");
				gdispDrawString(120, 75, str, font1, Magenta);

				// Draw the second button(Selected)

				gdispFillArea(80, 105, 160, 40, backgroundColor);
				gdispFillArc(80, 125, 20, 90, 270, backgroundColor);
				gdispFillArc(240, 125, 20, 270, 90, backgroundColor);
				gdispDrawArc(240, 125, 20, 270, 90, Color);
				gdispDrawLine(80, 105, 240, 105, Color);
				gdispDrawLine(80, 145, 240, 145, Color);
				gdispDrawArc(80, 125, 20, 90, 270, Color);
				sprintf(str, "Double player");
				gdispDrawString(120, 120, str, font1, Color);

				// Draw the Third button
				gdispFillArea(80, 150, 160, 40, Cyan);
				gdispFillArc(80, 170, 20, 90, 270, Cyan);
				gdispFillArc(240, 170, 20, 270, 90, Cyan);
				gdispDrawArc(240, 170, 20, 270, 90, Magenta);
				gdispDrawLine(80, 150, 240, 150, Magenta);
				gdispDrawLine(80, 190, 240, 190, Magenta);
				gdispDrawArc(80, 170, 20, 90, 270, Magenta);
				sprintf(str, "Setting");
				gdispDrawString(140, 165, str, font1, Magenta);
				break;
			case STATE_THREE:
				pressed = 3;

				// Draw high score places Player1
				gdispFillArea(65, 15 , 80, 42, Yellow);
				gdispFillArc(65, 35, 20, 90, 270, Yellow);
				gdispFillArc(145, 35, 20, 270, 90, Yellow);
				sprintf(str, "High score 1: %d", highScore1);
				gdispDrawString(60, 30, str, font1, Red);

				//Draw high score places Player2
				gdispFillArea(190, 15 , 80, 42, Yellow);
				gdispFillArc(190, 35, 20, 90, 270, Yellow);
				gdispFillArc(270, 35, 20, 270, 90, Yellow);
				sprintf(str, "High score 2: %d", highScore2);
				gdispDrawString(190, 30, str, font1, Red);

				// Draw the first button
				gdispFillArea(80, 60, 160, 40, Cyan);
				gdispFillArc(80, 80, 20, 90, 270, Cyan);
				gdispFillArc(240, 80, 20, 270, 90, Cyan);
				gdispDrawArc(240, 80, 20, 270, 90, Magenta);
				gdispDrawLine(80, 60, 240, 60, Magenta);
				gdispDrawLine(80, 100, 240, 100, Magenta);
				gdispDrawArc(80, 80, 20, 90, 270, Magenta);
				sprintf(str, "Single Player");
				gdispDrawString(120, 75, str, font1, Magenta);

				// Draw the second button

				gdispFillArea(80, 105, 160, 40, Cyan);
				gdispFillArc(80, 125, 20, 90, 270, Cyan);
				gdispFillArc(240, 125, 20, 270, 90, Cyan);
				gdispDrawArc(240, 125, 20, 270, 90, Magenta);
				gdispDrawLine(80, 105, 240, 105, Magenta);
				gdispDrawLine(80, 145, 240, 145, Magenta);
				gdispDrawArc(80, 125, 20, 90, 270, Magenta);
				sprintf(str, "Double player");
				gdispDrawString(120, 120, str, font1, Magenta);

				// Draw the Third button(Selected)
				gdispFillArea(80, 150, 160, 40, backgroundColor);
				gdispFillArc(80, 170, 20, 90, 270, backgroundColor);
				gdispFillArc(240, 170, 20, 270, 90, backgroundColor);
				gdispDrawArc(240, 170, 20, 270, 90, Color);
				gdispDrawLine(80, 150, 240, 150, Color);
				gdispDrawLine(80, 190, 240, 190, Color);
				gdispDrawArc(80, 170, 20, 90, 270, Color);
				sprintf(str, "Setting");
				gdispDrawString(140, 165, str, font1, Color);
				break;

			default:
				break;
			}
			// write the use way
			sprintf(str, "A : Up.  C : Down.  E : Select");
			gdispDrawString(80, 200, str, font1, Red);
			sprintf(str, "E from any window to come back to the menu");
			gdispDrawString(35, 215, str, font1, Red);




			// control the pressed item to move to other window
			if (pressed == 1) {
				const unsigned char next_state_signal = NEXT_TASK;
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_E,
						ESPL_Pin_Button_E) == 0) {
					debounceFlagE = 0;
				}
				if (debounceFlagE == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_E,
								ESPL_Pin_Button_E) == 1) {
					debounceFlagE = 1;
					xQueueSend(StateQueue, &next_state_signal, 100);
				}

			} else if (pressed == 2) {
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_E,
						ESPL_Pin_Button_E) == 0) {
					debounceFlagE = 0;
				}
				if (debounceFlagE == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_E,
								ESPL_Pin_Button_E) == 1) {
					debounceFlagE = 1;

				}

			} else if (pressed == 3) {
				const unsigned char next_state_signal = SETTING;
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_E,
						ESPL_Pin_Button_E) == 0) {
					debounceFlagE = 0;
				}
				if (debounceFlagE == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_E,
								ESPL_Pin_Button_E) == 1) {
					debounceFlagE = 1;
					xQueueSend(StateQueue, &next_state_signal, 100);
				}

			}
		//	xSemaphoreGive(DrawLock);
		}
	}

}
//****
// set the setting and variables
//****
void setting(void * params) {
	char str[100]; // buffer for messages to draw to display
	struct buttons buttonStatus; // joystick queue input buffer
	const unsigned char next_state_signal = MENU;
	int debounceFlagA = 1;
	int debounceFlagB = 1;
	int debounceFlagC = 1;
	int debounceFlagD = 1;
	int debounceFlagE = 1;
	int pressed = 1;
	int selected = 1;
	int backgroundColor = White;
	int Color = Green;

	while (1) {

		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready

			while (xQueueReceive(ButtonQueue, &buttonStatus, 0) == pdTRUE)
				;

			// change background color
			gdispFillArea(0, 0, 320, 240, Black);
			// control go back to the menu use E
			if (GPIO_ReadInputDataBit(ESPL_Register_Button_E, ESPL_Pin_Button_E)
					== 0) {
				debounceFlagE = 0;
			}
			if (debounceFlagE == 0
					&& GPIO_ReadInputDataBit(ESPL_Register_Button_E,
							ESPL_Pin_Button_E) == 1) {
				debounceFlagE = 1;
				xQueueSend(StateQueue, &next_state_signal, 100);
			}

			// if A pressed go up
			if (selected > 0) {
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_A,
						ESPL_Pin_Button_A) == 0) {
					debounceFlagA = 0;
				}
				if (debounceFlagA == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_A,
								ESPL_Pin_Button_A) == 1) {
					debounceFlagA = 1;
					selected--;
					if (selected <= 0) {
						selected = 1;
					}
				}
			}	// if C pressed go down

			if (selected < 7) {
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_C,
						ESPL_Pin_Button_C) == 0) {
					debounceFlagC = 0;
				}
				if (debounceFlagC == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_C,
								ESPL_Pin_Button_C) == 1) {
					debounceFlagC = 1;
					selected++;
					if (selected >= 7) {
						selected = 6;
					}
				}
			}

			// find the selected item
			switch (selected) {
			case STATE_ONE:
				pressed = 1;

				// Draw the first button (Selected)
				gdispFillArea(130, 60, 60, 20, backgroundColor);
				gdispFillArc(130, 70, 10, 90, 270, backgroundColor);
				gdispFillArc(190, 70, 10, 270, 90, backgroundColor);
				gdispDrawArc(190, 70, 10, 270, 90, Color);
				gdispDrawLine(130, 60, 190, 60, Color);
				gdispDrawLine(130, 80, 190, 80, Color);
				gdispDrawArc(130, 70, 10, 90, 270, Color);
				sprintf(str, "live: %d", live);
				gdispDrawString(145, 65, str, font, Color);

				// Draw the second button

				gdispFillArea(130, 85, 60, 20, Yellow);
				gdispFillArc(130, 95, 10, 90, 270, Yellow);
				gdispFillArc(190, 95, 10, 270, 90, Yellow);
				gdispDrawArc(190, 95, 10, 270, 90, Red);
				gdispDrawLine(130, 85, 190, 85, Red);
				gdispDrawLine(130, 105, 190, 105, Red);
				gdispDrawArc(130, 95, 10, 90, 270, Red);
				sprintf(str, "score: %d", score);
				gdispDrawString(135, 90, str, font1, Red);

				// Draw the Third button
				gdispFillArea(130, 110, 60, 20, Yellow);
				gdispFillArc(130, 120, 10, 90, 270, Yellow);
				gdispFillArc(190, 120, 10, 270, 90, Yellow);
				gdispDrawArc(190, 120, 10, 270, 90, Red);
				gdispDrawLine(130, 110, 190, 110, Red);
				gdispDrawLine(130, 130, 190, 130, Red);
				gdispDrawArc(130, 120, 10, 90, 270, Red);
				sprintf(str, "power: %d", power);
				gdispDrawString(135, 115, str, font1, Red);

				// Draw the Fourth button
				gdispFillArea(130, 135, 60, 20, Yellow);
				gdispFillArc(130, 145, 10, 90, 270, Yellow);
				gdispFillArc(190, 145, 10, 270, 90, Yellow);
				gdispDrawArc(190, 145, 10, 270, 90, Red);
				gdispDrawLine(130, 135, 190, 135, Red);
				gdispDrawLine(130, 155, 190, 155, Red);
				gdispDrawArc(130, 145, 10, 90, 270, Red);
				sprintf(str, "level: %d", level);
				gdispDrawString(135, 140, str, font1, Red);

				// Draw the fifth button
				gdispFillArea(130, 160, 60, 20, Yellow);
				gdispFillArc(130, 170, 10, 90, 270, Yellow);
				gdispFillArc(190, 170, 10, 270, 90, Yellow);
				gdispDrawArc(190, 170, 10, 270, 90, Red);
				gdispDrawLine(130, 160, 190, 160, Red);
				gdispDrawLine(130, 180, 190, 180, Red);
				gdispDrawArc(130, 170, 10, 90, 270, Red);
				sprintf(str, "G.Speed: %d", Ghostspeed);
				gdispDrawString(135, 165, str, font1, Red);

				// Draw the sixth button
				gdispFillArea(130, 185, 60, 20, Yellow);
				gdispFillArc(130, 195, 10, 90, 270, Yellow);
				gdispFillArc(190, 195, 10, 270, 90, Yellow);
				gdispDrawArc(190, 195, 10, 270, 90, Red);
				gdispDrawLine(130, 185, 190, 185, Red);
				gdispDrawLine(130, 205 , 190, 205, Red);
				gdispDrawArc(130, 195, 10, 90, 270, Red);
				sprintf(str, "PacSpeed: %d", Pspeed);
				gdispDrawString(125, 190 , str, font1, Red);

				break;
			case STATE_TWO:
				pressed = 2;

				// Draw the first button (Selected)
				gdispFillArea(130, 60, 60, 20, Yellow);
				gdispFillArc(130, 70, 10, 90, 270, Yellow);
				gdispFillArc(190, 70, 10, 270, 90, Yellow);
				gdispDrawArc(190, 70, 10, 270, 90, Red);
				gdispDrawLine(130, 60, 190, 60, Red);
				gdispDrawLine(130, 80, 190, 80, Red);
				gdispDrawArc(130, 70, 10, 90, 270, Red);
				sprintf(str, "live: %d", live);
				gdispDrawString(145, 65, str, font, Red);

				// Draw the second button (selected)

				gdispFillArea(130, 85, 60, 20, backgroundColor);
				gdispFillArc(130, 95, 10, 90, 270, backgroundColor);
				gdispFillArc(190, 95, 10, 270, 90, backgroundColor);
				gdispDrawArc(190, 95, 10, 270, 90, Color);
				gdispDrawLine(130, 85, 190, 85, Color);
				gdispDrawLine(130, 105, 190, 105, Color);
				gdispDrawArc(130, 95, 10, 90, 270, Color);
				sprintf(str, "score: %d", score);
				gdispDrawString(135, 90, str, font1, Color);

				// Draw the Third button
				gdispFillArea(130, 110, 60, 20, Yellow);
				gdispFillArc(130, 120, 10, 90, 270, Yellow);
				gdispFillArc(190, 120, 10, 270, 90, Yellow);
				gdispDrawArc(190, 120, 10, 270, 90, Red);
				gdispDrawLine(130, 110, 190, 110, Red);
				gdispDrawLine(130, 130, 190, 130, Red);
				gdispDrawArc(130, 120, 10, 90, 270, Red);
				sprintf(str, "power: %d", power);
				gdispDrawString(135, 115, str, font1, Red);

				// Draw the Fourth button
				gdispFillArea(130, 135, 60, 20, Yellow);
				gdispFillArc(130, 145, 10, 90, 270, Yellow);
				gdispFillArc(190, 145, 10, 270, 90, Yellow);
				gdispDrawArc(190, 145, 10, 270, 90, Red);
				gdispDrawLine(130, 135, 190, 135, Red);
				gdispDrawLine(130, 155, 190, 155, Red);
				gdispDrawArc(130, 145, 10, 90, 270, Red);
				sprintf(str, "level: %d", level);
				gdispDrawString(135, 140, str, font1, Red);

				// Draw the fifth button
				gdispFillArea(130, 160, 60, 20, Yellow);
				gdispFillArc(130, 170, 10, 90, 270, Yellow);
				gdispFillArc(190, 170, 10, 270, 90, Yellow);
				gdispDrawArc(190, 170, 10, 270, 90, Red);
				gdispDrawLine(130, 160, 190, 160, Red);
				gdispDrawLine(130, 180, 190, 180, Red);
				gdispDrawArc(130, 170, 10, 90, 270, Red);
				sprintf(str, "G.Speed: %d", Ghostspeed);
				gdispDrawString(135, 165, str, font1, Red);

				// Draw the sixth button
				gdispFillArea(130, 185, 60, 20, Yellow);
				gdispFillArc(130, 195, 10, 90, 270, Yellow);
				gdispFillArc(190, 195, 10, 270, 90, Yellow);
				gdispDrawArc(190, 195, 10, 270, 90, Red);
				gdispDrawLine(130, 185, 190, 185, Red);
				gdispDrawLine(130, 205 , 190, 205, Red);
				gdispDrawArc(130, 195, 10, 90, 270, Red);
				sprintf(str, "PacSpeed: %d", Pspeed);
				gdispDrawString(125, 190 , str, font1, Red);

				break;
			case STATE_THREE:
				pressed = 3;

				// Draw the first button
				gdispFillArea(130, 60, 60, 20, Yellow);
				gdispFillArc(130, 70, 10, 90, 270, Yellow);
				gdispFillArc(190, 70, 10, 270, 90, Yellow);
				gdispDrawArc(190, 70, 10, 270, 90, Red);
				gdispDrawLine(130, 60, 190, 60, Red);
				gdispDrawLine(130, 80, 190, 80, Red);
				gdispDrawArc(130, 70, 10, 90, 270, Red);
				sprintf(str, "live: %d", live);
				gdispDrawString(145, 65, str, font, Red);

				// Draw the second button

				gdispFillArea(130, 85, 60, 20, Yellow);
				gdispFillArc(130, 95, 10, 90, 270, Yellow);
				gdispFillArc(190, 95, 10, 270, 90, Yellow);
				gdispDrawArc(190, 95, 10, 270, 90, Red);
				gdispDrawLine(130, 85, 190, 85, Red);
				gdispDrawLine(130, 105, 190, 105, Red);
				gdispDrawArc(130, 95, 10, 90, 270, Red);
				sprintf(str, "score: %d", score);
				gdispDrawString(135, 90, str, font1, Red);

				// Draw the Third button (selected)
				gdispFillArea(130, 110, 60, 20, backgroundColor);
				gdispFillArc(130, 120, 10, 90, 270, backgroundColor);
				gdispFillArc(190, 120, 10, 270, 90, backgroundColor);
				gdispDrawArc(190, 120, 10, 270, 90, Color);
				gdispDrawLine(130, 110, 190, 110, Color);
				gdispDrawLine(130, 130, 190, 130, Color);
				gdispDrawArc(130, 120, 10, 90, 270, Color);
				sprintf(str, "power: %d", power);
				gdispDrawString(135, 115, str, font1, Color);

				// Draw the Fourth button
				gdispFillArea(130, 135, 60, 20, Yellow);
				gdispFillArc(130, 145, 10, 90, 270, Yellow);
				gdispFillArc(190, 145, 10, 270, 90, Yellow);
				gdispDrawArc(190, 145, 10, 270, 90, Red);
				gdispDrawLine(130, 135, 190, 135, Red);
				gdispDrawLine(130, 155, 190, 155, Red);
				gdispDrawArc(130, 145, 10, 90, 270, Red);
				sprintf(str, "level: %d", level);
				gdispDrawString(135, 140, str, font1, Red);

				// Draw the fifth button
				gdispFillArea(130, 160, 60, 20, Yellow);
				gdispFillArc(130, 170, 10, 90, 270, Yellow);
				gdispFillArc(190, 170, 10, 270, 90, Yellow);
				gdispDrawArc(190, 170, 10, 270, 90, Red);
				gdispDrawLine(130, 160, 190, 160, Red);
				gdispDrawLine(130, 180, 190, 180, Red);
				gdispDrawArc(130, 170, 10, 90, 270, Red);
				sprintf(str, "G.Speed: %d", Ghostspeed);
				gdispDrawString(135, 165, str, font1, Red);

				// Draw the sixth button
				gdispFillArea(130, 185, 60, 20, Yellow);
				gdispFillArc(130, 195, 10, 90, 270, Yellow);
				gdispFillArc(190, 195, 10, 270, 90, Yellow);
				gdispDrawArc(190, 195, 10, 270, 90, Red);
				gdispDrawLine(130, 185, 190, 185, Red);
				gdispDrawLine(130, 205 , 190, 205, Red);
				gdispDrawArc(130, 195, 10, 90, 270, Red);
				sprintf(str, "PacSpeed: %d", Pspeed);
				gdispDrawString(125, 190 , str, font1, Red);
				break;
			case 4:
				pressed = 4;

				// Draw the first button
				gdispFillArea(130, 60, 60, 20, Yellow);
				gdispFillArc(130, 70, 10, 90, 270, Yellow);
				gdispFillArc(190, 70, 10, 270, 90, Yellow);
				gdispDrawArc(190, 70, 10, 270, 90, Red);
				gdispDrawLine(130, 60, 190, 60, Red);
				gdispDrawLine(130, 80, 190, 80, Red);
				gdispDrawArc(130, 70, 10, 90, 270, Red);
				sprintf(str, "live: %d", live);
				gdispDrawString(145, 65, str, font, Red);

				// Draw the second button

				gdispFillArea(130, 85, 60, 20, Yellow);
				gdispFillArc(130, 95, 10, 90, 270, Yellow);
				gdispFillArc(190, 95, 10, 270, 90, Yellow);
				gdispDrawArc(190, 95, 10, 270, 90, Red);
				gdispDrawLine(130, 85, 190, 85, Red);
				gdispDrawLine(130, 105, 190, 105, Red);
				gdispDrawArc(130, 95, 10, 90, 270, Red);
				sprintf(str, "score: %d", score);
				gdispDrawString(135, 90, str, font1, Red);

				// Draw the Third button
				gdispFillArea(130, 110, 60, 20, Yellow);
				gdispFillArc(130, 120, 10, 90, 270, Yellow);
				gdispFillArc(190, 120, 10, 270, 90, Yellow);
				gdispDrawArc(190, 120, 10, 270, 90, Red);
				gdispDrawLine(130, 110, 190, 110, Red);
				gdispDrawLine(130, 130, 190, 130, Red);
				gdispDrawArc(130, 120, 10, 90, 270, Red);
				sprintf(str, "power: %d", power);
				gdispDrawString(135, 115, str, font1, Red);

				// Draw the Fourth button (selected
				gdispFillArea(130, 135, 60, 20, backgroundColor);
				gdispFillArc(130, 145, 10, 90, 270, backgroundColor);
				gdispFillArc(190, 145, 10, 270, 90, backgroundColor);
				gdispDrawArc(190, 145, 10, 270, 90, Color);
				gdispDrawLine(130, 135, 190, 135, Color);
				gdispDrawLine(130, 155, 190, 155, Color);
				gdispDrawArc(130, 145, 10, 90, 270, Color);
				sprintf(str, "level: %d", level);
				gdispDrawString(135, 140, str, font1, Color);

				// Draw the fifth button
				gdispFillArea(130, 160, 60, 20, Yellow);
				gdispFillArc(130, 170, 10, 90, 270, Yellow);
				gdispFillArc(190, 170, 10, 270, 90, Yellow);
				gdispDrawArc(190, 170, 10, 270, 90, Red);
				gdispDrawLine(130, 160, 190, 160, Red);
				gdispDrawLine(130, 180, 190, 180, Red);
				gdispDrawArc(130, 170, 10, 90, 270, Red);
				sprintf(str, "G.Speed: %d", Ghostspeed);
				gdispDrawString(135, 165, str, font1, Red);

				// Draw the sixth button
				gdispFillArea(130, 185, 60, 20, Yellow);
				gdispFillArc(130, 195, 10, 90, 270, Yellow);
				gdispFillArc(190, 195, 10, 270, 90, Yellow);
				gdispDrawArc(190, 195, 10, 270, 90, Red);
				gdispDrawLine(130, 185, 190, 185, Red);
				gdispDrawLine(130, 205 , 190, 205, Red);
				gdispDrawArc(130, 195, 10, 90, 270, Red);
				sprintf(str, "PacSpeed: %d", Pspeed);
				gdispDrawString(125, 190 , str, font1, Red);

			break;
			case 5:
				pressed = 5;

				// Draw the first button
				gdispFillArea(130, 60, 60, 20, Yellow);
				gdispFillArc(130, 70, 10, 90, 270, Yellow);
				gdispFillArc(190, 70, 10, 270, 90, Yellow);
				gdispDrawArc(190, 70, 10, 270, 90, Red);
				gdispDrawLine(130, 60, 190, 60, Red);
				gdispDrawLine(130, 80, 190, 80, Red);
				gdispDrawArc(130, 70, 10, 90, 270, Red);
				sprintf(str, "live: %d", live);
				gdispDrawString(145, 65, str, font, Red);

				// Draw the second button

				gdispFillArea(130, 85, 60, 20, Yellow);
				gdispFillArc(130, 95, 10, 90, 270, Yellow);
				gdispFillArc(190, 95, 10, 270, 90, Yellow);
				gdispDrawArc(190, 95, 10, 270, 90, Red);
				gdispDrawLine(130, 85, 190, 85, Red);
				gdispDrawLine(130, 105, 190, 105, Red);
				gdispDrawArc(130, 95, 10, 90, 270, Red);
				sprintf(str, "score: %d", score);
				gdispDrawString(135, 90, str, font1, Red);

				// Draw the Third button
				gdispFillArea(130, 110, 60, 20, Yellow);
				gdispFillArc(130, 120, 10, 90, 270, Yellow);
				gdispFillArc(190, 120, 10, 270, 90, Yellow);
				gdispDrawArc(190, 120, 10, 270, 90, Red);
				gdispDrawLine(130, 110, 190, 110, Red);
				gdispDrawLine(130, 130, 190, 130, Red);
				gdispDrawArc(130, 120, 10, 90, 270, Red);
				sprintf(str, "power: %d", power);
				gdispDrawString(135, 115, str, font1, Red);

				// Draw the Fourth button
				gdispFillArea(130, 135, 60, 20, Yellow);
				gdispFillArc(130, 145, 10, 90, 270, Yellow);
				gdispFillArc(190, 145, 10, 270, 90, Yellow);
				gdispDrawArc(190, 145, 10, 270, 90, Red);
				gdispDrawLine(130, 135, 190, 135, Red);
				gdispDrawLine(130, 155, 190, 155, Red);
				gdispDrawArc(130, 145, 10, 90, 270, Red);
				sprintf(str, "level: %d", level);
				gdispDrawString(135, 140, str, font1, Red);

				// Draw the fifth button (selected)
				gdispFillArea(130, 160, 60, 20, backgroundColor);
				gdispFillArc(130, 170, 10, 90, 270, backgroundColor);
				gdispFillArc(190, 170, 10, 270, 90, backgroundColor);
				gdispDrawArc(190, 170, 10, 270, 90, Color);
				gdispDrawLine(130, 160, 190, 160, Color);
				gdispDrawLine(130, 180, 190, 180, Color);
				gdispDrawArc(130, 170, 10, 90, 270, Color);
				sprintf(str, "G.Speed: %d", Ghostspeed);
				gdispDrawString(135, 165, str, font1, Color);

				// Draw the sixth button
				gdispFillArea(130, 185, 60, 20, Yellow);
				gdispFillArc(130, 195, 10, 90, 270, Yellow);
				gdispFillArc(190, 195, 10, 270, 90, Yellow);
				gdispDrawArc(190, 195, 10, 270, 90, Red);
				gdispDrawLine(130, 185, 190, 185, Red);
				gdispDrawLine(130, 205 , 190, 205, Red);
				gdispDrawArc(130, 195, 10, 90, 270, Red);
				sprintf(str, "PacSpeed: %d", Pspeed);
				gdispDrawString(125, 190 , str, font1, Red);


			break;
			case 6:
				pressed = 6;

				// Draw the first button
				gdispFillArea(130, 60, 60, 20, Yellow);
				gdispFillArc(130, 70, 10, 90, 270, Yellow);
				gdispFillArc(190, 70, 10, 270, 90, Yellow);
				gdispDrawArc(190, 70, 10, 270, 90, Red);
				gdispDrawLine(130, 60, 190, 60, Red);
				gdispDrawLine(130, 80, 190, 80, Red);
				gdispDrawArc(130, 70, 10, 90, 270, Red);
				sprintf(str, "live: %d", live);
				gdispDrawString(145, 65, str, font, Red);

				// Draw the second button

				gdispFillArea(130, 85, 60, 20, Yellow);
				gdispFillArc(130, 95, 10, 90, 270, Yellow);
				gdispFillArc(190, 95, 10, 270, 90, Yellow);
				gdispDrawArc(190, 95, 10, 270, 90, Red);
				gdispDrawLine(130, 85, 190, 85, Red);
				gdispDrawLine(130, 105, 190, 105, Red);
				gdispDrawArc(130, 95, 10, 90, 270, Red);
				sprintf(str, "score: %d", score);
				gdispDrawString(135, 90, str, font1, Red);

				// Draw the Third button (selected)
				gdispFillArea(130, 110, 60, 20, Yellow);
				gdispFillArc(130, 120, 10, 90, 270, Yellow);
				gdispFillArc(190, 120, 10, 270, 90, Yellow);
				gdispDrawArc(190, 120, 10, 270, 90, Red);
				gdispDrawLine(130, 110, 190, 110, Red);
				gdispDrawLine(130, 130, 190, 130, Red);
				gdispDrawArc(130, 120, 10, 90, 270, Red);
				sprintf(str, "power: %d", power);
				gdispDrawString(135, 115, str, font1, Red);

				// Draw the Fourth button
				gdispFillArea(130, 135, 60, 20, Yellow);
				gdispFillArc(130, 145, 10, 90, 270, Yellow);
				gdispFillArc(190, 145, 10, 270, 90, Yellow);
				gdispDrawArc(190, 145, 10, 270, 90, Red);
				gdispDrawLine(130, 135, 190, 135, Red);
				gdispDrawLine(130, 155, 190, 155, Red);
				gdispDrawArc(130, 145, 10, 90, 270, Red);
				sprintf(str, "level: %d", level);
				gdispDrawString(135, 140, str, font1, Red);

				// Draw the fifth button
				gdispFillArea(130, 160, 60, 20, Yellow);
				gdispFillArc(130, 170, 10, 90, 270, Yellow);
				gdispFillArc(190, 170, 10, 270, 90, Yellow);
				gdispDrawArc(190, 170, 10, 270, 90, Red);
				gdispDrawLine(130, 160, 190, 160, Red);
				gdispDrawLine(130, 180, 190, 180, Red);
				gdispDrawArc(130, 170, 10, 90, 270, Red);
				sprintf(str, "G.Speed: %d", Ghostspeed);
				gdispDrawString(135, 165, str, font1, Red);

				// Draw the sixth button (selected)
				gdispFillArea(130, 185, 60, 20, backgroundColor);
				gdispFillArc(130, 195, 10, 90, 270, backgroundColor);
				gdispFillArc(190, 195, 10, 270, 90, backgroundColor);
				gdispDrawArc(190, 195, 10, 270, 90, Color);
				gdispDrawLine(130, 185, 190, 185, Color);
				gdispDrawLine(130, 205 , 190, 205, Color);
				gdispDrawArc(130, 195, 10, 90, 270, Color);
				sprintf(str, "PacSpeed: %d", Pspeed);
				gdispDrawString(125, 190 , str, font1, Color);


			break;
			default:
				break;
			}

			// control the selected variable to be more with pressing D and less with pressing B
			if (pressed == 1) {
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_D,
						ESPL_Pin_Button_D) == 0) {
					debounceFlagD = 0;
				}
				if (debounceFlagD == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_D,
								ESPL_Pin_Button_D) == 1) {
					debounceFlagD = 1;
					live++;
				}
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_B,
						ESPL_Pin_Button_B) == 0) {
					debounceFlagB = 0;
				}
				if (debounceFlagB == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_B,
								ESPL_Pin_Button_B) == 1) {
					debounceFlagB = 1;
					live--;
				}
			} else if (pressed == 2) {
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_D,
						ESPL_Pin_Button_D) == 0) {
					debounceFlagD = 0;
				}
				if (debounceFlagD == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_D,
								ESPL_Pin_Button_D) == 1) {
					debounceFlagD = 1;
					score = score + 10;
				}
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_B,
						ESPL_Pin_Button_B) == 0) {
					debounceFlagB = 0;
				}
				if (debounceFlagB == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_B,
								ESPL_Pin_Button_B) == 1) {
					debounceFlagB = 1;
					score = score - 10;

				}
			}else if(pressed == 3){

				if (GPIO_ReadInputDataBit(ESPL_Register_Button_D,
						ESPL_Pin_Button_D) == 0) {
					debounceFlagD = 0;
				}
				if (debounceFlagD == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_D,
								ESPL_Pin_Button_D) == 1) {
					debounceFlagD = 1;
					power++;
				}
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_B,
						ESPL_Pin_Button_B) == 0) {
					debounceFlagB = 0;
				}
				if (debounceFlagB == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_B,
								ESPL_Pin_Button_B) == 1) {
					debounceFlagB = 1;
					power--;

				}
			}else if(pressed == 4){

				if (GPIO_ReadInputDataBit(ESPL_Register_Button_D,
						ESPL_Pin_Button_D) == 0) {
					debounceFlagD = 0;
				}
				if (debounceFlagD == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_D,
								ESPL_Pin_Button_D) == 1) {
					debounceFlagD = 1;
					level ++;
					if(level == 4){
						level = 3;
					}
				}
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_B,
						ESPL_Pin_Button_B) == 0) {
					debounceFlagB = 0;
				}
				if (debounceFlagB == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_B,
								ESPL_Pin_Button_B) == 1) {
					debounceFlagB = 1;
					level --;
					if (level ==0){
						level =1;
					}

				}
			}else if(pressed == 5){

				if (GPIO_ReadInputDataBit(ESPL_Register_Button_D,
						ESPL_Pin_Button_D) == 0) {
					debounceFlagD = 0;
				}
				if (debounceFlagD == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_D,
								ESPL_Pin_Button_D) == 1) {
					debounceFlagD = 1;
					Ghostspeed ++;
					if(Ghostspeed == 11){
						Ghostspeed = 10;
					}
				}
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_B,
						ESPL_Pin_Button_B) == 0) {
					debounceFlagB = 0;
				}
				if (debounceFlagB == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_B,
								ESPL_Pin_Button_B) == 1) {
					debounceFlagB = 1;
					Ghostspeed --;
					if (Ghostspeed ==0){
						Ghostspeed =1;
					}

				}
			}else if(pressed == 6){

				if (GPIO_ReadInputDataBit(ESPL_Register_Button_D,
						ESPL_Pin_Button_D) == 0) {
					debounceFlagD = 0;
				}
				if (debounceFlagD == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_D,
								ESPL_Pin_Button_D) == 1) {
					debounceFlagD = 1;
					Pspeed ++;
					if(Pspeed == 11){
						Pspeed = 10;
					}
				}
				if (GPIO_ReadInputDataBit(ESPL_Register_Button_B,
						ESPL_Pin_Button_B) == 0) {
					debounceFlagB = 0;
				}
				if (debounceFlagB == 0
						&& GPIO_ReadInputDataBit(ESPL_Register_Button_B,
								ESPL_Pin_Button_B) == 1) {
					debounceFlagB = 1;
					Pspeed --;
					if (Pspeed ==0){
						Pspeed =1;
					}

				}
			}



		}
	}
}


//******
// lose window
//*****
void lose(void * params) {

	char str[100]; // buffer for messages to draw to display
	struct buttons buttonStatus;
	int debounceFlagD = 1;
	int debounceFlagE = 1;
	while (1) {

		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready
			while (xQueueReceive(ButtonQueue, &buttonStatus, 0) == pdTRUE)
				;
			// change background color
			gdispFillArea(0, 0, 320, 240, Black);
			sprintf(str, "You Lose!");
			gdispDrawString(140, 100, str, font1, Red);
			sprintf(str, "Click E to go to Menu");
			gdispDrawString(105, 120, str, font1, Red);
			sprintf(str, "Click D to go to play again");
			gdispDrawString(90, 140, str, font1, Red);
			if (GPIO_ReadInputDataBit(ESPL_Register_Button_E, ESPL_Pin_Button_E)
					== 0) {
				debounceFlagE = 0;
			}
			if (debounceFlagE == 0
					&& GPIO_ReadInputDataBit(ESPL_Register_Button_E,
							ESPL_Pin_Button_E) == 1) {
				debounceFlagE = 1;

				const unsigned char	next_state_signal = MENU;
				xQueueSend(StateQueue, &next_state_signal, 100);
			}

			if (GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D)
					== 0) {
				debounceFlagD = 0;
			}
			if (debounceFlagD == 0
					&& GPIO_ReadInputDataBit(ESPL_Register_Button_D,
							ESPL_Pin_Button_D) == 1) {
				debounceFlagD = 1;
				const unsigned char next_state_signal = FIELD;
				xQueueSend(StateQueue, &next_state_signal, 100);
			}
		}
	}
}
//*******
///Pause window
/****///
void pause(void * params) {

	char str[100]; // buffer for messages to draw to display
	struct buttons buttonStatus;
	int debounceFlagA = 1;
	int debounceFlagE = 1;
	while (1) {

		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready
			while (xQueueReceive(ButtonQueue, &buttonStatus, 0) == pdTRUE)
				;
			// change background color
			gdispFillArea(0, 0, 320, 240, Black);
			sprintf(str, "Paused!");
			gdispDrawString(140, 100, str, font1, Green);
			sprintf(str, "Click A to go to play again");
			gdispDrawString(95, 120, str, font1, Green);
			if (GPIO_ReadInputDataBit(ESPL_Register_Button_E, ESPL_Pin_Button_E)
					== 0) {
				debounceFlagE = 0;
			}
			if (debounceFlagE == 0
					&& GPIO_ReadInputDataBit(ESPL_Register_Button_E,
							ESPL_Pin_Button_E) == 1) {
				debounceFlagE = 1;

				const unsigned char	next_state_signal = MENU;
				xQueueSend(StateQueue, &next_state_signal, 100);
			}

			if (GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A)
					== 0) {
				debounceFlagA = 0;
			}
			if (debounceFlagA == 0
					&& GPIO_ReadInputDataBit(ESPL_Register_Button_A,
							ESPL_Pin_Button_A) == 1) {
				debounceFlagA = 1;
				const unsigned char next_state_signal = CONTINUE;
				xQueueSend(StateQueue, &next_state_signal, 100);
			}
		}
	}
}



//Draw the ready field
void draw (void * params){
	char str[100]; // buffer for messages to draw to display
	struct buttons buttonStatus; // joystick queue input buffer
	int Color = Blue;
	struct myPacMan PacMan ={
	       {
	          .x = 112,
	          .y = 180,
	       },
	       .vx = 0,
	       .vy = 0,
	       .lives = 3,
	       .chasing = false,
		   .Speed = 1,
	       .food_collected = 0,
		   .animation = 0,
		   .color= Yellow,
		   .startMove = false,
		   .GOSX=0,
		   .GOSY=0,
			.nx =0,
			.ny=0
	     };

	struct myPacMan GhostRed ={
	       {
	          .x = 110,
	          .y = 115,
	       },
	       .vx = 0,
	       .vy = 0,
	       .lives = 3,
	       .chasing = false,
		   .Speed = 1,
	       .food_collected = 0,
		   .animation = 0,
		   .color = Red,
		   .startMove= false,
			.GOSX=0,
			.GOSY=0,
			.nx =0,
			.ny=0
	     };

	while (1) {

			if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready

				while (xQueueReceive(ButtonQueue, &buttonStatus, 0) == pdTRUE)
					;

				// State machine input

			DrawField(Color, true);
			gdispFillCircle(PacMan.position.x , PacMan.position.y , 5 , PacMan.color);
			gdispFillCircle(GhostRed.position.x  , GhostRed.position.y , 2 , GhostRed.color);
			sprintf(str, "Ready! " );
			gdispDrawString(95, 128 , str, font, Red);

			}
	}
}

void drawTask1(void * params) {
	char str[100]; // buffer for messages to draw to display
	struct buttons buttonStatus; // joystick queue input buffer
	unsigned char both_buffers = false;
#define displaySizeX  320
#define displaySizeY  240
	struct myPacMan PacMan ={
	       {
	          .x = 112,
	          .y = 180,
	       },
	       .vx = 0,
	       .vy = 0,
	       .lives = live,
	       .chasing = false,
		   .Speed = Pspeed,
	       .food_collected = score,
		   .animation = 0,
		   .color= Yellow,
		   .startMove = false,
		   .GOSX=0,
		   .GOSY=0,
			.nx =0,
			.ny=0
	     };

	struct myPacMan GhostRed ={
	       {
	          .x = 110,
	          .y = 115,
	       },
	       .vx = 0,
	       .vy = 0,
	       .lives = 3,
	       .chasing = false,
		   .Speed = Ghostspeed,
	       .food_collected = 0,
		   .animation = 0,
		   .color = Red,
		   .startMove= false,
			.GOSX=0,
			.GOSY=0,
			.nx =0,
			.ny=0
	     };
	int posX, posY , GhostX, GhostY, reGhostX = 110, reGhostY =115;
	float GhostcountX =0;
	static bool DoorOpen = false;
	int diffGhostX=0, diffGhostY =0, savedGX, savedGY;
	int Color =Blue;
	int debounceFlagE = 1, debounceFlagA = 1;
	int z=0;
	static bool escapeMode = false, Gchasing = false, Garrived = false, GhostSwitcher = false, switcher = false, StartMOVE = false;
	bool rTol = false, lTor = false;
	unsigned char refresh = false;
	while (1) {

		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready
//			xSemaphoreTake(DrawLock, portMAX_DELAY);

			while (xQueueReceive(ButtonQueue, &buttonStatus, 0) == pdTRUE)
				;
			// State machine input
			// control go back to the menu use E
			if (GPIO_ReadInputDataBit(ESPL_Register_Button_E, ESPL_Pin_Button_E)
					== 0) {
				debounceFlagE = 0;
			}
			if (debounceFlagE == 0
					&& GPIO_ReadInputDataBit(ESPL_Register_Button_E,
							ESPL_Pin_Button_E) == 1) {
				unsigned char next_state_signal = MENU;
				debounceFlagE = 1;
				xQueueSend(StateQueue, &next_state_signal, 100);
			}
			// make Paused task mit A

			if (GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A)
					== 0) {
				debounceFlagA = 0;
			}
			if (debounceFlagA == 0
					&& GPIO_ReadInputDataBit(ESPL_Register_Button_A,
							ESPL_Pin_Button_A) == 1) {
				unsigned char next_state_signal = PAUSED;
				debounceFlagA = 1;
				xQueueSend(StateQueue, &next_state_signal, 100);
			}


						// controll end game and go back actions
			if (PacMan.lives == 0) {
				GhostRed.startMove = false;
				GhostSwitcher = false;
				DoorOpen = false;
				unsigned char next_state_signal = LOSE;
				xQueueSend(StateQueue, &next_state_signal, 100);
			}
					if(restField == true){
						StartMOVE =false;
				GhostRed.startMove = false;
				GhostSwitcher = false;
				GhostRed.position.x = 110;
				GhostRed.position.y = 115;
				PacMan.position.x = 112;
				PacMan.position.y = 180;
				DoorOpen = false;
					}


					// draw the field from FieldOne.c

					//try and get semaphore
					if(xSemaphoreTake(RefreshScreen, 0) == pdTRUE){
							both_buffers = true;
							DrawField(Color, (volatile unsigned char)true);
					}else if(both_buffers){
						both_buffers = false;
						DrawField(Color, (volatile unsigned char)true);
					}
					else{
						DrawField(Color, (volatile unsigned char)false);
			}
			// check for refresh signal from state machine
			// set refresh

			//Conrtol the Player;
			if (buttonStatus.joystick.x >= 135) {
				PacMan.vx = PacMan.Speed;
				PacMan.vy = 0;
			} else if (buttonStatus.joystick.y >= 130) {
				PacMan.vx = 0;
				PacMan.vy = PacMan.Speed;
			} else if (buttonStatus.joystick.x <= 120) {
				PacMan.vx = -PacMan.Speed;
				PacMan.vy = 0;
			} else if (buttonStatus.joystick.y <= 110) {
				PacMan.vx = 0;
				PacMan.vy = -PacMan.Speed;
			}

			posY = PacMan.position.y + PacMan.vy;
			if (Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x - (2))
					/ 8] != '.'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != 'C'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != 'M'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != ':'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != 'K'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != '$'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != 'J'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != 'r'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != 'l'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != '&'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != '!') {
				PacMan.vy = 0;
			}
			if (Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x) / 8]
					!= '.'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x)
							/ 8] != 'C'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x)
							/ 8] != 'M'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x)
							/ 8] != ':'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x)
							/ 8] != 'K'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x)
							/ 8] != '$'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							+ (2)) / 8] != 'J'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != 'r'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != 'l'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != '&'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != '!') {
				PacMan.vy = 0;
			}
			if (Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x + (2))
					/ 8] != '.'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							+ (2)) / 8] != 'C'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							+ (2)) / 8] != 'M'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							+ (2)) / 8] != ':'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							+ (2)) / 8] != 'K'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							+ (2)) / 8] != '$'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							+ (2)) / 8] != 'J'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != 'r'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != 'l'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != '&'
					&& Field1[(posY + (3 * PacMan.vy)) / 8][(PacMan.position.x
							- (2)) / 8] != '!') {
				PacMan.vy = 0;
			}

			posX = PacMan.position.x + PacMan.vx;
			if (Field1[(PacMan.position.y - (2)) / 8][(posX + (3 * PacMan.vx))
					/ 8] != '.'
					&& Field1[(PacMan.position.y - (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != 'C'
					&& Field1[(PacMan.position.y - (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != 'M'
					&& Field1[(PacMan.position.y + (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != ':'
					&& Field1[(PacMan.position.y - (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != 'K'
					&& Field1[(PacMan.position.y - (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != '$'
					&& Field1[(PacMan.position.y - (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != 'J'
					&& Field1[(PacMan.position.y - (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != 'r'
					&& Field1[(PacMan.position.y - (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != 'l'
					&& Field1[(PacMan.position.y - (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != '&'
					&& Field1[(PacMan.position.y - (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != '!') {
				PacMan.vx = 0;
			}
			if (Field1[(PacMan.position.y) / 8][(posX + (3 * PacMan.vx)) / 8]
					!= '.'
					&& Field1[(PacMan.position.y) / 8][(posX + (3 * PacMan.vx))
							/ 8] != 'C'
					&& Field1[(PacMan.position.y) / 8][(posX + (3 * PacMan.vx))
							/ 8] != 'M'
					&& Field1[(PacMan.position.y) / 8][(posX + (3 * PacMan.vx))
							/ 8] != ':'
					&& Field1[(PacMan.position.y) / 8][(posX + (3 * PacMan.vx))
							/ 8] != 'K'
					&& Field1[(PacMan.position.y) / 8][(posX + (3 * PacMan.vx))
							/ 8] != '$'
					&& Field1[(PacMan.position.y) / 8][(posX + (3 * PacMan.vx))
							/ 8] != 'J'
					&& Field1[(PacMan.position.y) / 8][(posX + (3 * PacMan.vx))
							/ 8] != 'r'
					&& Field1[(PacMan.position.y) / 8][(posX + (3 * PacMan.vx))
							/ 8] != 'l'
					&& Field1[(PacMan.position.y) / 8][(posX + (3 * PacMan.vx))
							/ 8] != '&'
					&& Field1[(PacMan.position.y) / 8][(posX + (3 * PacMan.vx))
							/ 8] != '!') {
				PacMan.vx = 0;
			}
			if (Field1[(PacMan.position.y + (2)) / 8][(posX + (3 * PacMan.vx))
					/ 8] != '.'
					&& Field1[(PacMan.position.y + (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != 'C'
					&& Field1[(PacMan.position.y + (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != 'M'
					&& Field1[(PacMan.position.y + (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != ':'
					&& Field1[(PacMan.position.y + (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != 'K'
					&& Field1[(PacMan.position.y + (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != '$'
					&& Field1[(PacMan.position.y + (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != 'J'
					&& Field1[(PacMan.position.y + (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != 'r'
					&& Field1[(PacMan.position.y + (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != 'l'
					&& Field1[(PacMan.position.y + (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != '&'
					&& Field1[(PacMan.position.y + (2)) / 8][(posX
							+ (3 * PacMan.vx)) / 8] != '!') {
				PacMan.vx = 0;
			}



			// Deleted selected food and reset the score




							 // right and left tunnel and delete the last pacman from old position
							 if(Field1[PacMan.position.y /8][PacMan.position.x/8] =='r'){
								 gdispFillArea(PacMan.position.x-5 , PacMan.position.y-5 , 12 , 12 , Black);
								 	 rTol = true;

								}else if(Field1[PacMan.position.y /8][PacMan.position.x/8] =='l' ){
									gdispFillArea(PacMan.position.x-5 , PacMan.position.y-5 , 12 , 12 , Black);
									lTor = true;

								}
								if(PacMan.lives == 0){
														PacMan.vx =0;
														PacMan.vy =0;
													}

								if(rTol == true){
									gdispFillArea(PacMan.position.x-5 , PacMan.position.y-5 , 12 , 12 , Black);
									PacMan.position.x = 210;
								}
								if(lTor == true){
									gdispFillArea(PacMan.position.x-5 , PacMan.position.y-5 , 12 , 12 , Black);
									PacMan.position.x = 12;
								}
								if(PacMan.position.x == 210){
									rTol = false;
								}
								if(PacMan.position.x == 12){
									lTor = false;
								}

							// Draw moved Player
						 	 // Animation for do not move

							//clear previous pacman from buffer

								//draw a circle on pacman that is blue
							if(PacMan.chasing == true){
								gdispFillArea(PacMan.position.x-5 , PacMan.position.y-5 , 12 , 12 , Black);

								PacMan.position.x = 112;
								PacMan.position.y = 180;

							}
							if(PacMan.position.x == 112 && PacMan.position.y ==180){
								PacMan.chasing = false;
							}

							gdispFillArea(PacMan.position.x-6 , PacMan.position.y-6 , 13 , 13 , Black);
						 	 if(posY == PacMan.position.y && posX == PacMan.position.x){
							 	 PacMan.position.x += PacMan.vx;
							 	 PacMan.position.y += PacMan.vy;
						 	gdispFillCircle(PacMan.position.x , PacMan.position.y , 5 , PacMan.color);
						 	 }
						 	 // Animation to upper move
						 	 if(posY < PacMan.position.y){
							 	 PacMan.position.x += PacMan.vx;
							 	 PacMan.position.y += PacMan.vy;
						 		if((int)PacMan.animation %4 ==0 ){
						 			gdispFillArc(PacMan.position.x, PacMan.position.y, 5 , 90+45, 45 , PacMan.color);
						 		}else if((int)PacMan.animation % 4 == 1){
						 			gdispFillArc(PacMan.position.x, PacMan.position.y, 5 , 90+20, 70 , PacMan.color);
						 		}else if((int)PacMan.animation % 4 == 2){
						 			gdispFillArc(PacMan.position.x, PacMan.position.y, 5 , 90, 89 , PacMan.color);
						 		}else if((int)PacMan.animation % 4 == 3){
						 			gdispFillArc(PacMan.position.x, PacMan.position.y, 5 , 90+20, 70 , PacMan.color);
						 		}
						 	 }
						 	 // Animation to down move
						 	 if(posY > PacMan.position.y){
							 	 PacMan.position.x += PacMan.vx;
							 	 PacMan.position.y += PacMan.vy;
						 		if((int)PacMan.animation %4 ==0 ){
						 			gdispFillArc(PacMan.position.x, PacMan.position.y, 5 , 270+45, 270-45 , PacMan.color);
						 		}
						 			else if((int)PacMan.animation % 4 == 1){
						 			gdispFillArc(PacMan.position.x, PacMan.position.y, 5 , 270+20, 270-20 , PacMan.color);
						 		}else if((int)PacMan.animation % 4 == 2){
						 			gdispFillArc(PacMan.position.x, PacMan.position.y, 5 , 271, 270 , PacMan.color);
						 		}else if((int)PacMan.animation % 4 == 3){
						 			gdispFillArc(PacMan.position.x, PacMan.position.y, 5 , 270+20, 270-20 , PacMan.color);
						 		}
						 	 }
						 	 // Animation to right move
						 	 if(posX > PacMan.position.x){
							 	 PacMan.position.x += PacMan.vx;
							 	 PacMan.position.y += PacMan.vy;
						 		if((int)PacMan.animation %4 ==0 ){
						 			gdispFillArc(PacMan.position.x, PacMan.position.y, 5 , 45, 325 , PacMan.color);
						 		}else if((int)PacMan.animation % 4 == 1){
						 			gdispFillArc(PacMan.position.x, PacMan.position.y, 5 , 20 , 345 , PacMan.color);
						 		}else if((int)PacMan.animation % 4 == 2){
								 	gdispFillCircle(PacMan.position.x , PacMan.position.y , 5 , PacMan.color);
						 		}else if((int)PacMan.animation % 4 == 3){
						 			gdispFillArc(PacMan.position.x, PacMan.position.y, 5 , 20 , 345,  PacMan.color);
						 		}
						 	 }
						 	 // Animation to left move
						 	 if(posX < PacMan.position.x){
							 	 PacMan.position.x += PacMan.vx;
							 	 PacMan.position.y += PacMan.vy;
						 		if((int)PacMan.animation %4 ==0 ){
						 			gdispFillArc(PacMan.position.x, PacMan.position.y, 5 , 180+45, 180-45 , PacMan.color);
						 		}else if((int)PacMan.animation % 4 == 1){
						 			gdispFillArc(PacMan.position.x, PacMan.position.y, 5 , 180+20, 180-20 , PacMan.color);
						 		}else if((int)PacMan.animation % 4 == 2){
						 			gdispFillCircle(PacMan.position.x , PacMan.position.y , 5 , PacMan.color);
						 		}else if((int)PacMan.animation % 4 == 3){
						 			gdispFillArc(PacMan.position.x, PacMan.position.y, 5 , 180+20, 180-20 , PacMan.color);
						 		}
						 	 }
						 	 	 // mouth speed
						 		PacMan.animation += 0.6;

			if (Field1[PacMan.position.y / 8][PacMan.position.x / 8] == '.'|| Field1[PacMan.position.y / 8][PacMan.position.x / 8] == 'C') {
				if (PacMan.lives > 0) {
					Field1[PacMan.position.y / 8][PacMan.position.x  / 8] = 'N';
					PacMan.food_collected += power;

				}
			}

			// control the fruits

			int i, j;
			int Field_X = 8, Field_Y = 8;

			for (i = 0; i < Field_Height; i++) {
				for (j = 0; j < Field_Width; j++) {

					if ((PacMan.food_collected >= (50 * power)
							&& PacMan.food_collected <= (100 * power))
							|| (PacMan.food_collected >= (150 * power)
									&& PacMan.food_collected <= (190 * power))) {
						switch (Field1[i][j]) {
						case '&':
							gdispDrawArc(Field_X, Field_Y, 4, 90, 180, Red);
							break;

						}

						if (Field1[posY / 8][posX / 8] == '&') {
							Field1[posY / 8][posX / 8] = '!';
							PacMan.food_collected += power * 20;
						}

					}

					if (PacMan.food_collected == (110 * power)) {
						switch (Field1[i][j]) {
						case '!':
							Field1[i][j] = '&';
							break;

						}
					}

					if (PacMan.lives == 0 || restField == true) {
						switch (Field1[i][j]) {
						case 'M':
							Field1[i][j] = '.';
							break;
						case 'K':
							Field1[i][j] = '.';
							break;
						case 'J':
						Field1[i][j] = ':';
						break;
						}

					}
					// controll the bonus live every 3000 score
					if (PacMan.food_collected % 3000 == 0
							&& PacMan.food_collected != 0) {
						PacMan.lives++;
					}

					Field_X += 8;
				}
				Field_X = 8;
				Field_Y += 8;
			}







				   	       // Draw and calculate the score and the level and live and pause
						gdispFillArea(255 , 115 , 60, 30, Black);
				   		sprintf(str, "Score: %d", PacMan.food_collected );
				   		gdispDrawString(250, 120 , str, font1, White);
						// set the high score

						if (PacMan.food_collected > highScore1) {
							highScore1 = PacMan.food_collected;

						}
						gdispFillArea(255 , 85, 30, 30, Black);
						sprintf(str, "Live: %d", PacMan.lives);
						gdispDrawString(250, 90, str, font1, White);
						gdispFillArea(255 , 55, 30, 30, Black);
						sprintf(str, "Level: %d", level);
						gdispDrawString(250, 60, str, font1, White);
						sprintf(str, "A: Pause");
						gdispDrawString(245, 40, str, font1, Green);

	 		// Draw the Red very aggressive Ghost
			//Move the Ghosts in Ready position

			if(escapeMode == false){
			if (GhostRed.startMove == false) {

					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== ' ') {
						Field1[GhostRed.position.y / 8][GhostRed.position.x / 8] =
								'k';
					}
					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== 'M') {
						Field1[GhostRed.position.y / 8][GhostRed.position.x / 8] =
								'K';
					}

				GhostRed.vx = 1;
				GhostX = GhostRed.position.x + GhostRed.vx;
				GhostY = GhostRed.position.y + GhostRed.vy;
				if(PacMan.lives == 0){
										GhostRed.vx=0;
										GhostRed.vy=0;
									}

				if (GhostX > GhostRed.position.x
						&& Field1[(GhostY) / 8][(GhostX) / 8] == 'V') {
					switcher = true;
				}
				if (switcher == true) {
					GhostRed.vx = -1;
					GhostRed.position.x += GhostRed.vx;
					gdispFillCircle(GhostRed.position.x, GhostRed.position.y, 2,
							GhostRed.color);
					GhostX = GhostRed.position.x + GhostRed.vx;
					GhostY = GhostRed.position.y + GhostRed.vy;
					if (GhostX < GhostRed.position.x
							&& Field1[(GhostY) / 8][(GhostX) / 8] == 'V') {
						switcher = false;
					}
				}

				if (switcher == false) {
					GhostRed.vx = 1;
					GhostRed.position.x += GhostRed.vx;
					gdispFillCircle(GhostRed.position.x, GhostRed.position.y, 2,
							GhostRed.color);
				}

				// Delete and open the door when move the PacMan
					for (int i = 0; i < Field_Height; i++) {
						for (int j = 0; j < Field_Width; j++) {
							if (buttonStatus.joystick.x >= 135
									|| buttonStatus.joystick.y >= 130
									|| buttonStatus.joystick.x <= 120
									|| buttonStatus.joystick.y <= 110) {

								DoorOpen = true;
								GhostRed.startMove = true;
							}
							if (Field1[i][j] == 'D' && DoorOpen == true) {
								Field1[i][j] = 'S';

							}

						}
					}
				}

				// Draw the Red very aggressive Ghost (Blinky)
				if (GhostRed.startMove == true) {

					// move the ghost to out the box and set the manual move
					// set start point x

					if (GhostSwitcher == false) {

						if (GhostRed.position.x > 108) {
							if (PacMan.lives == 0) {
								GhostRed.vx = 0;
								GhostRed.vy = 0;
							} else {
								if (Field1[GhostRed.position.y / 8][GhostRed.position.x
										/ 8] == ' ') {
									Field1[GhostRed.position.y / 8][GhostRed.position.x
											/ 8] = 'k';
								}
								if (Field1[GhostRed.position.y / 8][GhostRed.position.x
										/ 8] == 'M') {
									Field1[GhostRed.position.y / 8][GhostRed.position.x
											/ 8] = 'K';
								}
								if(Field1[GhostRed.position.y / 8][GhostRed.position.x
												/ 8] == 'S'){
									gdispFillArea(GhostRed.position.x -6 ,GhostRed.position.y - 6, 12 , 12, Black);
								}
								GhostRed.vx = -1;
								GhostRed.position.x += GhostRed.vx;
							}
							gdispFillCircle(GhostRed.position.x,
									GhostRed.position.y, 2, GhostRed.color);
						} else if (GhostRed.position.x <= 108) {
							if (PacMan.lives == 0) {
								GhostRed.vx = 0;
								GhostRed.vy = 0;
							} else {
								if (Field1[GhostRed.position.y / 8][GhostRed.position.x
										/ 8] == ' ') {
									Field1[GhostRed.position.y / 8][GhostRed.position.x
											/ 8] = 'k';
								}
								if (Field1[GhostRed.position.y / 8][GhostRed.position.x
										/ 8] == 'M') {
									Field1[GhostRed.position.y / 8][GhostRed.position.x
											/ 8] = 'K';
								}
								if (Field1[GhostRed.position.y / 8][GhostRed.position.x
										/ 8] == 'S') {
									gdispFillArea(GhostRed.position.x - 6,
											GhostRed.position.y - 6, 12, 12,
											Black);
								}
								if (Field1[GhostRed.position.y / 8][GhostRed.position.x
										/ 8] == '.') {
									Field1[GhostRed.position.y / 8][GhostRed.position.x
											/ 8] = 'C';
								}
								GhostRed.vx = 1;
								GhostRed.position.x += GhostRed.vx;
							}
						}
						gdispFillCircle(GhostRed.position.x,
								GhostRed.position.y, 2, GhostRed.color);
						//set start point y
						if (GhostRed.position.y >= 83) {
							if (PacMan.lives == 0) {
								GhostRed.vx = 0;
								GhostRed.vy = 0;
							} else {
								if (Field1[GhostRed.position.y / 8][GhostRed.position.x
										/ 8] == ' ') {
									Field1[GhostRed.position.y / 8][GhostRed.position.x
											/ 8] = 'k';
								}
								if (Field1[GhostRed.position.y / 8][GhostRed.position.x
										/ 8] == 'M') {
									Field1[GhostRed.position.y / 8][GhostRed.position.x
											/ 8] = 'K';
								}
								if (Field1[GhostRed.position.y / 8][GhostRed.position.x
										/ 8] == 'S') {
									gdispFillArea(GhostRed.position.x - 6,
											GhostRed.position.y - 6, 12, 12,
											Black);
								}
								if (Field1[GhostRed.position.y / 8][GhostRed.position.x
										/ 8] == '.') {
									Field1[GhostRed.position.y / 8][GhostRed.position.x
											/ 8] = 'C';
								}
								GhostRed.vy = -1;
								GhostRed.position.y += GhostRed.vy;
							}
							gdispFillCircle(GhostRed.position.x,
									GhostRed.position.y, 2, GhostRed.color);
						}
						if (GhostRed.position.y == 82
								&& (GhostRed.position.x == 109
										|| GhostRed.position.x == 108))
							GhostSwitcher = true;

					}

					// Find the random way in the field for the ghost and true out the box

					if (GhostSwitcher == true) {
						//Generate directoins
						// 1 bottom right
						if (PacMan.position.x > GhostRed.position.x
								&& PacMan.position.y > GhostRed.position.y
								&& GhostRed.GOSX != -1 && GhostRed.GOSY != -1) {
							GhostRed.vx = GhostRed.Speed;
							GhostRed.vy = GhostRed.Speed;

						} else if (PacMan.position.x < GhostRed.position.x
								&& PacMan.position.y < GhostRed.position.y
								&& GhostRed.GOSX != 1 && GhostRed.GOSY != 1) { // 2 Top left
							GhostRed.vx = -GhostRed.Speed;
							GhostRed.vy = -GhostRed.Speed;
						} else if (PacMan.position.x > GhostRed.position.x
								&& PacMan.position.y < GhostRed.position.y
								&& GhostRed.GOSX != -1 && GhostRed.GOSY != 1) { // 3 Top right
							GhostRed.vx = GhostRed.Speed;
							GhostRed.vy = -GhostRed.Speed;
						} else if (PacMan.position.x < GhostRed.position.x
								&& PacMan.position.y > GhostRed.position.y
								&& GhostRed.GOSX != 1 && GhostRed.GOSY != -1) { // 4 bottom left
							GhostRed.vx = -GhostRed.Speed;
							GhostRed.vy = GhostRed.Speed;
						}

						savedGX = GhostRed.vx;
						savedGY = GhostRed.vy;

						GhostY = GhostRed.position.y + GhostRed.vy;
						if (Field1[(GhostY + (3 * GhostRed.vy)) / 8][GhostRed.position.x
								/ 8] == 'V'
								|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x)
										/ 8] == '#'
								|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x)
										/ 8] == 'D'
								|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x)
										/ 8] == 'b'
								|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x)
										/ 8] == 'B'
								|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x)
										/ 8] == 'R'
								|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x)
										/ 8] == 'L'
								|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x)
										/ 8] == 'S') {
							GhostRed.vy = 0;
						}
						GhostX = GhostRed.position.x + GhostRed.vx;
						if (Field1[(GhostRed.position.y) / 8][(GhostX
								+ (3 * GhostRed.vx)) / 8] == 'V'
								|| Field1[(GhostRed.position.y) / 8][(GhostX
										+ (3 * GhostRed.vx)) / 8] == '#'
								|| Field1[(GhostRed.position.y) / 8][(GhostX
										+ (3 * GhostRed.vx)) / 8] == 'D'
								|| Field1[(GhostRed.position.y) / 8][(GhostX
										+ (3 * GhostRed.vx)) / 8] == 'b'
								|| Field1[(GhostRed.position.y) / 8][(GhostX
										+ (3 * GhostRed.vx)) / 8] == 'B'
								|| Field1[(GhostRed.position.y) / 8][(GhostX
										+ (3 * GhostRed.vx)) / 8] == 'R'
								|| Field1[(GhostRed.position.y) / 8][(GhostX
										+ (3 * GhostRed.vx)) / 8] == 'L'
								|| Field1[(GhostRed.position.y) / 8][(GhostX
										+ (3 * GhostRed.vx)) / 8] == 'S') {
							GhostRed.vx = 0;
						}

						// Random to the debug in corners in the same side like PacMan and debug the same line positions
						if (GhostRed.vx == 0 && GhostRed.vy == 0) {

							if (PacMan.position.x > GhostRed.position.x
									&& PacMan.position.y
											> GhostRed.position.y) {
								GhostRed.vx = -GhostRed.Speed;
								GhostRed.vy = GhostRed.Speed;

							} else if (PacMan.position.x < GhostRed.position.x
									&& PacMan.position.y
											< GhostRed.position.y) { // 2 Top left
								GhostRed.vx = GhostRed.Speed;
								GhostRed.vy = -GhostRed.Speed;
							} else if (PacMan.position.x > GhostRed.position.x
									&& PacMan.position.y
											< GhostRed.position.y) { // 3 Top right
								GhostRed.vx = -GhostRed.Speed;
								GhostRed.vy = -GhostRed.Speed;
							} else if (PacMan.position.x < GhostRed.position.x
									&& PacMan.position.y
											> GhostRed.position.y) { // 4 bottom left
								GhostRed.vx = GhostRed.Speed;
								GhostRed.vy = GhostRed.Speed;
							}
							//			 generate random positions to debug the same line positions
							if ((PacMan.position.x == GhostRed.position.x
									&& GhostRed.vx == 0 && GhostRed.vy == 0)
									|| (PacMan.position.y == GhostRed.position.y
											&& GhostRed.vy == 0
											&& GhostRed.vx == 0)) {
								GhostcountX = rand() % (4);
								if ((int) GhostcountX == 0
										&& (GhostRed.GOSX != -1
												|| GhostRed.GOSY != -1)) {
									GhostRed.vx = GhostRed.Speed;
									GhostRed.vy = GhostRed.Speed;

								} else if ((int) GhostcountX == 1
										&& (GhostRed.GOSX != 1
												|| GhostRed.GOSY != 1)) {
									GhostRed.vx = -GhostRed.Speed;
									GhostRed.vy = -GhostRed.Speed;
								} else if ((int) GhostcountX == 2
										&& (GhostRed.GOSX != -1
												|| GhostRed.GOSY != 1)) {
									GhostRed.vx = GhostRed.Speed;
									GhostRed.vy = -GhostRed.Speed;

								} else if ((int) GhostcountX == 3
										&& (GhostRed.GOSX != 1
												|| GhostRed.GOSY != -1)) {
									GhostRed.vx = -GhostRed.Speed;
									GhostRed.vy = GhostRed.Speed;

								}
							}
							GhostY = GhostRed.position.y + GhostRed.vy;
							if (Field1[(GhostY + (3 * GhostRed.vy)) / 8][GhostRed.position.x / 8] == 'V'
									|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
											) / 8] == '#'
									|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
											) / 8] == 'D'
									|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
											) / 8] == 'b'
									|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
											) / 8] == 'B'
									|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
											) / 8] == 'R'
									|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
											) / 8] == 'L'
									|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
											) / 8] == 'S') {
								GhostRed.vy = 0;
							}
							GhostX = GhostRed.position.x + GhostRed.vx;
							if (Field1[(GhostRed.position.y )
									/ 8][(GhostX + (3 * GhostRed.vx)) / 8]
									== 'V'
									|| Field1[(GhostRed.position.y
											) / 8][(GhostX
											+ (3 * GhostRed.vx)) / 8] == '#'
									|| Field1[(GhostRed.position.y
											) / 8][(GhostX
											+ (3 * GhostRed.vx)) / 8] == 'D'
									|| Field1[(GhostRed.position.y
											) / 8][(GhostX
											+ (3 * GhostRed.vx)) / 8] == 'b'
									|| Field1[(GhostRed.position.y
											) / 8][(GhostX
											+ (3 * GhostRed.vx)) / 8] == 'B'
									|| Field1[(GhostRed.position.y
											) / 8][(GhostX
											+ (3 * GhostRed.vx)) / 8] == 'R'
									|| Field1[(GhostRed.position.y
											) / 8][(GhostX
											+ (3 * GhostRed.vx)) / 8] == 'L'
									|| Field1[(GhostRed.position.y
											) / 8][(GhostX
											+ (3 * GhostRed.vx)) / 8] == 'S') {
								GhostRed.vx = 0;
							}
							GhostRed.GOSX = GhostRed.vx;
							GhostRed.GOSY = GhostRed.vy;

						}
						GhostRed.GOSX = GhostRed.vx;
						GhostRed.GOSY = GhostRed.vy;
						// tunnel right left for the ghost
						if (Field1[GhostRed.position.y / 8][GhostRed.position.x
								/ 8] == 'r') {
							gdispFillArea(GhostRed.position.x-10 ,GhostRed.position.y - 10, 10 , 10, Black);
							GhostRed.position.x = 210;

						} else if (Field1[GhostRed.position.y / 8][GhostRed.position.x
								/ 8] == 'l') {
							gdispFillArea(GhostRed.position.x-10 ,GhostRed.position.y - 10, 10 , 10, Black);
							GhostRed.position.x = 12;
						}

						if (PacMan.lives == 0) {
							GhostRed.vx = 0;
							GhostRed.vy = 0;
						}


						if (Field1[GhostRed.position.y / 8][GhostRed.position.x
								/ 8] == ' ') {
							Field1[GhostRed.position.y / 8][GhostRed.position.x
									/ 8] = 'k';
						}
						if (Field1[GhostRed.position.y / 8][GhostRed.position.x
								/ 8] == 'M') {
							Field1[GhostRed.position.y / 8][GhostRed.position.x
									/ 8] = 'K';
						}						if (Field1[GhostRed.position.y / 8][GhostRed.position.x
								/ 8] == '.') {
							Field1[GhostRed.position.y / 8][GhostRed.position.x/ 8] ='C';
						}

						// Draw the Ghost in the filtered position
						GhostRed.position.x += GhostRed.vx;
						GhostRed.position.y += GhostRed.vy;
						gdispFillCircle(GhostRed.position.x,
								GhostRed.position.y, 2, GhostRed.color);
					}



					if (Field1[posY / 8][posX / 8] == ':') {
						Field1[posY / 8][posX / 8] = 'J';
						escapeMode = true;
					}

				}
				if ((GhostRed.position.x + (4 * GhostRed.vx)
						== PacMan.position.x + (4 * PacMan.vx)
						&& GhostRed.position.y + (4 * GhostRed.vy)
								== PacMan.position.y + (4 * PacMan.vy))
						|| (GhostRed.position.x + (8 * GhostRed.vx)
								== PacMan.position.x + (PacMan.vx)
								&& GhostRed.position.y + (8 * GhostRed.vy)
										== PacMan.position.y + (PacMan.vy))
						|| (GhostRed.position.x + (1 * GhostRed.vx)
								== PacMan.position.x + (1 * PacMan.vx)
								&& GhostRed.position.y + (1 * GhostRed.vy)
										== PacMan.position.y
												+ (1 * PacMan.vy))) {
					gdispFillArea(PacMan.position.x-6 , PacMan.position.y-6 , 13 , 13 , Black);
					if (PacMan.lives > 0) {
						StartMOVE = false;
						GhostRed.startMove = false;
						GhostSwitcher = false;
						GhostRed.position.x = 110;
						GhostRed.position.y = 115;
						DoorOpen = false;
						PacMan.lives--;
						PacMan.chasing = true;
					}

				}


			}else if (escapeMode == true) { // make the Ghosts escape mode. move away from Pac man
				if(Gchasing == false){

									// Find the random way in the field for the ghost and true out the box

										//Generate directoins
										// 1 bottom right
										if (PacMan.position.x > GhostRed.position.x
												&& PacMan.position.y > GhostRed.position.y
												&& GhostRed.GOSX != -1 && GhostRed.GOSY != -1) {
											GhostRed.vx = GhostRed.Speed;
											GhostRed.vy = GhostRed.Speed;

										} else if (PacMan.position.x < GhostRed.position.x
												&& PacMan.position.y < GhostRed.position.y
												&& GhostRed.GOSX != 1 && GhostRed.GOSY != 1) { // 2 Top left
											GhostRed.vx = -GhostRed.Speed;
											GhostRed.vy = -GhostRed.Speed;
										} else if (PacMan.position.x > GhostRed.position.x
												&& PacMan.position.y < GhostRed.position.y
												&& GhostRed.GOSX != -1 && GhostRed.GOSY != 1) { // 3 Top right
											GhostRed.vx = GhostRed.Speed;
											GhostRed.vy = -GhostRed.Speed;
										} else if (PacMan.position.x < GhostRed.position.x
												&& PacMan.position.y > GhostRed.position.y
												&& GhostRed.GOSX != 1 && GhostRed.GOSY != -1) { // 4 bottom left
											GhostRed.vx = -GhostRed.Speed;
											GhostRed.vy = GhostRed.Speed;
										}

										savedGX = GhostRed.vx;
										savedGY = GhostRed.vy;

										GhostY = GhostRed.position.y + GhostRed.vy;
										if (Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
												+ (3 * GhostRed.vx)) / 8] == 'V'
												|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
														+ (3 * GhostRed.vx)) / 8] == '#'
												|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
														+ (3 * GhostRed.vx)) / 8] == 'D'
												|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
														+ (3 * GhostRed.vx)) / 8] == 'b'
												|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
														+ (3 * GhostRed.vx)) / 8] == 'B'
												|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
														+ (3 * GhostRed.vx)) / 8] == 'R'
												|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
														+ (3 * GhostRed.vx)) / 8] == 'L'
												|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
														+ (3 * GhostRed.vx)) / 8] == 'S') {
											GhostRed.vy = 0;
										}
										GhostX = GhostRed.position.x + GhostRed.vx;
										if (Field1[(GhostRed.position.y + (3 * GhostRed.vy)) / 8][(GhostX
												+ (3 * GhostRed.vx)) / 8] == 'V'
												|| Field1[(GhostRed.position.y
														+ (3 * GhostRed.vy)) / 8][(GhostX
														+ (3 * GhostRed.vx)) / 8] == '#'
												|| Field1[(GhostRed.position.y
														+ (3 * GhostRed.vy)) / 8][(GhostX
														+ (3 * GhostRed.vx)) / 8] == 'D'
												|| Field1[(GhostRed.position.y
														+ (3 * GhostRed.vy)) / 8][(GhostX
														+ (3 * GhostRed.vx)) / 8] == 'b'
												|| Field1[(GhostRed.position.y
														+ (3 * GhostRed.vy)) / 8][(GhostX
														+ (3 * GhostRed.vx)) / 8] == 'B'
												|| Field1[(GhostRed.position.y
														+ (3 * GhostRed.vy)) / 8][(GhostX
														+ (3 * GhostRed.vx)) / 8] == 'R'
												|| Field1[(GhostRed.position.y
														+ (3 * GhostRed.vy)) / 8][(GhostX
														+ (3 * GhostRed.vx)) / 8] == 'L'
												|| Field1[(GhostRed.position.y
														+ (3 * GhostRed.vy)) / 8][(GhostX
														+ (3 * GhostRed.vx)) / 8] == 'S') {
											GhostRed.vx = 0;
										}

										// Random to the debug in corners in the same side like PacMan and debug the same line positions
										if (GhostRed.vx == 0 && GhostRed.vy == 0) {

											if (PacMan.position.x > GhostRed.position.x
													&& PacMan.position.y
															> GhostRed.position.y) {
												GhostRed.vx = -GhostRed.Speed;
												GhostRed.vy = GhostRed.Speed;

											} else if (PacMan.position.x < GhostRed.position.x
													&& PacMan.position.y
															< GhostRed.position.y) { // 2 Top left
												GhostRed.vx = GhostRed.Speed;
												GhostRed.vy = -GhostRed.Speed;
											} else if (PacMan.position.x > GhostRed.position.x
													&& PacMan.position.y
															< GhostRed.position.y) { // 3 Top right
												GhostRed.vx = -GhostRed.Speed;
												GhostRed.vy = -GhostRed.Speed;
											} else if (PacMan.position.x < GhostRed.position.x
													&& PacMan.position.y
															> GhostRed.position.y) { // 4 bottom left
												GhostRed.vx = GhostRed.Speed;
												GhostRed.vy = GhostRed.Speed;
											}
											//			 generate random positions to debug the same line positions
											if ((PacMan.position.x == GhostRed.position.x
													&& GhostRed.vx == 0 && GhostRed.vy == 0)
													|| (PacMan.position.y == GhostRed.position.y
															&& GhostRed.vy == 0
															&& GhostRed.vx == 0)) {
												GhostcountX = rand() % (4);
												if ((int) GhostcountX == 0
														&& (GhostRed.GOSX != -1
																|| GhostRed.GOSY != -1)) {
													GhostRed.vx = GhostRed.Speed;
													GhostRed.vy = GhostRed.Speed;

												} else if ((int) GhostcountX == 1
														&& (GhostRed.GOSX != 1
																|| GhostRed.GOSY != 1)) {
													GhostRed.vx = -GhostRed.Speed;
													GhostRed.vy = -GhostRed.Speed;
												} else if ((int) GhostcountX == 2
														&& (GhostRed.GOSX != -1
																|| GhostRed.GOSY != 1)) {
													GhostRed.vx = GhostRed.Speed;
													GhostRed.vy = -GhostRed.Speed;

												} else if ((int) GhostcountX == 3
														&& (GhostRed.GOSX != 1
																|| GhostRed.GOSY != -1)) {
													GhostRed.vx = -GhostRed.Speed;
													GhostRed.vy = GhostRed.Speed;

												}
											}
											GhostY = GhostRed.position.y + GhostRed.vy;
											if (Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
													+ (3 * GhostRed.vx)) / 8] == 'V'
													|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
															+ (3 * GhostRed.vx)) / 8] == '#'
													|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
															+ (3 * GhostRed.vx)) / 8] == 'D'
													|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
															+ (3 * GhostRed.vx)) / 8] == 'b'
													|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
															+ (3 * GhostRed.vx)) / 8] == 'B'
													|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
															+ (3 * GhostRed.vx)) / 8] == 'R'
													|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
															+ (3 * GhostRed.vx)) / 8] == 'L'
													|| Field1[(GhostY + (3 * GhostRed.vy)) / 8][(GhostRed.position.x
															+ (3 * GhostRed.vx)) / 8] == 'S') {
												GhostRed.vy = 0;
											}
											GhostX = GhostRed.position.x + GhostRed.vx;
											if (Field1[(GhostRed.position.y + (3 * GhostRed.vy))
													/ 8][(GhostX + (3 * GhostRed.vx)) / 8]
													== 'V'
													|| Field1[(GhostRed.position.y
															+ (3 * GhostRed.vy)) / 8][(GhostX
															+ (3 * GhostRed.vx)) / 8] == '#'
													|| Field1[(GhostRed.position.y
															+ (3 * GhostRed.vy)) / 8][(GhostX
															+ (3 * GhostRed.vx)) / 8] == 'D'
													|| Field1[(GhostRed.position.y
															+ (3 * GhostRed.vy)) / 8][(GhostX
															+ (3 * GhostRed.vx)) / 8] == 'b'
													|| Field1[(GhostRed.position.y
															+ (3 * GhostRed.vy)) / 8][(GhostX
															+ (3 * GhostRed.vx)) / 8] == 'B'
													|| Field1[(GhostRed.position.y
															+ (3 * GhostRed.vy)) / 8][(GhostX
															+ (3 * GhostRed.vx)) / 8] == 'R'
													|| Field1[(GhostRed.position.y
															+ (3 * GhostRed.vy)) / 8][(GhostX
															+ (3 * GhostRed.vx)) / 8] == 'L'
													|| Field1[(GhostRed.position.y
															+ (3 * GhostRed.vy)) / 8][(GhostX
															+ (3 * GhostRed.vx)) / 8] == 'S') {
												GhostRed.vx = 0;
											}
											GhostRed.GOSX = GhostRed.vx;
											GhostRed.GOSY = GhostRed.vy;

										}
										GhostRed.GOSX = GhostRed.vx;
										GhostRed.GOSY = GhostRed.vy;
										// tunnel right left for the ghost
										if (Field1[GhostRed.position.y / 8][GhostRed.position.x
												/ 8] == 'r') {
											GhostRed.position.x = 210;

										} else if (Field1[GhostRed.position.y / 8][GhostRed.position.x
												/ 8] == 'l') {
											GhostRed.position.x = 12;
										}

										if (PacMan.lives == 0) {
											GhostRed.vx = 0;
											GhostRed.vy = 0;
										}

										// Draw the Ghost in the filtered position
					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== ' ') {
						Field1[GhostRed.position.y / 8][GhostRed.position.x / 8] =
								'k';
					}
					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== 'M') {
						Field1[GhostRed.position.y / 8][GhostRed.position.x / 8] =
								'K';
					}
					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== '.') {
						Field1[GhostRed.position.y / 8][GhostRed.position.x / 8] =
								'C';
					}

					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== '!') {
						gdispFillArea(Field_X - 10, Field_Y - 10, 12, 12,
								Black);
					}
					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== 'J') {
						gdispFillArea(Field_X - 10, Field_Y - 10, 12, 12,
								Black);
					}
										GhostRed.position.x += GhostRed.vx;
										GhostRed.position.y += GhostRed.vy;
										gdispFillCircle(GhostRed.position.x,
												GhostRed.position.y, 2, Green);


									if ((GhostRed.position.x + (4 * GhostRed.vx)
											== PacMan.position.x + (4 * PacMan.vx)
											&& GhostRed.position.y + (4 * GhostRed.vy)
													== PacMan.position.y + (4 * PacMan.vy))
											|| (GhostRed.position.x + (8 * GhostRed.vx)
													== PacMan.position.x + (PacMan.vx)
													&& GhostRed.position.y + (8 * GhostRed.vy)
															== PacMan.position.y + (PacMan.vy))
											|| (GhostRed.position.x + (1 * GhostRed.vx)
													== PacMan.position.x + (1 * PacMan.vx)
													&& GhostRed.position.y + (1 * GhostRed.vy)
															== PacMan.position.y
																	+ (1 * PacMan.vy))) {
										PacMan.food_collected += 200;
										Gchasing = true;
									}

								}

				else if (Gchasing == true && Garrived == false) {
					//Generate directoins
					// 1 bottom right
					int GstartX = 110;
					int GstartY = 84;

					if (GstartX  > GhostRed.position.x
							&& GstartY > GhostRed.position.y
							&& GhostRed.GOSX != -1 && GhostRed.GOSY != -1) {
						GhostRed.vx = GhostRed.Speed;
						GhostRed.vy = GhostRed.Speed;


					} else if (GstartX < GhostRed.position.x
							&& GstartY < GhostRed.position.y
							&& GhostRed.GOSX != 1 && GhostRed.GOSY != 1) { // 2 Top left
						GhostRed.vx = -GhostRed.Speed;
						GhostRed.vy = -GhostRed.Speed;

					} else if (GstartX > GhostRed.position.x
							&& GstartY < GhostRed.position.y
							&& GhostRed.GOSX != -1 && GhostRed.GOSY != 1) { // 3 Top right
						GhostRed.vx = GhostRed.Speed;
						GhostRed.vy = -GhostRed.Speed;
						z =0;
					} else if (GstartX < GhostRed.position.x
							&& GstartY > GhostRed.position.y
							&& GhostRed.GOSX != 1 && GhostRed.GOSY != -1) { // 4 bottom left
						GhostRed.vx = -GhostRed.Speed;
						GhostRed.vy = GhostRed.Speed;

					}else{
						GhostRed.vx =0;
						GhostRed.vy =0;
					}

					savedGX = GhostRed.vx;
					savedGY = GhostRed.vy;

					GhostY = GhostRed.position.y + GhostRed.vy;
					if (Field1[(GhostY + (3 * GhostRed.vy)) / 8][GhostRed.position.x
							/ 8] == 'V'
							|| Field1[(GhostY + (4 * GhostRed.vy)) / 8][GhostRed.position.x
									/ 8] == '#'
							|| Field1[(GhostY + (4 * GhostRed.vy)) / 8][GhostRed.position.x
									/ 8] == 'D'
							|| Field1[(GhostY + (4 * GhostRed.vy)) / 8][GhostRed.position.x
									/ 8] == 'b'
							|| Field1[(GhostY + (4 * GhostRed.vy)) / 8][GhostRed.position.x
									/ 8] == 'B'
							|| Field1[(GhostY + (4 * GhostRed.vy)) / 8][GhostRed.position.x
									/ 8] == 'R'
							|| Field1[(GhostY + (4 * GhostRed.vy)) / 8][GhostRed.position.x
									/ 8] == 'L'
							|| Field1[(GhostY + (4 * GhostRed.vy)) / 8][GhostRed.position.x
									/ 8] == 'S') {
						GhostRed.vy = 0;
					}
					GhostX = GhostRed.position.x + GhostRed.vx;
					if (Field1[GhostRed.position.y / 8][(GhostX
							+ (4 * GhostRed.vx)) / 8] == 'V'
							|| Field1[GhostRed.position.y / 8][(GhostX
									+ (4 * GhostRed.vx)) / 8] == '#'
							|| Field1[GhostRed.position.y / 8][(GhostX
									+ (4 * GhostRed.vx)) / 8] == 'D'
							|| Field1[GhostRed.position.y / 8][(GhostX
									+ (4 * GhostRed.vx)) / 8] == 'b'
							|| Field1[GhostRed.position.y / 8][(GhostX
									+ (4 * GhostRed.vx)) / 8] == 'B'
							|| Field1[GhostRed.position.y / 8][(GhostX
									+ (4 * GhostRed.vx)) / 8] == 'R'
							|| Field1[GhostRed.position.y / 8][(GhostX
									+ (4 * GhostRed.vx)) / 8] == 'L'
							|| Field1[GhostRed.position.y / 8][(GhostX
									+ (4 * GhostRed.vx)) / 8] == 'S') {
						GhostRed.vx = 0;
								}
					// make random movement to debug in position
					if (GhostRed.vx == 0 && GhostRed.vy == 0 && Garrived == false) {

						if (GstartX > GhostRed.position.x
								&& GstartY > GhostRed.position.y) {
							GhostRed.vx = -GhostRed.Speed;
							GhostRed.vy = GhostRed.Speed;

						} else if (GstartX < GhostRed.position.x
								&& GstartY < GhostRed.position.y) { // 2 Top left
							GhostRed.vx = GhostRed.Speed;
							GhostRed.vy = -GhostRed.Speed;
						} else if (GstartX > GhostRed.position.x
								&& GstartY < GhostRed.position.y) { // 3 Top right
							GhostRed.vx = -GhostRed.Speed;
							GhostRed.vy = -GhostRed.Speed;
						} else if (GstartX < GhostRed.position.x
								&& GstartY > GhostRed.position.y) { // 4 bottom left
							GhostRed.vx = GhostRed.Speed;
							GhostRed.vy = GhostRed.Speed;
						}


						// debug the same y move direction
						if (GstartX < GhostRed.position.x
								&& GstartY < GhostRed.position.y
								&& GhostRed.GOSX != 0 && GhostRed.GOSY == 1) {
							GhostRed.vx = -GhostRed.Speed;
							GhostRed.vy = GhostRed.Speed;

						}

						if (GstartX > GhostRed.position.x
								&& GstartY < GhostRed.position.y
								&& GhostRed.GOSX != 0 && GhostRed.GOSY == 1) {
							GhostRed.vx = GhostRed.Speed;
							GhostRed.vy = GhostRed.Speed;

						}


						// control the other conditions
						z++;
						GhostRed.GOSX = GhostRed.vx;
						GhostRed.GOSY = GhostRed.vy;
					}
						//			 generate random positions to debug the same line positions
						if ((GstartX == GhostRed.position.x
								)
								|| (GstartY == GhostRed.position.y
										)) {
							GhostcountX = rand() % (4);
							if ((int) GhostcountX == 0
									&& (GhostRed.GOSX != -1
											|| GhostRed.GOSY != -1)) {
								GhostRed.vx = GhostRed.Speed;
								GhostRed.vy = GhostRed.Speed;

							} else if ((int) GhostcountX == 1
									&& (GhostRed.GOSX != 1 || GhostRed.GOSY != 1)) {
								GhostRed.vx = -GhostRed.Speed;
								GhostRed.vy = -GhostRed.Speed;
							} else if ((int) GhostcountX == 2
									&& (GhostRed.GOSX != -1
											|| GhostRed.GOSY != 1)) {
								GhostRed.vx = GhostRed.Speed;
								GhostRed.vy = -GhostRed.Speed;

							} else if ((int) GhostcountX == 3
									&& (GhostRed.GOSX != 1
											|| GhostRed.GOSY != -1)) {
								GhostRed.vx = -GhostRed.Speed;
								GhostRed.vy = GhostRed.Speed;

							}
						z++;
						GhostRed.GOSX = GhostRed.vx;
						GhostRed.GOSY = GhostRed.vy;
					}
						if (Field1[(GhostY + (3 * GhostRed.vy)) / 8][GhostRed.position.x
								/ 8] == 'V'
								|| Field1[(GhostY + (4 * GhostRed.vy)) / 8][GhostRed.position.x
										/ 8] == '#'
								|| Field1[(GhostY + (4 * GhostRed.vy)) / 8][GhostRed.position.x
										/ 8] == 'D'
								|| Field1[(GhostY + (4 * GhostRed.vy)) / 8][GhostRed.position.x
										/ 8] == 'b'
								|| Field1[(GhostY + (4 * GhostRed.vy)) / 8][GhostRed.position.x
										/ 8] == 'B'
								|| Field1[(GhostY + (4 * GhostRed.vy)) / 8][GhostRed.position.x
										/ 8] == 'R'
								|| Field1[(GhostY + (4 * GhostRed.vy)) / 8][GhostRed.position.x
										/ 8] == 'L'
								|| Field1[(GhostY + (4 * GhostRed.vy)) / 8][GhostRed.position.x
										/ 8] == 'S') {
							GhostRed.vy = 0;
						}
						GhostX = GhostRed.position.x + GhostRed.vx;
						if (Field1[GhostRed.position.y / 8][(GhostX
								+ (4 * GhostRed.vx)) / 8] == 'V'
								|| Field1[GhostRed.position.y / 8][(GhostX
										+ (4 * GhostRed.vx)) / 8] == '#'
								|| Field1[GhostRed.position.y / 8][(GhostX
										+ (4 * GhostRed.vx)) / 8] == 'D'
								|| Field1[GhostRed.position.y / 8][(GhostX
										+ (4 * GhostRed.vx)) / 8] == 'b'
								|| Field1[GhostRed.position.y / 8][(GhostX
										+ (4 * GhostRed.vx)) / 8] == 'B'
								|| Field1[GhostRed.position.y / 8][(GhostX
										+ (4 * GhostRed.vx)) / 8] == 'R'
								|| Field1[GhostRed.position.y / 8][(GhostX
										+ (4 * GhostRed.vx)) / 8] == 'L'
								|| Field1[GhostRed.position.y / 8][(GhostX
										+ (4 * GhostRed.vx)) / 8] == 'S') {
							GhostRed.vx = 0;
									}



						diffGhostX = GhostRed.position.x - GstartX;
					diffGhostY = GhostRed.position.y - GstartY;
					GhostRed.nx = GhostRed.position.x + GhostRed.vx;
					GhostRed.ny = GhostRed.position.y + GhostRed.vy;
					int m = abs(diffGhostX);
					int n = abs(diffGhostY);
					int nextX = GhostRed.nx - GstartX;
					int nextY = GhostRed.ny - GstartY;
					int a = abs(nextX);
					int b = abs(nextY);
					if (z >= 50){

						if (m < a) {
							GhostRed.vx = 0;
						}
						if (n < b) {
							GhostRed.vy = 0;
						}

						GhostRed.GOSX = GhostRed.vx;
						GhostRed.GOSY = GhostRed.vy;
						z = 0;

					}

					if(b < n && a > m && GhostRed.vy !=0){
						GhostRed.vx =0;

					}


					if(GhostRed.vx !=0 && GhostRed.vy !=0){
						if(b > a){
							GhostRed.vx = 0;
						}
						if(a > b){
							GhostRed.vy =0;
						}

					}


					if ((GhostRed.position.x + (1 * GhostRed.vx)
									== GstartX
									&& GhostRed.position.y + (1 * GhostRed.vy)
											== GstartY)) {
						Garrived = true;
					}
					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== ' ') {
						Field1[GhostRed.position.y / 8][GhostRed.position.x / 8] =
								'k';
					}
					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== 'M') {
						Field1[GhostRed.position.y / 8][GhostRed.position.x / 8] =
								'K';
					}
					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== '.') {
						Field1[GhostRed.position.y / 8][GhostRed.position.x / 8] =
								'C';
					}
					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== '!') {
						gdispFillArea(Field_X - 10, Field_Y - 10, 12, 12,
								Black);
					}
					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== 'J') {
						gdispFillArea(Field_X - 10, Field_Y - 10, 12, 12,
								Black);
					}
					GhostRed.position.x += GhostRed.vx;
												GhostRed.position.y += GhostRed.vy;
												gdispFillCircle(GhostRed.position.x,
														GhostRed.position.y, 2, Red);


		}

				if (Garrived == true) {

					GhostRed.vx=1;
					GhostRed.vy=1;
					GhostRed.position.x += GhostRed.vx;
					GhostRed.position.y += GhostRed.vy;
					if (GhostRed.position.x > 120) {
						GhostRed.vx = 0;
					}
					if (GhostRed.position.y > 110) {
						GhostRed.vy = 0;
					}

					if (GhostRed.position.x >= 120
							&& GhostRed.position.y >= 110) {
						escapeMode = false;
						Gchasing = false;
						Garrived = false;
						GhostSwitcher = false;
						GhostRed.vx = 0;
						GhostRed.vy = 0;
					}


					// update only some pixels
					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== ' '){
						Field1[GhostRed.position.y / 8][GhostRed.position.x / 8] =
														'k';
					}
					if ( Field1[GhostRed.position.y / 8][GhostRed.position.x
									/ 8] == 'M') {
						Field1[GhostRed.position.y / 8][GhostRed.position.x / 8] =
								'K';
					}
					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== '.') {
						Field1[GhostRed.position.y / 8][GhostRed.position.x / 8] =
								'C';
					}
					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== '!') {
						gdispFillArea(Field_X - 10, Field_Y - 10, 12, 12,
								Black);
					}

					if (Field1[GhostRed.position.y / 8][GhostRed.position.x / 8]
							== 'J') {
						gdispFillArea(Field_X - 10, Field_Y - 10, 12, 12,
								Black);
					}
					GhostRed.position.x += GhostRed.vx;
					GhostRed.position.y += GhostRed.vy;
					gdispFillCircle(GhostRed.position.x, GhostRed.position.y, 2,
							Blue);
				}


			}
		/************
		 * start Pinky movement
		 */

			pinky(&PacMan.food_collected , &PacMan.lives, PacMan.position.x , PacMan.position.y, PacMan.vx, PacMan.vy, &PacMan.chasing, restField ,buttonStatus.joystick.x, buttonStatus.joystick.y);
			inky(&PacMan.food_collected , &PacMan.lives, PacMan.position.x , PacMan.position.y, PacMan.vx, PacMan.vy, &PacMan.chasing, restField ,buttonStatus.joystick.x, buttonStatus.joystick.y);
			clyde(&PacMan.food_collected , &PacMan.lives, PacMan.position.x , PacMan.position.y, PacMan.vx, PacMan.vy, &PacMan.chasing, restField ,buttonStatus.joystick.x, buttonStatus.joystick.y);


			restField = false;

		}
	}
}



void drawTask2(void * params) {
	char str[3][70] = { "PAUSED", "can you see the problem when",
			"you have no button debouncing?" }; // buffer for messages to draw to display
	struct buttons buttonStatus; // joystick queue input buffer
	const unsigned char next_state_signal = PREV_TASK;
	uint8_t debounceFlagE = 0;

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) { // Block until screen is ready

			while (xQueueReceive(ButtonQueue, &buttonStatus, 0) == pdTRUE)
				;

			// State machine input
			if (buttonStatus.E && debounceFlagE == 0){
					debounceFlagE = 1;

				} if(!buttonStatus.E && debounceFlagE == 1){
					debounceFlagE = 1;
					xQueueSend(StateQueue, &next_state_signal, 100);
				}

			for (unsigned char i = 0; i < 3; i++)
				gdispDrawString(PAUSED_TEXT_X(str[i]), PAUSED_TEXT_Y(i), str[i],
						font1, Black);
		}
	}
}



/**
 * Example function to send data over UART
 *
 * Sends coordinates of a given position via UART.
 * Structure of a package:
 *  8 bit start byte
 *  8 bit x-coordinate
 *  8 bit y-coordinate
 *  8 bit checksum (= x-coord XOR y-coord)
 *  8 bit stop byte
 */
void sendPosition(struct buttons buttonStatus) {
	const uint8_t checksum = buttonStatus.joystick.x ^ buttonStatus.joystick.y
			^ buttonStatus.A ^ buttonStatus.B ^ buttonStatus.C ^ buttonStatus.D
			^ buttonStatus.E ^ buttonStatus.K;

	UART_SendData(startByte);
	UART_SendData(buttonStatus.joystick.x);
	UART_SendData(buttonStatus.joystick.y);
	UART_SendData(buttonStatus.A);
	UART_SendData(buttonStatus.B);
	UART_SendData(buttonStatus.C);
	UART_SendData(buttonStatus.D);
	UART_SendData(buttonStatus.E);
	UART_SendData(buttonStatus.K);
	UART_SendData(checksum);
	UART_SendData(stopByte);
}

/**
 * Example how to receive data over UART (see protocol above)
 */
void uartReceive() {
	char input;
	uint8_t pos = 0;
	char checksum;
	char buffer[11]; // Start byte,4* line byte, checksum (all xor), End byte
	struct buttons buttonStatus = { { 0 } };
	while (TRUE) {
		// wait for data in queue
		xQueueReceive(ESPL_RxQueue, &input, portMAX_DELAY);

		// decode package by buffer position
		switch (pos) {
		// start byte
		case 0:
			if (input != startByte)
				break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			// read received data in buffer
			buffer[pos] = input;
			pos++;
			break;
		case 10:
			// Check if package is corrupted
			checksum = buffer[1] ^ buffer[2] ^ buffer[3] ^ buffer[4] ^ buffer[5]
					^ buffer[6] ^ buffer[7] ^ buffer[8];
			if (input == stopByte || checksum == buffer[9]) {
				// pass position to Joystick Queue
				buttonStatus.joystick.x = buffer[1];
				buttonStatus.joystick.y = buffer[2];
				buttonStatus.A = buffer[3];
				buttonStatus.B = buffer[4];
				buttonStatus.C = buffer[5];
				buttonStatus.D = buffer[6];
				buttonStatus.E = buffer[7];
				buttonStatus.K = buffer[8];
				xQueueSend(ButtonQueue, &buttonStatus, 100);
			}
			pos = 0;
		}
	}
}

/*
 *  Hook definitions needed fCall_Tick2_signalor FreeRTOS to function.
 */
void vApplicationIdleHook() {
	while (TRUE) {
	};
}

void vApplicationMallocFailedHook() {
	while (TRUE) {
	};

}
/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
