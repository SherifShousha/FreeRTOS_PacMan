/*
 * buttons.h
 *
 *  Created on: Jul 4, 2019
 *      Author: lab_espl_stud16
 */

#ifndef CODE_BUTTONS_H_
#define CODE_BUTTONS_H_

#include "FreeRTOS.h"
#include "queue.h"
#include "stdint.h"

struct coord {
	uint8_t x;
	uint8_t y;
};


struct buttons {
    struct coord joystick;
    unsigned char A;
    unsigned char B;
    unsigned char C;
    unsigned char D;
    unsigned char E;
    unsigned char K;
};

void checkButtons(void * params);

extern QueueHandle_t ButtonQueue;

#endif /* CODE_BUTTONS_H_ */
