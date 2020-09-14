#include "includes.h"
#include "FieldOne.h"
#include <math.h>
#include <stdbool.h>
#include "string.h"
#include "stdlib.h"
#include "moveLogic.h"


// Draw the first map
char Field1[Field_Height][Field_Width] =
{
"R############LR############L",
"V............VV............V",
"V.R##L.R###L.VV.R###L.R##L.V",
"V:b##B.b###B.bB.b###B.b##B:V",
"V..........................V",
"V.R##L.RL.R######L.RL.R##L.V",
"V.b##B.VV.b##LR##B.VV.b##B.V",
"V......VV....VV....VV......V",
"b####L.Vb##L.VV.R##BV.R####B",
"     V.VR##B.bB.b##LV.V     ",
"     V.VV..........VV.V     ",
"     V.VV.R##DD##L.VV.V     ",
"#####B.bB.V      V.bB.b#####",
"r.........V      V.........l",
"#####L.RL.V      V.RL.R#####",
"     V.VV.b######B.VV.V     ",
"     V.VV.....&....VV.V     ",
"     V.VV.R######L.VV.V     ",
"R####B.bB.b##LR##B.bB.b####L",
"V............VV............V",
"V.R##L.R###L.VV.R###L.R##L.V",
"V.b#LV.b###B.bB.b###B.VR#B.V",
"V:..VV.......$........VV..:V",
"b#L.VV.RL.R######L.RL.VV.R#B",
"R#B.bB.VV.b##LR##B.VV.bB.b#L",
"V......VV....VV....VV......V",
"V.R####Bb##L.VV.R##Bb####L.V",
"V.b########B.bB.b########B.V",
"V..........................V",
"b##########################B"
};




//MAP
void DrawField(int Color, volatile unsigned char new_screen){
	volatile unsigned char refresh = new_screen;
	if(refresh == true){
	gdispFillArea(0, 0, 320, 240, Black);
	}
	int i,j;
	int Field_X=8, Field_Y=8;
// Draw MAP
	 for (i = 0; i < Field_Height; i++)
	   {
	      for (j = 0; j <Field_Width ; j++)
	      {
	          switch(Field1[i][j]){
	          case 'R':
	        	  if(refresh == true)
	        	  gdispDrawArc(Field_X, Field_Y, 4, 90, 180, Color);
	        	  break;
	          case 'L':
	        	  if(refresh == true)
	        	  gdispDrawArc(Field_X-8, Field_Y, 4, 0, 90, Color);
	        	  break;
	          case 'b':
	        	  if(refresh == true)
	        	  gdispDrawArc(Field_X, Field_Y-8, 4, 180, 270, Color);
	        	  break;
	          case 'B':
	        	  if(refresh == true)
	        	  gdispDrawArc(Field_X-8, Field_Y-8, 4, 270, 0, Color);
	        	  break;
	          case 'D':
	        	  if(refresh == true)
	        	  gdispDrawLine(Field_X-8, Field_Y-4 , Field_X , Field_Y-4 , Red);
	        	  break;
	          case 'V':
	        	  if(refresh == true)
	        	  gdispDrawLine(Field_X-4, Field_Y-8 , Field_X-4 , Field_Y , Color);
	        	  break;
	          case '#':
	        	  if(refresh == true)
	        		  gdispDrawLine(Field_X-8, Field_Y-4 , Field_X, Field_Y-4 , Color);
	        	  break;
	          case '.':
	        	  if(refresh == true)
	        	  gdispFillCircle(Field_X-4, Field_Y-4 , 1 , Yellow);
	        	  break;
			case ':':
				gdispFillCircle(Field_X - 4, Field_Y - 4, 3, Cyan);
				break;
			case 'N':
				gdispFillCircle(Field_X-4, Field_Y-4 , 1 , Black);
				Field1[i][j] = 'M';
				break;
			case 'C':
				gdispFillArea(Field_X-10 ,Field_Y-10, 12 , 12, Black);
				gdispFillCircle(Field_X-4, Field_Y-4 , 1 , Yellow);
				break;
			case 'K':
				gdispFillArea(Field_X-10 ,Field_Y-10, 12 , 12, Black);
				break;
			case 'k':
				gdispFillArea(Field_X - 10, Field_Y - 10, 12, 12, Black);
				break;

	          }
	          Field_X+=8;
	          }
	      	  Field_X=8;
	    	  Field_Y+=8;
	      }

	}

void DrawFieldlose(){
	gdispFillArea(0, 0, 320, 240, Black);
	int i,j;
	int Field_X=8, Field_Y=8;
// Draw MAP
	 for (i = 0; i < Field_Height; i++)
	   {
	      for (j = 0; j <Field_Width ; j++)
	      {
	          switch(Field1[i][j]){
	          case 'R':
	        	  gdispDrawArc(Field_X, Field_Y, 4, 90, 180, Red);
	        	  break;
	          case 'L':
	        	  gdispDrawArc(Field_X-8, Field_Y, 4, 0, 90, Red);
	        	  break;
	          case 'b':
	        	  gdispDrawArc(Field_X, Field_Y-8, 4, 180, 270, Red);
	        	  break;
	          case 'B':
	        	  gdispDrawArc(Field_X-8, Field_Y-8, 4, 270, 0, Red);
	        	  break;
	          case 'D':
	        	  gdispDrawLine(Field_X-8, Field_Y-4 , Field_X , Field_Y-4 , Red);
	        	  break;
	          case 'V':
	        	  gdispDrawLine(Field_X-4, Field_Y-8 , Field_X-4 , Field_Y , Red);
	        	  break;
	          case '#':
	        	  gdispDrawLine(Field_X-8, Field_Y-4 , Field_X, Field_Y-4 , Red);
	        	  break;
	          case '.':
	        	  gdispFillCircle(Field_X-4, Field_Y-4 , 1 , Yellow);
	        	  break;


	          }
	          Field_X+=8;
	          }
	      	  Field_X=8;
	    	  Field_Y+=8;
	      }

	}





