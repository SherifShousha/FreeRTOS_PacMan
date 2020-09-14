#include "includes.h"
#include <math.h>
#include <stdbool.h>
#include "string.h"
#include "stdlib.h"
#include "Demo.h"

#define Field_Height  30
#define Field_Width   28

extern  char Field1[Field_Height][Field_Width];


void DrawField(int Color, volatile unsigned char new_screen);
void DrawFieldlose();




