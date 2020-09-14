#include "includes.h"
#include <math.h>
#include <stdbool.h>
#include "string.h"
#include "stdlib.h"
#include "FieldOne.h"
#include "Demo.h"
#include "buttons.h"
#include "moveLogic.h"

struct myPacMan Pinky = { { .x = 110, .y = 110, }, .vx = 0, .vy = 0, .lives = 3,
		.chasing = false, .Speed = 1, .food_collected = 0, .animation = 0,
		.color = Magenta, .startMove = false, .GOSX = 0, .GOSY = 0, .nx = 0,
		.ny = 0 };



	void pinky(int (*food_collected), int *lives, int position_x,
			int position_y, int posVX, int posVY, bool *chasing, bool StartMOVE ,int joyStick_x,
			int joyStick_y) {

		struct buttons buttonStatus;
		int posX, posY, GhostX, GhostY, reGhostX = 110, reGhostY = 110;
		float GhostcountX = 0;
		static bool DoorOpen = false;
		int diffGhostX = 0, diffGhostY = 0, savedGX, savedGY;
		int Color = Blue;
		int debounceFlagE = 1, debounceFlagA = 1;
		int z = 0;
		static bool escapeMode = false, Gchasing = false, Garrived = false,
				GhostSwitcher = false, switcher = false;

		buttonStatus.joystick.x = joyStick_x;
		buttonStatus.joystick.y = joyStick_y;

		if (escapeMode == false) {
			if (Pinky.startMove == false) {

			if (Field1[Pinky.position.y / 8][Pinky.position.x / 8]
					== ' ') {
				Field1[Pinky.position.y / 8][Pinky.position.x / 8] = 'k';
			}
			if (Field1[Pinky.position.y / 8][Pinky.position.x / 8]
					== 'M') {
				Field1[Pinky.position.y / 8][Pinky.position.x / 8] = 'K';
			}

				Pinky.vx = 1;
				GhostX = Pinky.position.x + Pinky.vx;
				GhostY = Pinky.position.y + Pinky.vy;
				if (*lives == 0) {
					Pinky.vx = 0;
					Pinky.vy = 0;
				}

				if (GhostX > Pinky.position.x
						&& Field1[(GhostY) / 8][(GhostX) / 8] == 'V') {
					switcher = true;
				}
				if (switcher == true) {
					Pinky.vx = -1;
					Pinky.position.x += Pinky.vx;
					gdispFillCircle(Pinky.position.x, Pinky.position.y, 2,
							Pinky.color);
					GhostX = Pinky.position.x + Pinky.vx;
					GhostY = Pinky.position.y + Pinky.vy;
					if (GhostX < Pinky.position.x
							&& Field1[(GhostY) / 8][(GhostX) / 8] == 'V') {
						switcher = false;
					}
				}

				if (switcher == false) {
					Pinky.vx = 1;
					Pinky.position.x += Pinky.vx;
					gdispFillCircle(Pinky.position.x, Pinky.position.y, 2,
							Pinky.color);
				}

				// Delete and open the door when move the PacMan
				for (int i = 0; i < Field_Height; i++) {
					for (int j = 0; j < Field_Width; j++) {
						if ((*food_collected) >= 800) {

							DoorOpen = true;
							Pinky.startMove = true;
						}
						if (Field1[i][j] == 'D' && DoorOpen == true) {
							Field1[i][j] = 'S';

						}

					}
				}
			}

			// Draw the Red very aggressive Ghost (Pinky)
			if (Pinky.startMove == true) {

				// move the ghost to out the box and set the manual move
				// set start point x

				if (GhostSwitcher == false) {

					if (Pinky.position.x > 108) {
						if (*lives == 0) {
							Pinky.vx = 0;
							Pinky.vy = 0;
						} else {
						if (Field1[Pinky.position.y / 8][Pinky.position.x
								/ 8] == ' ') {
							Field1[Pinky.position.y / 8][Pinky.position.x
									/ 8] = 'k';
						}
						if (Field1[Pinky.position.y / 8][Pinky.position.x
								/ 8] == 'M') {
							Field1[Pinky.position.y / 8][Pinky.position.x
									/ 8] = 'K';
						}
							if (Field1[Pinky.position.y / 8][Pinky.position.x
									/ 8] == 'S') {
								gdispFillArea(Pinky.position.x - 6,
										Pinky.position.y - 6, 12, 12, Black);
							}
							Pinky.vx = -1;
							Pinky.position.x += Pinky.vx;
						}
						gdispFillCircle(Pinky.position.x, Pinky.position.y, 2,
								Pinky.color);
					} else if (Pinky.position.x <= 108) {
						if (*lives == 0) {
							Pinky.vx = 0;
							Pinky.vy = 0;
						} else {
						if (Field1[Pinky.position.y / 8][Pinky.position.x
								/ 8] == ' ') {
							Field1[Pinky.position.y / 8][Pinky.position.x
									/ 8] = 'k';
						}
						if (Field1[Pinky.position.y / 8][Pinky.position.x
								/ 8] == 'M') {
							Field1[Pinky.position.y / 8][Pinky.position.x
									/ 8] = 'K';
						}
							if (Field1[Pinky.position.y / 8][Pinky.position.x
									/ 8] == 'S') {
								gdispFillArea(Pinky.position.x - 6,
										Pinky.position.y - 6, 12, 12, Black);
							}
							if (Field1[Pinky.position.y / 8][Pinky.position.x
									/ 8] == '.') {
								Field1[Pinky.position.y / 8][Pinky.position.x
										/ 8] = 'C';
							}
							Pinky.vx = 1;
							Pinky.position.x += Pinky.vx;
						}
					}
					gdispFillCircle(Pinky.position.x, Pinky.position.y, 2,
							Pinky.color);
					//set start point y
					if (Pinky.position.y >= 83) {
						if (*lives == 0) {
							Pinky.vx = 0;
							Pinky.vy = 0;
						} else {
						if (Field1[Pinky.position.y / 8][Pinky.position.x
								/ 8] == ' ') {
							Field1[Pinky.position.y / 8][Pinky.position.x
									/ 8] = 'k';
						}
						if (Field1[Pinky.position.y / 8][Pinky.position.x
								/ 8] == 'M') {
							Field1[Pinky.position.y / 8][Pinky.position.x
									/ 8] = 'K';
						}
							if (Field1[Pinky.position.y / 8][Pinky.position.x
									/ 8] == 'S') {
								gdispFillArea(Pinky.position.x - 6,
										Pinky.position.y - 6, 12, 12, Black);
							}
							if (Field1[Pinky.position.y / 8][Pinky.position.x
									/ 8] == '.') {
								Field1[Pinky.position.y / 8][Pinky.position.x
										/ 8] = 'C';
							}
							Pinky.vy = -1;
							Pinky.position.y += Pinky.vy;
						}
						gdispFillCircle(Pinky.position.x, Pinky.position.y, 2,
								Pinky.color);
					}
					if (Pinky.position.y == 82
							&& (Pinky.position.x == 109
									|| Pinky.position.x == 108))
						GhostSwitcher = true;

				}

				// Find the random way in the field for the ghost and true out the box

				if (GhostSwitcher == true) {
					//Generate directoins
					// 1 bottom right
					if (position_x > Pinky.position.x
							&& position_y > Pinky.position.y
							&& Pinky.GOSX != -1 && Pinky.GOSY != -1) {
						Pinky.vx = Pinky.Speed;
						Pinky.vy = Pinky.Speed;

					} else if (position_x < Pinky.position.x
							&& position_y < Pinky.position.y && Pinky.GOSX != 1
							&& Pinky.GOSY != 1) { // 2 Top left
						Pinky.vx = -Pinky.Speed;
						Pinky.vy = -Pinky.Speed;
					} else if (position_x > Pinky.position.x
							&& position_y < Pinky.position.y
							&& Pinky.GOSX != -1 && Pinky.GOSY != 1) { // 3 Top right
						Pinky.vx = Pinky.Speed;
						Pinky.vy = -Pinky.Speed;
					} else if (position_x < Pinky.position.x
							&& position_y > Pinky.position.y && Pinky.GOSX != 1
							&& Pinky.GOSY != -1) { // 4 bottom left
						Pinky.vx = -Pinky.Speed;
						Pinky.vy = Pinky.Speed;
					}

					savedGX = Pinky.vx;
					savedGY = Pinky.vy;

					GhostY = Pinky.position.y + Pinky.vy;
					if (Field1[(GhostY + (3 * Pinky.vy)) / 8][Pinky.position.x
							/ 8] == 'V'
							|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x)
									/ 8] == '#'
							|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x)
									/ 8] == 'D'
							|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x)
									/ 8] == 'b'
							|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x)
									/ 8] == 'B'
							|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x)
									/ 8] == 'R'
							|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x)
									/ 8] == 'L'
							|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x)
									/ 8] == 'S') {
						Pinky.vy = 0;
					}
					GhostX = Pinky.position.x + Pinky.vx;
					if (Field1[(Pinky.position.y) / 8][(GhostX + (3 * Pinky.vx))
							/ 8] == 'V'
							|| Field1[(Pinky.position.y) / 8][(GhostX
									+ (3 * Pinky.vx)) / 8] == '#'
							|| Field1[(Pinky.position.y) / 8][(GhostX
									+ (3 * Pinky.vx)) / 8] == 'D'
							|| Field1[(Pinky.position.y) / 8][(GhostX
									+ (3 * Pinky.vx)) / 8] == 'b'
							|| Field1[(Pinky.position.y) / 8][(GhostX
									+ (3 * Pinky.vx)) / 8] == 'B'
							|| Field1[(Pinky.position.y) / 8][(GhostX
									+ (3 * Pinky.vx)) / 8] == 'R'
							|| Field1[(Pinky.position.y) / 8][(GhostX
									+ (3 * Pinky.vx)) / 8] == 'L'
							|| Field1[(Pinky.position.y) / 8][(GhostX
									+ (3 * Pinky.vx)) / 8] == 'S') {
						Pinky.vx = 0;
					}

					// Random to the debug in corners in the same side like PacMan and debug the same line positions
					if (Pinky.vx == 0 && Pinky.vy == 0) {

						if (position_x > Pinky.position.x
								&& position_y > Pinky.position.y) {
							Pinky.vx = -Pinky.Speed;
							Pinky.vy = Pinky.Speed;

						} else if (position_x < Pinky.position.x
								&& position_y < Pinky.position.y) { // 2 Top left
							Pinky.vx = Pinky.Speed;
							Pinky.vy = -Pinky.Speed;
						} else if (position_x > Pinky.position.x
								&& position_y < Pinky.position.y) { // 3 Top right
							Pinky.vx = -Pinky.Speed;
							Pinky.vy = -Pinky.Speed;
						} else if (position_x < Pinky.position.x
								&& position_y > Pinky.position.y) { // 4 bottom left
							Pinky.vx = Pinky.Speed;
							Pinky.vy = Pinky.Speed;
						}
						//			 generate random positions to debug the same line positions
						if ((position_x == Pinky.position.x && Pinky.vx == 0
								&& Pinky.vy == 0)
								|| (position_y == Pinky.position.y
										&& Pinky.vy == 0 && Pinky.vx == 0)) {
							GhostcountX = rand() % (4);
							if ((int) GhostcountX == 0
									&& (Pinky.GOSX != -1 || Pinky.GOSY != -1)) {
								Pinky.vx = Pinky.Speed;
								Pinky.vy = Pinky.Speed;

							} else if ((int) GhostcountX == 1
									&& (Pinky.GOSX != 1 || Pinky.GOSY != 1)) {
								Pinky.vx = -Pinky.Speed;
								Pinky.vy = -Pinky.Speed;
							} else if ((int) GhostcountX == 2
									&& (Pinky.GOSX != -1 || Pinky.GOSY != 1)) {
								Pinky.vx = Pinky.Speed;
								Pinky.vy = -Pinky.Speed;

							} else if ((int) GhostcountX == 3
									&& (Pinky.GOSX != 1 || Pinky.GOSY != -1)) {
								Pinky.vx = -Pinky.Speed;
								Pinky.vy = Pinky.Speed;

							}
						}
						GhostY = Pinky.position.y + Pinky.vy;
						if (Field1[(GhostY + (3 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == 'V'
								|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x)
										/ 8] == '#'
								|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x)
										/ 8] == 'D'
								|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x)
										/ 8] == 'b'
								|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x)
										/ 8] == 'B'
								|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x)
										/ 8] == 'R'
								|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x)
										/ 8] == 'L'
								|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x)
										/ 8] == 'S') {
							Pinky.vy = 0;
						}
						GhostX = Pinky.position.x + Pinky.vx;
						if (Field1[(Pinky.position.y) / 8][(GhostX
								+ (3 * Pinky.vx)) / 8] == 'V'
								|| Field1[(Pinky.position.y) / 8][(GhostX
										+ (3 * Pinky.vx)) / 8] == '#'
								|| Field1[(Pinky.position.y) / 8][(GhostX
										+ (3 * Pinky.vx)) / 8] == 'D'
								|| Field1[(Pinky.position.y) / 8][(GhostX
										+ (3 * Pinky.vx)) / 8] == 'b'
								|| Field1[(Pinky.position.y) / 8][(GhostX
										+ (3 * Pinky.vx)) / 8] == 'B'
								|| Field1[(Pinky.position.y) / 8][(GhostX
										+ (3 * Pinky.vx)) / 8] == 'R'
								|| Field1[(Pinky.position.y) / 8][(GhostX
										+ (3 * Pinky.vx)) / 8] == 'L'
								|| Field1[(Pinky.position.y) / 8][(GhostX
										+ (3 * Pinky.vx)) / 8] == 'S') {
							Pinky.vx = 0;
						}
						Pinky.GOSX = Pinky.vx;
						Pinky.GOSY = Pinky.vy;

					}
					Pinky.GOSX = Pinky.vx;
					Pinky.GOSY = Pinky.vy;
					// tunnel right left for the ghost
					if (Field1[Pinky.position.y / 8][Pinky.position.x / 8]
							== 'r') {
						gdispFillArea(Pinky.position.x - 10,
								Pinky.position.y - 10, 10, 10, Black);
						Pinky.position.x = 210;

					} else if (Field1[Pinky.position.y / 8][Pinky.position.x / 8]
							== 'l') {
						gdispFillArea(Pinky.position.x - 10,
								Pinky.position.y - 10, 10, 10, Black);
						Pinky.position.x = 12;
					}

					if (*lives == 0) {
						Pinky.vx = 0;
						Pinky.vy = 0;
					}

				if (Field1[Pinky.position.y / 8][Pinky.position.x / 8]
						== ' ') {
					Field1[Pinky.position.y / 8][Pinky.position.x / 8] =
							'k';
				}
				if (Field1[Pinky.position.y / 8][Pinky.position.x / 8]
						== 'M') {
					Field1[Pinky.position.y / 8][Pinky.position.x / 8] =
							'K';
				}
					if (Field1[Pinky.position.y / 8][Pinky.position.x / 8]
							== '.') {
						Field1[Pinky.position.y / 8][Pinky.position.x / 8] =
								'C';
					}

					// Draw the Ghost in the filtered position
					Pinky.position.x += Pinky.vx;
					Pinky.position.y += Pinky.vy;
					gdispFillCircle(Pinky.position.x, Pinky.position.y, 2,
							Pinky.color);
				}

				if (Field1[posY / 8][posX / 8] == ':') {
					Field1[posY / 8][posX / 8] = 'J';
					escapeMode = true;
				}

			}
			if ((Pinky.position.x + (4 * Pinky.vx) == position_x + (4 * posVX)
					&& Pinky.position.y + (4 * Pinky.vy)
							== position_y + (4 * posVY))
					|| (Pinky.position.x + (8 * Pinky.vx)
							== position_x + (posVX)
							&& Pinky.position.y + (8 * Pinky.vy)
									== position_y + (posVY))
					|| (Pinky.position.x + (1 * Pinky.vx)
							== position_x + (1 * posVX)
							&& Pinky.position.y + (1 * Pinky.vy)
									== position_y + (1 * posVY))) {
				gdispFillArea(position_x - 6, position_y - 6, 13, 13, Black);
				if (*lives >= 0) {
					Pinky.startMove = false;
					GhostSwitcher = false;
					DoorOpen = false;
					Pinky.position.x = 115;
					Pinky.position.y = 110;
					(*lives)--;
					(*chasing) = true;
				}

			}
			if(*lives == 0 || StartMOVE == true){
			Pinky.startMove = false;
			GhostSwitcher = false;
			DoorOpen = false;
			Pinky.position.x = 115;
			Pinky.position.y = 110;
			}



		} else if (escapeMode == true) { // make the Ghosts escape mode. move away from Pac man
			if (Gchasing == false) {

				// Find the random way in the field for the ghost and true out the box

				//Generate directoins
				// 1 bottom right
				if (position_x > Pinky.position.x
						&& position_y > Pinky.position.y && Pinky.GOSX != -1
						&& Pinky.GOSY != -1) {
					Pinky.vx = Pinky.Speed;
					Pinky.vy = Pinky.Speed;

				} else if (position_x < Pinky.position.x
						&& position_y < Pinky.position.y && Pinky.GOSX != 1
						&& Pinky.GOSY != 1) { // 2 Top left
					Pinky.vx = -Pinky.Speed;
					Pinky.vy = -Pinky.Speed;
				} else if (position_x > Pinky.position.x
						&& position_y < Pinky.position.y && Pinky.GOSX != -1
						&& Pinky.GOSY != 1) { // 3 Top right
					Pinky.vx = Pinky.Speed;
					Pinky.vy = -Pinky.Speed;
				} else if (position_x < Pinky.position.x
						&& position_y > Pinky.position.y && Pinky.GOSX != 1
						&& Pinky.GOSY != -1) { // 4 bottom left
					Pinky.vx = -Pinky.Speed;
					Pinky.vy = Pinky.Speed;
				}

				savedGX = Pinky.vx;
				savedGY = Pinky.vy;

				GhostY = Pinky.position.y + Pinky.vy;
				if (Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
						+ (3 * Pinky.vx)) / 8] == 'V'
						|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
								+ (3 * Pinky.vx)) / 8] == '#'
						|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
								+ (3 * Pinky.vx)) / 8] == 'D'
						|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
								+ (3 * Pinky.vx)) / 8] == 'b'
						|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
								+ (3 * Pinky.vx)) / 8] == 'B'
						|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
								+ (3 * Pinky.vx)) / 8] == 'R'
						|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
								+ (3 * Pinky.vx)) / 8] == 'L'
						|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
								+ (3 * Pinky.vx)) / 8] == 'S') {
					Pinky.vy = 0;
				}
				GhostX = Pinky.position.x + Pinky.vx;
				if (Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
						+ (3 * Pinky.vx)) / 8] == 'V'
						|| Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
								+ (3 * Pinky.vx)) / 8] == '#'
						|| Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
								+ (3 * Pinky.vx)) / 8] == 'D'
						|| Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
								+ (3 * Pinky.vx)) / 8] == 'b'
						|| Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
								+ (3 * Pinky.vx)) / 8] == 'B'
						|| Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
								+ (3 * Pinky.vx)) / 8] == 'R'
						|| Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
								+ (3 * Pinky.vx)) / 8] == 'L'
						|| Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
								+ (3 * Pinky.vx)) / 8] == 'S') {
					Pinky.vx = 0;
				}

				// Random to the debug in corners in the same side like PacMan and debug the same line positions
				if (Pinky.vx == 0 && Pinky.vy == 0) {

					if (position_x > Pinky.position.x
							&& position_y > Pinky.position.y) {
						Pinky.vx = -Pinky.Speed;
						Pinky.vy = Pinky.Speed;

					} else if (position_x < Pinky.position.x
							&& position_y < Pinky.position.y) { // 2 Top left
						Pinky.vx = Pinky.Speed;
						Pinky.vy = -Pinky.Speed;
					} else if (position_x > Pinky.position.x
							&& position_y < Pinky.position.y) { // 3 Top right
						Pinky.vx = -Pinky.Speed;
						Pinky.vy = -Pinky.Speed;
					} else if (position_x < Pinky.position.x
							&& position_y > Pinky.position.y) { // 4 bottom left
						Pinky.vx = Pinky.Speed;
						Pinky.vy = Pinky.Speed;
					}
					//			 generate random positions to debug the same line positions
					if ((position_x == Pinky.position.x && Pinky.vx == 0
							&& Pinky.vy == 0)
							|| (position_y == Pinky.position.y && Pinky.vy == 0
									&& Pinky.vx == 0)) {
						GhostcountX = rand() % (4);
						if ((int) GhostcountX == 0
								&& (Pinky.GOSX != -1 || Pinky.GOSY != -1)) {
							Pinky.vx = Pinky.Speed;
							Pinky.vy = Pinky.Speed;

						} else if ((int) GhostcountX == 1
								&& (Pinky.GOSX != 1 || Pinky.GOSY != 1)) {
							Pinky.vx = -Pinky.Speed;
							Pinky.vy = -Pinky.Speed;
						} else if ((int) GhostcountX == 2
								&& (Pinky.GOSX != -1 || Pinky.GOSY != 1)) {
							Pinky.vx = Pinky.Speed;
							Pinky.vy = -Pinky.Speed;

						} else if ((int) GhostcountX == 3
								&& (Pinky.GOSX != 1 || Pinky.GOSY != -1)) {
							Pinky.vx = -Pinky.Speed;
							Pinky.vy = Pinky.Speed;

						}
					}
					GhostY = Pinky.position.y + Pinky.vy;
					if (Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
							+ (3 * Pinky.vx)) / 8] == 'V'
							|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
									+ (3 * Pinky.vx)) / 8] == '#'
							|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
									+ (3 * Pinky.vx)) / 8] == 'D'
							|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
									+ (3 * Pinky.vx)) / 8] == 'b'
							|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
									+ (3 * Pinky.vx)) / 8] == 'B'
							|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
									+ (3 * Pinky.vx)) / 8] == 'R'
							|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
									+ (3 * Pinky.vx)) / 8] == 'L'
							|| Field1[(GhostY + (3 * Pinky.vy)) / 8][(Pinky.position.x
									+ (3 * Pinky.vx)) / 8] == 'S') {
						Pinky.vy = 0;
					}
					GhostX = Pinky.position.x + Pinky.vx;
					if (Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
							+ (3 * Pinky.vx)) / 8] == 'V'
							|| Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
									+ (3 * Pinky.vx)) / 8] == '#'
							|| Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
									+ (3 * Pinky.vx)) / 8] == 'D'
							|| Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
									+ (3 * Pinky.vx)) / 8] == 'b'
							|| Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
									+ (3 * Pinky.vx)) / 8] == 'B'
							|| Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
									+ (3 * Pinky.vx)) / 8] == 'R'
							|| Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
									+ (3 * Pinky.vx)) / 8] == 'L'
							|| Field1[(Pinky.position.y + (3 * Pinky.vy)) / 8][(GhostX
									+ (3 * Pinky.vx)) / 8] == 'S') {
						Pinky.vx = 0;
					}
					Pinky.GOSX = Pinky.vx;
					Pinky.GOSY = Pinky.vy;

				}
				Pinky.GOSX = Pinky.vx;
				Pinky.GOSY = Pinky.vy;
				// tunnel right left for the ghost
				if (Field1[Pinky.position.y / 8][Pinky.position.x / 8] == 'r') {
					Pinky.position.x = 210;

				} else if (Field1[Pinky.position.y / 8][Pinky.position.x / 8]
						== 'l') {
					Pinky.position.x = 12;
				}

				if (*lives == 0) {
					Pinky.vx = 0;
					Pinky.vy = 0;
				}

				// Draw the Ghost in the filtered position
			if (Field1[Pinky.position.y / 8][Pinky.position.x / 8]
					== ' ') {
				Field1[Pinky.position.y / 8][Pinky.position.x / 8] = 'k';
			}
			if (Field1[Pinky.position.y / 8][Pinky.position.x / 8]
					== 'M') {
				Field1[Pinky.position.y / 8][Pinky.position.x / 8] = 'K';
			}
				if (Field1[Pinky.position.y / 8][Pinky.position.x / 8] == '.') {
					Field1[Pinky.position.y / 8][Pinky.position.x / 8] = 'C';
				}

				if (Field1[Pinky.position.y / 8][Pinky.position.x / 8] == '!') {
					gdispFillArea( Pinky.position.x - 10, Pinky.position.y - 10, 12, 12, Black);
				}
				if (Field1[Pinky.position.y / 8][Pinky.position.x / 8] == 'J') {
					gdispFillArea(Pinky.position.x - 10, Pinky.position.y - 10, 12, 12, Black);
				}
				Pinky.position.x += Pinky.vx;
				Pinky.position.y += Pinky.vy;
				gdispFillCircle(Pinky.position.x, Pinky.position.y, 2, Green);

				if ((Pinky.position.x + (4 * Pinky.vx)
						== position_x + (4 * posVX)
						&& Pinky.position.y + (4 * Pinky.vy)
								== position_y + (4 * posVY))
						|| (Pinky.position.x + (8 * Pinky.vx)
								== position_x + (posVX)
								&& Pinky.position.y + (8 * Pinky.vy)
										== position_y + (posVY))
						|| (Pinky.position.x + (1 * Pinky.vx)
								== position_x + (1 * posVX)
								&& Pinky.position.y + (1 * Pinky.vy)
										== position_y + (1 * posVY))) {
					(*food_collected) += 200;
					Gchasing = true;
				}

			}

			else if (Gchasing == true && Garrived == false) {
				//Generate directoins
				// 1 bottom right
				int GstartX = 110;
				int GstartY = 84;

				if (GstartX > Pinky.position.x && GstartY > Pinky.position.y
						&& Pinky.GOSX != -1 && Pinky.GOSY != -1) {
					Pinky.vx = Pinky.Speed;
					Pinky.vy = Pinky.Speed;

				} else if (GstartX < Pinky.position.x
						&& GstartY < Pinky.position.y && Pinky.GOSX != 1
						&& Pinky.GOSY != 1) { // 2 Top left
					Pinky.vx = -Pinky.Speed;
					Pinky.vy = -Pinky.Speed;

				} else if (GstartX > Pinky.position.x
						&& GstartY < Pinky.position.y && Pinky.GOSX != -1
						&& Pinky.GOSY != 1) { // 3 Top right
					Pinky.vx = Pinky.Speed;
					Pinky.vy = -Pinky.Speed;
					z = 0;
				} else if (GstartX < Pinky.position.x
						&& GstartY > Pinky.position.y && Pinky.GOSX != 1
						&& Pinky.GOSY != -1) { // 4 bottom left
					Pinky.vx = -Pinky.Speed;
					Pinky.vy = Pinky.Speed;

				} else {
					Pinky.vx = 0;
					Pinky.vy = 0;
				}

				savedGX = Pinky.vx;
				savedGY = Pinky.vy;

				GhostY = Pinky.position.y + Pinky.vy;
				if (Field1[(GhostY + (3 * Pinky.vy)) / 8][Pinky.position.x / 8]
						== 'V'
						|| Field1[(GhostY + (4 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == '#'
						|| Field1[(GhostY + (4 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == 'D'
						|| Field1[(GhostY + (4 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == 'b'
						|| Field1[(GhostY + (4 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == 'B'
						|| Field1[(GhostY + (4 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == 'R'
						|| Field1[(GhostY + (4 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == 'L'
						|| Field1[(GhostY + (4 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == 'S') {
					Pinky.vy = 0;
				}
				GhostX = Pinky.position.x + Pinky.vx;
				if (Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx)) / 8]
						== 'V'
						|| Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx))
								/ 8] == '#'
						|| Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx))
								/ 8] == 'D'
						|| Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx))
								/ 8] == 'b'
						|| Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx))
								/ 8] == 'B'
						|| Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx))
								/ 8] == 'R'
						|| Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx))
								/ 8] == 'L'
						|| Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx))
								/ 8] == 'S') {
					Pinky.vx = 0;
				}
				// make random movement to debug in position
				if (Pinky.vx == 0 && Pinky.vy == 0 && Garrived == false) {

					if (GstartX > Pinky.position.x
							&& GstartY > Pinky.position.y) {
						Pinky.vx = -Pinky.Speed;
						Pinky.vy = Pinky.Speed;

					} else if (GstartX < Pinky.position.x
							&& GstartY < Pinky.position.y) { // 2 Top left
						Pinky.vx = Pinky.Speed;
						Pinky.vy = -Pinky.Speed;
					} else if (GstartX > Pinky.position.x
							&& GstartY < Pinky.position.y) { // 3 Top right
						Pinky.vx = -Pinky.Speed;
						Pinky.vy = -Pinky.Speed;
					} else if (GstartX < Pinky.position.x
							&& GstartY > Pinky.position.y) { // 4 bottom left
						Pinky.vx = Pinky.Speed;
						Pinky.vy = Pinky.Speed;
					}

					// debug the same y move direction
					if (GstartX < Pinky.position.x && GstartY < Pinky.position.y
							&& Pinky.GOSX != 0 && Pinky.GOSY == 1) {
						Pinky.vx = -Pinky.Speed;
						Pinky.vy = Pinky.Speed;

					}

					if (GstartX > Pinky.position.x && GstartY < Pinky.position.y
							&& Pinky.GOSX != 0 && Pinky.GOSY == 1) {
						Pinky.vx = Pinky.Speed;
						Pinky.vy = Pinky.Speed;

					}

					// control the other conditions
					z++;
					Pinky.GOSX = Pinky.vx;
					Pinky.GOSY = Pinky.vy;
				}
				//			 generate random positions to debug the same line positions
				if ((GstartX == Pinky.position.x)
						|| (GstartY == Pinky.position.y)) {
					GhostcountX = rand() % (4);
					if ((int) GhostcountX == 0
							&& (Pinky.GOSX != -1 || Pinky.GOSY != -1)) {
						Pinky.vx = Pinky.Speed;
						Pinky.vy = Pinky.Speed;

					} else if ((int) GhostcountX == 1
							&& (Pinky.GOSX != 1 || Pinky.GOSY != 1)) {
						Pinky.vx = -Pinky.Speed;
						Pinky.vy = -Pinky.Speed;
					} else if ((int) GhostcountX == 2
							&& (Pinky.GOSX != -1 || Pinky.GOSY != 1)) {
						Pinky.vx = Pinky.Speed;
						Pinky.vy = -Pinky.Speed;

					} else if ((int) GhostcountX == 3
							&& (Pinky.GOSX != 1 || Pinky.GOSY != -1)) {
						Pinky.vx = -Pinky.Speed;
						Pinky.vy = Pinky.Speed;

					}
					z++;
					Pinky.GOSX = Pinky.vx;
					Pinky.GOSY = Pinky.vy;
				}
				if (Field1[(GhostY + (3 * Pinky.vy)) / 8][Pinky.position.x / 8]
						== 'V'
						|| Field1[(GhostY + (4 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == '#'
						|| Field1[(GhostY + (4 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == 'D'
						|| Field1[(GhostY + (4 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == 'b'
						|| Field1[(GhostY + (4 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == 'B'
						|| Field1[(GhostY + (4 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == 'R'
						|| Field1[(GhostY + (4 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == 'L'
						|| Field1[(GhostY + (4 * Pinky.vy)) / 8][Pinky.position.x
								/ 8] == 'S') {
					Pinky.vy = 0;
				}
				GhostX = Pinky.position.x + Pinky.vx;
				if (Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx)) / 8]
						== 'V'
						|| Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx))
								/ 8] == '#'
						|| Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx))
								/ 8] == 'D'
						|| Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx))
								/ 8] == 'b'
						|| Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx))
								/ 8] == 'B'
						|| Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx))
								/ 8] == 'R'
						|| Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx))
								/ 8] == 'L'
						|| Field1[Pinky.position.y / 8][(GhostX + (4 * Pinky.vx))
								/ 8] == 'S') {
					Pinky.vx = 0;
				}

				diffGhostX = Pinky.position.x - GstartX;
				diffGhostY = Pinky.position.y - GstartY;
				Pinky.nx = Pinky.position.x + Pinky.vx;
				Pinky.ny = Pinky.position.y + Pinky.vy;
				int m = abs(diffGhostX);
				int n = abs(diffGhostY);
				int nextX = Pinky.nx - GstartX;
				int nextY = Pinky.ny - GstartY;
				int a = abs(nextX);
				int b = abs(nextY);
				if (z >= 50) {

					if (m < a) {
						Pinky.vx = 0;
					}
					if (n < b) {
						Pinky.vy = 0;
					}

					Pinky.GOSX = Pinky.vx;
					Pinky.GOSY = Pinky.vy;
					z = 0;

				}

				if (b < n && a > m && Pinky.vy != 0) {
					Pinky.vx = 0;

				}

				if (Pinky.vx != 0 && Pinky.vy != 0) {
					if (b > a) {
						Pinky.vx = 0;
					}
					if (a > b) {
						Pinky.vy = 0;
					}

				}

				if ((Pinky.position.x + (1 * Pinky.vx) == GstartX
						&& Pinky.position.y + (1 * Pinky.vy) == GstartY)) {
					Garrived = true;
				}
			if (Field1[Pinky.position.y / 8][Pinky.position.x / 8]
					== ' ') {
				Field1[Pinky.position.y / 8][Pinky.position.x / 8] = 'k';
			}
			if (Field1[Pinky.position.y / 8][Pinky.position.x / 8]
					== 'M') {
				Field1[Pinky.position.y / 8][Pinky.position.x / 8] = 'K';
			}
				if (Field1[Pinky.position.y / 8][Pinky.position.x / 8] == '.') {
					Field1[Pinky.position.y / 8][Pinky.position.x / 8] = 'C';
				}
				if (Field1[Pinky.position.y / 8][Pinky.position.x / 8] == '!') {
					gdispFillArea(Pinky.position.x - 10, Pinky.position.y - 10, 12, 12, Black);
				}
				if (Field1[Pinky.position.y / 8][Pinky.position.x / 8] == 'J') {
					gdispFillArea(Pinky.position.x  - 10, Pinky.position.y - 10, 12, 12, Black);
				}
				Pinky.position.x += Pinky.vx;
				Pinky.position.y += Pinky.vy;
				gdispFillCircle(Pinky.position.x, Pinky.position.y, 2, Red);

			}

			if (Garrived == true) {

				Pinky.vx = 1;
				Pinky.vy = 1;
				Pinky.position.x += Pinky.vx;
				Pinky.position.y += Pinky.vy;
				if (Pinky.position.x > 120) {
					Pinky.vx = 0;
				}
				if (Pinky.position.y > 110) {
					Pinky.vy = 0;
				}

				if (Pinky.position.x >= 120 && Pinky.position.y >= 110) {
					escapeMode = false;
					Gchasing = false;
					Garrived = false;
					GhostSwitcher = false;
					Pinky.vx = 0;
					Pinky.vy = 0;
				}
			if (Field1[Pinky.position.y / 8][Pinky.position.x / 8]
					== ' ') {
				Field1[Pinky.position.y / 8][Pinky.position.x / 8] = 'k';
			}
			if (Field1[Pinky.position.y / 8][Pinky.position.x / 8]
					== 'M') {
				Field1[Pinky.position.y / 8][Pinky.position.x / 8] = 'K';
			}
				if (Field1[Pinky.position.y / 8][Pinky.position.x / 8] == '.') {
					Field1[Pinky.position.y / 8][Pinky.position.x / 8] = 'C';
				}
				if (Field1[Pinky.position.y / 8][Pinky.position.x / 8] == '!') {
					gdispFillArea(Pinky.position.x - 10, Pinky.position.y - 10, 12, 12, Black);
				}

				if (Field1[Pinky.position.y / 8][Pinky.position.x / 8] == 'J') {
					gdispFillArea(Pinky.position.x - 10, Pinky.position.y - 10, 12, 12, Black);
				}
				Pinky.position.x += Pinky.vx;
				Pinky.position.y += Pinky.vy;
				gdispFillCircle(Pinky.position.x, Pinky.position.y, 2, Blue);
			}

		}

	}
