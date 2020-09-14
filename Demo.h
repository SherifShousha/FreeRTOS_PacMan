/**
 * Function definitions for the main project file.
 *
 * @author: Jonathan Müller-Boruttau,
 * 			Tobias Fuchs tobias.fuchs@tum.de,
 * 			Nadja Peters nadja.peters@tum.de (RCS, TUM)
 */

#ifndef Demo_INCLUDED
#define Demo_INCLUDED
#include "FieldOne.h"
#include "moveLogic.h"
#include "inky.h"
#include "clyde.h"


struct coord1 {
	uint16_t x;
	uint16_t y;
};

// void RePinky(int position_x,int position.y,int lives);


struct myPacMan {
	struct coord1 position;
	int vx;
    int vy;
	int lives;
	bool chasing;
	int Speed;
	int food_collected;
	float animation;
	int color;
    bool startMove;
	int GOSX;
    int GOSY;
    int nx;
    int ny;
    char Name;
};




void PacMan();


void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize );
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize );


#endif
