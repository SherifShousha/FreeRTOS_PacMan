#include "includes.h"
#include <math.h>
#include <stdbool.h>
#include "string.h"
#include "stdlib.h"
#include "FieldOne.h"
#include "Demo.h"
#include "buttons.h"
#include "moveLogic.h"


struct myPacMan Clyde = { { .x = 95, .y = 100, }, .vx = 0, .vy = 0, .lives = 3,
		.chasing = false, .Speed = 1, .food_collected = 0, .animation = 0,
		.color = Orange, .startMove = false, .GOSX = 0, .GOSY = 0, .nx = 0, .ny =
				0 };

void clyde(int (*food_collected), int *lives, int position_x, int position_y,
		int posVX, int posVY, bool *chasing, bool StartMOVE, int joyStick_x,
		int joyStick_y) {
	struct buttons buttonStatus;
	int posX, posY, GhostX, GhostY, reGhostX = 110, reGhostY = 100;
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
		if (Clyde.startMove == false) {

			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == ' ') {
				Field1[Clyde.position.y / 8][Clyde.position.x / 8] = 'k';
			}
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == 'M') {
				Field1[Clyde.position.y / 8][Clyde.position.x / 8] = 'K';
			}

			Clyde.vx = 1;
			GhostX = Clyde.position.x + Clyde.vx;
			GhostY = Clyde.position.y + Clyde.vy;
			if (*lives == 0) {
				Clyde.vx = 0;
				Clyde.vy = 0;
			}

			if (GhostX > Clyde.position.x
					&& Field1[(GhostY) / 8][(GhostX) / 8] == 'V') {
				switcher = true;
			}
			if (switcher == true) {
				Clyde.vx = -1;
				Clyde.position.x += Clyde.vx;
				gdispFillCircle(Clyde.position.x, Clyde.position.y, 2,
						Clyde.color);
				GhostX = Clyde.position.x + Clyde.vx;
				GhostY = Clyde.position.y + Clyde.vy;
				if (GhostX < Clyde.position.x
						&& Field1[(GhostY) / 8][(GhostX) / 8] == 'V') {
					switcher = false;
				}
			}

			if (switcher == false) {
				Clyde.vx = 1;
				Clyde.position.x += Clyde.vx;
				gdispFillCircle(Clyde.position.x, Clyde.position.y, 2,
						Clyde.color);
			}

			// Delete and open the door when move the PacMan
			for (int i = 0; i < Field_Height; i++) {
				for (int j = 0; j < Field_Width; j++) {
					if ((*food_collected) >= 2500) {

						DoorOpen = true;
						Clyde.startMove = true;
					}
					if (Field1[i][j] == 'D' && DoorOpen == true) {
						Field1[i][j] = 'S';

					}

				}
			}
		}

		// Draw the Red very aggressive Ghost (Clyde)
		if (Clyde.startMove == true) {

			// move the ghost to out the box and set the manual move
			// set start point x

			if (GhostSwitcher == false) {

				if (Clyde.position.x > 108) {
					if (*lives == 0) {
						Clyde.vx = 0;
						Clyde.vy = 0;
					} else {
						if (Field1[Clyde.position.y / 8][Clyde.position.x / 8]
								== ' ') {
							Field1[Clyde.position.y / 8][Clyde.position.x / 8] =
									'k';
						}
						if (Field1[Clyde.position.y / 8][Clyde.position.x / 8]
								== 'M') {
							Field1[Clyde.position.y / 8][Clyde.position.x / 8] =
									'K';
						}
						if (Field1[Clyde.position.y / 8][Clyde.position.x / 8]
								== 'S') {
							gdispFillArea(Clyde.position.x - 6,
									Clyde.position.y - 6, 12, 12, Black);
						}
						Clyde.vx = -1;
						Clyde.position.x += Clyde.vx;
					}
					gdispFillCircle(Clyde.position.x, Clyde.position.y, 2,
							Clyde.color);
				} else if (Clyde.position.x <= 108) {
					if (*lives == 0) {
						Clyde.vx = 0;
						Clyde.vy = 0;
					} else {
						if (Field1[Clyde.position.y / 8][Clyde.position.x / 8]
								== ' ') {
							Field1[Clyde.position.y / 8][Clyde.position.x / 8] =
									'k';
						}
						if (Field1[Clyde.position.y / 8][Clyde.position.x / 8]
								== 'M') {
							Field1[Clyde.position.y / 8][Clyde.position.x / 8] =
									'K';
						}
						if (Field1[Clyde.position.y / 8][Clyde.position.x / 8]
								== 'S') {
							gdispFillArea(Clyde.position.x - 6,
									Clyde.position.y - 6, 12, 12, Black);
						}
						if (Field1[Clyde.position.y / 8][Clyde.position.x / 8]
								== '.') {
							Field1[Clyde.position.y / 8][Clyde.position.x / 8] =
									'C';
						}
						Clyde.vx = 1;
						Clyde.position.x += Clyde.vx;
					}
				}
				gdispFillCircle(Clyde.position.x, Clyde.position.y, 2,
						Clyde.color);
				//set start point y
				if (Clyde.position.y >= 83) {
					if (*lives == 0) {
						Clyde.vx = 0;
						Clyde.vy = 0;
					} else {
						if (Field1[Clyde.position.y / 8][Clyde.position.x / 8]
								== ' ') {
							Field1[Clyde.position.y / 8][Clyde.position.x / 8] =
									'k';
						}
						if (Field1[Clyde.position.y / 8][Clyde.position.x / 8]
								== 'M') {
							Field1[Clyde.position.y / 8][Clyde.position.x / 8] =
									'K';
						}
						if (Field1[Clyde.position.y / 8][Clyde.position.x / 8]
								== 'S') {
							gdispFillArea(Clyde.position.x - 6,
									Clyde.position.y - 6, 12, 12, Black);
						}
						if (Field1[Clyde.position.y / 8][Clyde.position.x / 8]
								== '.') {
							Field1[Clyde.position.y / 8][Clyde.position.x / 8] =
									'C';
						}
						Clyde.vy = -1;
						Clyde.position.y += Clyde.vy;
					}
					gdispFillCircle(Clyde.position.x, Clyde.position.y, 2,
							Clyde.color);
				}
				if (Clyde.position.y == 82
						&& (Clyde.position.x == 109 || Clyde.position.x == 108))
					GhostSwitcher = true;

			}

			// Find the random way in the field for the ghost and true out the box

			if (GhostSwitcher == true) {
				//Generate directoins
				// 1 bottom right
				if (position_x > Clyde.position.x && position_y > Clyde.position.y
						&& Clyde.GOSX != -1 && Clyde.GOSY != -1) {
					Clyde.vx = Clyde.Speed;
					Clyde.vy = Clyde.Speed;

				} else if (position_x < Clyde.position.x
						&& position_y < Clyde.position.y && Clyde.GOSX != 1
						&& Clyde.GOSY != 1) { // 2 Top left
					Clyde.vx = -Clyde.Speed;
					Clyde.vy = -Clyde.Speed;
				} else if (position_x > Clyde.position.x
						&& position_y < Clyde.position.y && Clyde.GOSX != -1
						&& Clyde.GOSY != 1) { // 3 Top right
					Clyde.vx = Clyde.Speed;
					Clyde.vy = -Clyde.Speed;
				} else if (position_x < Clyde.position.x
						&& position_y > Clyde.position.y && Clyde.GOSX != 1
						&& Clyde.GOSY != -1) { // 4 bottom left
					Clyde.vx = -Clyde.Speed;
					Clyde.vy = Clyde.Speed;
				}

				savedGX = Clyde.vx;
				savedGY = Clyde.vy;

				GhostY = Clyde.position.y + Clyde.vy;
				if (Field1[(GhostY + (3 * Clyde.vy)) / 8][Clyde.position.x / 8]
						== 'V'
						|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x)
								/ 8] == '#'
						|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x)
								/ 8] == 'D'
						|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x)
								/ 8] == 'b'
						|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x)
								/ 8] == 'B'
						|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x)
								/ 8] == 'R'
						|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x)
								/ 8] == 'L'
						|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x)
								/ 8] == 'S') {
					Clyde.vy = 0;
				}
				GhostX = Clyde.position.x + Clyde.vx;
				if (Field1[(Clyde.position.y) / 8][(GhostX + (3 * Clyde.vx)) / 8]
						== 'V'
						|| Field1[(Clyde.position.y) / 8][(GhostX + (3 * Clyde.vx))
								/ 8] == '#'
						|| Field1[(Clyde.position.y) / 8][(GhostX + (3 * Clyde.vx))
								/ 8] == 'D'
						|| Field1[(Clyde.position.y) / 8][(GhostX + (3 * Clyde.vx))
								/ 8] == 'b'
						|| Field1[(Clyde.position.y) / 8][(GhostX + (3 * Clyde.vx))
								/ 8] == 'B'
						|| Field1[(Clyde.position.y) / 8][(GhostX + (3 * Clyde.vx))
								/ 8] == 'R'
						|| Field1[(Clyde.position.y) / 8][(GhostX + (3 * Clyde.vx))
								/ 8] == 'L'
						|| Field1[(Clyde.position.y) / 8][(GhostX + (3 * Clyde.vx))
								/ 8] == 'S') {
					Clyde.vx = 0;
				}

				// Random to the debug in corners in the same side like PacMan and debug the same line positions
				if (Clyde.vx == 0 && Clyde.vy == 0) {

					if (position_x > Clyde.position.x
							&& position_y > Clyde.position.y) {
						Clyde.vx = -Clyde.Speed;
						Clyde.vy = Clyde.Speed;

					} else if (position_x < Clyde.position.x
							&& position_y < Clyde.position.y) { // 2 Top left
						Clyde.vx = Clyde.Speed;
						Clyde.vy = -Clyde.Speed;
					} else if (position_x > Clyde.position.x
							&& position_y < Clyde.position.y) { // 3 Top right
						Clyde.vx = -Clyde.Speed;
						Clyde.vy = -Clyde.Speed;
					} else if (position_x < Clyde.position.x
							&& position_y > Clyde.position.y) { // 4 bottom left
						Clyde.vx = Clyde.Speed;
						Clyde.vy = Clyde.Speed;
					}
					//			 generate random positions to debug the same line positions
					if ((position_x == Clyde.position.x && Clyde.vx == 0
							&& Clyde.vy == 0)
							|| (position_y == Clyde.position.y && Clyde.vy == 0
									&& Clyde.vx == 0)) {
						GhostcountX = rand() % (4);
						if ((int) GhostcountX == 0
								&& (Clyde.GOSX != -1 || Clyde.GOSY != -1)) {
							Clyde.vx = Clyde.Speed;
							Clyde.vy = Clyde.Speed;

						} else if ((int) GhostcountX == 1
								&& (Clyde.GOSX != 1 || Clyde.GOSY != 1)) {
							Clyde.vx = -Clyde.Speed;
							Clyde.vy = -Clyde.Speed;
						} else if ((int) GhostcountX == 2
								&& (Clyde.GOSX != -1 || Clyde.GOSY != 1)) {
							Clyde.vx = Clyde.Speed;
							Clyde.vy = -Clyde.Speed;

						} else if ((int) GhostcountX == 3
								&& (Clyde.GOSX != 1 || Clyde.GOSY != -1)) {
							Clyde.vx = -Clyde.Speed;
							Clyde.vy = Clyde.Speed;

						}
					}
					GhostY = Clyde.position.y + Clyde.vy;
					if (Field1[(GhostY + (3 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== 'V'
							|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x)
									/ 8] == '#'
							|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x)
									/ 8] == 'D'
							|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x)
									/ 8] == 'b'
							|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x)
									/ 8] == 'B'
							|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x)
									/ 8] == 'R'
							|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x)
									/ 8] == 'L'
							|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x)
									/ 8] == 'S') {
						Clyde.vy = 0;
					}
					GhostX = Clyde.position.x + Clyde.vx;
					if (Field1[(Clyde.position.y) / 8][(GhostX + (3 * Clyde.vx))
							/ 8] == 'V'
							|| Field1[(Clyde.position.y) / 8][(GhostX
									+ (3 * Clyde.vx)) / 8] == '#'
							|| Field1[(Clyde.position.y) / 8][(GhostX
									+ (3 * Clyde.vx)) / 8] == 'D'
							|| Field1[(Clyde.position.y) / 8][(GhostX
									+ (3 * Clyde.vx)) / 8] == 'b'
							|| Field1[(Clyde.position.y) / 8][(GhostX
									+ (3 * Clyde.vx)) / 8] == 'B'
							|| Field1[(Clyde.position.y) / 8][(GhostX
									+ (3 * Clyde.vx)) / 8] == 'R'
							|| Field1[(Clyde.position.y) / 8][(GhostX
									+ (3 * Clyde.vx)) / 8] == 'L'
							|| Field1[(Clyde.position.y) / 8][(GhostX
									+ (3 * Clyde.vx)) / 8] == 'S') {
						Clyde.vx = 0;
					}
					Clyde.GOSX = Clyde.vx;
					Clyde.GOSY = Clyde.vy;

				}
				Clyde.GOSX = Clyde.vx;
				Clyde.GOSY = Clyde.vy;
				// tunnel right left for the ghost
				if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == 'r') {
					gdispFillArea(Clyde.position.x - 10, Clyde.position.y - 10,
							10, 10, Black);
					Clyde.position.x = 210;

				} else if (Field1[Clyde.position.y / 8][Clyde.position.x / 8]
						== 'l') {
					gdispFillArea(Clyde.position.x - 10, Clyde.position.y - 10,
							10, 10, Black);
					Clyde.position.x = 12;
				}

				if (*lives == 0) {
					Clyde.vx = 0;
					Clyde.vy = 0;
				}

				if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == ' ') {
					Field1[Clyde.position.y / 8][Clyde.position.x / 8] = 'k';
				}
				if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == 'M') {
					Field1[Clyde.position.y / 8][Clyde.position.x / 8] = 'K';
				}
				if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == '.') {
					Field1[Clyde.position.y / 8][Clyde.position.x / 8] = 'C';
				}

				// Draw the Ghost in the filtered position
				Clyde.position.x += Clyde.vx;
				Clyde.position.y += Clyde.vy;
				gdispFillCircle(Clyde.position.x, Clyde.position.y, 2,
						Clyde.color);
			}

			if (Field1[posY / 8][posX / 8] == ':') {
				Field1[posY / 8][posX / 8] = 'J';
				escapeMode = true;
			}

		}
		if ((Clyde.position.x + (4 * Clyde.vx) == position_x + (4 * posVX)
				&& Clyde.position.y + (4 * Clyde.vy) == position_y + (4 * posVY))
				|| (Clyde.position.x + (8 * Clyde.vx) == position_x + (posVX)
						&& Clyde.position.y + (8 * Clyde.vy)
								== position_y + (posVY))
				|| (Clyde.position.x + (1 * Clyde.vx) == position_x + (1 * posVX)
						&& Clyde.position.y + (1 * Clyde.vy)
								== position_y + (1 * posVY))) {
			gdispFillArea(position_x - 6, position_y - 6, 13, 13, Black);
			if (*lives >= 0) {
				Clyde.startMove = false;
				GhostSwitcher = false;
				DoorOpen = false;
				Clyde.position.x = 95;
				Clyde.position.y = 100;
				(*lives)--;
				(*chasing) = true;
			}

		}
		if (*lives == 0 || StartMOVE == true) {
			Clyde.startMove = false;
			GhostSwitcher = false;
			DoorOpen = false;
			Clyde.position.x = 95;
			Clyde.position.y = 100;
		}

	} else if (escapeMode == true) { // make the Ghosts escape mode. move away from Pac man
		if (Gchasing == false) {

			// Find the random way in the field for the ghost and true out the box

			//Generate directoins
			// 1 bottom right
			if (position_x > Clyde.position.x && position_y > Clyde.position.y
					&& Clyde.GOSX != -1 && Clyde.GOSY != -1) {
				Clyde.vx = Clyde.Speed;
				Clyde.vy = Clyde.Speed;

			} else if (position_x < Clyde.position.x
					&& position_y < Clyde.position.y && Clyde.GOSX != 1
					&& Clyde.GOSY != 1) { // 2 Top left
				Clyde.vx = -Clyde.Speed;
				Clyde.vy = -Clyde.Speed;
			} else if (position_x > Clyde.position.x
					&& position_y < Clyde.position.y && Clyde.GOSX != -1
					&& Clyde.GOSY != 1) { // 3 Top right
				Clyde.vx = Clyde.Speed;
				Clyde.vy = -Clyde.Speed;
			} else if (position_x < Clyde.position.x
					&& position_y > Clyde.position.y && Clyde.GOSX != 1
					&& Clyde.GOSY != -1) { // 4 bottom left
				Clyde.vx = -Clyde.Speed;
				Clyde.vy = Clyde.Speed;
			}

			savedGX = Clyde.vx;
			savedGY = Clyde.vy;

			GhostY = Clyde.position.y + Clyde.vy;
			if (Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
					+ (3 * Clyde.vx)) / 8] == 'V'
					|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
							+ (3 * Clyde.vx)) / 8] == '#'
					|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
							+ (3 * Clyde.vx)) / 8] == 'D'
					|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
							+ (3 * Clyde.vx)) / 8] == 'b'
					|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
							+ (3 * Clyde.vx)) / 8] == 'B'
					|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
							+ (3 * Clyde.vx)) / 8] == 'R'
					|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
							+ (3 * Clyde.vx)) / 8] == 'L'
					|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
							+ (3 * Clyde.vx)) / 8] == 'S') {
				Clyde.vy = 0;
			}
			GhostX = Clyde.position.x + Clyde.vx;
			if (Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
					+ (3 * Clyde.vx)) / 8] == 'V'
					|| Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
							+ (3 * Clyde.vx)) / 8] == '#'
					|| Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
							+ (3 * Clyde.vx)) / 8] == 'D'
					|| Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
							+ (3 * Clyde.vx)) / 8] == 'b'
					|| Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
							+ (3 * Clyde.vx)) / 8] == 'B'
					|| Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
							+ (3 * Clyde.vx)) / 8] == 'R'
					|| Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
							+ (3 * Clyde.vx)) / 8] == 'L'
					|| Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
							+ (3 * Clyde.vx)) / 8] == 'S') {
				Clyde.vx = 0;
			}

			// Random to the debug in corners in the same side like PacMan and debug the same line positions
			if (Clyde.vx == 0 && Clyde.vy == 0) {

				if (position_x > Clyde.position.x
						&& position_y > Clyde.position.y) {
					Clyde.vx = -Clyde.Speed;
					Clyde.vy = Clyde.Speed;

				} else if (position_x < Clyde.position.x
						&& position_y < Clyde.position.y) { // 2 Top left
					Clyde.vx = Clyde.Speed;
					Clyde.vy = -Clyde.Speed;
				} else if (position_x > Clyde.position.x
						&& position_y < Clyde.position.y) { // 3 Top right
					Clyde.vx = -Clyde.Speed;
					Clyde.vy = -Clyde.Speed;
				} else if (position_x < Clyde.position.x
						&& position_y > Clyde.position.y) { // 4 bottom left
					Clyde.vx = Clyde.Speed;
					Clyde.vy = Clyde.Speed;
				}
				//			 generate random positions to debug the same line positions
				if ((position_x == Clyde.position.x && Clyde.vx == 0
						&& Clyde.vy == 0)
						|| (position_y == Clyde.position.y && Clyde.vy == 0
								&& Clyde.vx == 0)) {
					GhostcountX = rand() % (4);
					if ((int) GhostcountX == 0
							&& (Clyde.GOSX != -1 || Clyde.GOSY != -1)) {
						Clyde.vx = Clyde.Speed;
						Clyde.vy = Clyde.Speed;

					} else if ((int) GhostcountX == 1
							&& (Clyde.GOSX != 1 || Clyde.GOSY != 1)) {
						Clyde.vx = -Clyde.Speed;
						Clyde.vy = -Clyde.Speed;
					} else if ((int) GhostcountX == 2
							&& (Clyde.GOSX != -1 || Clyde.GOSY != 1)) {
						Clyde.vx = Clyde.Speed;
						Clyde.vy = -Clyde.Speed;

					} else if ((int) GhostcountX == 3
							&& (Clyde.GOSX != 1 || Clyde.GOSY != -1)) {
						Clyde.vx = -Clyde.Speed;
						Clyde.vy = Clyde.Speed;

					}
				}
				GhostY = Clyde.position.y + Clyde.vy;
				if (Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
						+ (3 * Clyde.vx)) / 8] == 'V'
						|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
								+ (3 * Clyde.vx)) / 8] == '#'
						|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
								+ (3 * Clyde.vx)) / 8] == 'D'
						|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
								+ (3 * Clyde.vx)) / 8] == 'b'
						|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
								+ (3 * Clyde.vx)) / 8] == 'B'
						|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
								+ (3 * Clyde.vx)) / 8] == 'R'
						|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
								+ (3 * Clyde.vx)) / 8] == 'L'
						|| Field1[(GhostY + (3 * Clyde.vy)) / 8][(Clyde.position.x
								+ (3 * Clyde.vx)) / 8] == 'S') {
					Clyde.vy = 0;
				}
				GhostX = Clyde.position.x + Clyde.vx;
				if (Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
						+ (3 * Clyde.vx)) / 8] == 'V'
						|| Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
								+ (3 * Clyde.vx)) / 8] == '#'
						|| Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
								+ (3 * Clyde.vx)) / 8] == 'D'
						|| Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
								+ (3 * Clyde.vx)) / 8] == 'b'
						|| Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
								+ (3 * Clyde.vx)) / 8] == 'B'
						|| Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
								+ (3 * Clyde.vx)) / 8] == 'R'
						|| Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
								+ (3 * Clyde.vx)) / 8] == 'L'
						|| Field1[(Clyde.position.y + (3 * Clyde.vy)) / 8][(GhostX
								+ (3 * Clyde.vx)) / 8] == 'S') {
					Clyde.vx = 0;
				}
				Clyde.GOSX = Clyde.vx;
				Clyde.GOSY = Clyde.vy;

			}
			Clyde.GOSX = Clyde.vx;
			Clyde.GOSY = Clyde.vy;
			// tunnel right left for the ghost
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == 'r') {
				Clyde.position.x = 210;

			} else if (Field1[Clyde.position.y / 8][Clyde.position.x / 8]
					== 'l') {
				Clyde.position.x = 12;
			}

			if (*lives == 0) {
				Clyde.vx = 0;
				Clyde.vy = 0;
			}

			// Draw the Ghost in the filtered position
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == ' ') {
				Field1[Clyde.position.y / 8][Clyde.position.x / 8] = 'k';
			}
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == 'M') {
				Field1[Clyde.position.y / 8][Clyde.position.x / 8] = 'K';
			}
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == '.') {
				Field1[Clyde.position.y / 8][Clyde.position.x / 8] = 'C';
			}

			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == '!') {
				gdispFillArea(Clyde.position.x - 10, Clyde.position.y - 10, 12,
						12, Black);
			}
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == 'J') {
				gdispFillArea(Clyde.position.x - 10, Clyde.position.y - 10, 12,
						12, Black);
			}
			Clyde.position.x += Clyde.vx;
			Clyde.position.y += Clyde.vy;
			gdispFillCircle(Clyde.position.x, Clyde.position.y, 2, Green);

			if ((Clyde.position.x + (4 * Clyde.vx) == position_x + (4 * posVX)
					&& Clyde.position.y + (4 * Clyde.vy)
							== position_y + (4 * posVY))
					|| (Clyde.position.x + (8 * Clyde.vx) == position_x + (posVX)
							&& Clyde.position.y + (8 * Clyde.vy)
									== position_y + (posVY))
					|| (Clyde.position.x + (1 * Clyde.vx)
							== position_x + (1 * posVX)
							&& Clyde.position.y + (1 * Clyde.vy)
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

			if (GstartX > Clyde.position.x && GstartY > Clyde.position.y
					&& Clyde.GOSX != -1 && Clyde.GOSY != -1) {
				Clyde.vx = Clyde.Speed;
				Clyde.vy = Clyde.Speed;

			} else if (GstartX < Clyde.position.x && GstartY < Clyde.position.y
					&& Clyde.GOSX != 1 && Clyde.GOSY != 1) { // 2 Top left
				Clyde.vx = -Clyde.Speed;
				Clyde.vy = -Clyde.Speed;

			} else if (GstartX > Clyde.position.x && GstartY < Clyde.position.y
					&& Clyde.GOSX != -1 && Clyde.GOSY != 1) { // 3 Top right
				Clyde.vx = Clyde.Speed;
				Clyde.vy = -Clyde.Speed;
				z = 0;
			} else if (GstartX < Clyde.position.x && GstartY > Clyde.position.y
					&& Clyde.GOSX != 1 && Clyde.GOSY != -1) { // 4 bottom left
				Clyde.vx = -Clyde.Speed;
				Clyde.vy = Clyde.Speed;

			} else {
				Clyde.vx = 0;
				Clyde.vy = 0;
			}

			savedGX = Clyde.vx;
			savedGY = Clyde.vy;

			GhostY = Clyde.position.y + Clyde.vy;
			if (Field1[(GhostY + (3 * Clyde.vy)) / 8][Clyde.position.x / 8] == 'V'
					|| Field1[(GhostY + (4 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== '#'
					|| Field1[(GhostY + (4 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== 'D'
					|| Field1[(GhostY + (4 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== 'b'
					|| Field1[(GhostY + (4 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== 'B'
					|| Field1[(GhostY + (4 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== 'R'
					|| Field1[(GhostY + (4 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== 'L'
					|| Field1[(GhostY + (4 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== 'S') {
				Clyde.vy = 0;
			}
			GhostX = Clyde.position.x + Clyde.vx;
			if (Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8] == 'V'
					|| Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8]
							== '#'
					|| Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8]
							== 'D'
					|| Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8]
							== 'b'
					|| Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8]
							== 'B'
					|| Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8]
							== 'R'
					|| Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8]
							== 'L'
					|| Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8]
							== 'S') {
				Clyde.vx = 0;
			}
			// make random movement to debug in position
			if (Clyde.vx == 0 && Clyde.vy == 0 && Garrived == false) {

				if (GstartX > Clyde.position.x && GstartY > Clyde.position.y) {
					Clyde.vx = -Clyde.Speed;
					Clyde.vy = Clyde.Speed;

				} else if (GstartX < Clyde.position.x
						&& GstartY < Clyde.position.y) { // 2 Top left
					Clyde.vx = Clyde.Speed;
					Clyde.vy = -Clyde.Speed;
				} else if (GstartX > Clyde.position.x
						&& GstartY < Clyde.position.y) { // 3 Top right
					Clyde.vx = -Clyde.Speed;
					Clyde.vy = -Clyde.Speed;
				} else if (GstartX < Clyde.position.x
						&& GstartY > Clyde.position.y) { // 4 bottom left
					Clyde.vx = Clyde.Speed;
					Clyde.vy = Clyde.Speed;
				}

				// debug the same y move direction
				if (GstartX < Clyde.position.x && GstartY < Clyde.position.y
						&& Clyde.GOSX != 0 && Clyde.GOSY == 1) {
					Clyde.vx = -Clyde.Speed;
					Clyde.vy = Clyde.Speed;

				}

				if (GstartX > Clyde.position.x && GstartY < Clyde.position.y
						&& Clyde.GOSX != 0 && Clyde.GOSY == 1) {
					Clyde.vx = Clyde.Speed;
					Clyde.vy = Clyde.Speed;

				}

				// control the other conditions
				z++;
				Clyde.GOSX = Clyde.vx;
				Clyde.GOSY = Clyde.vy;
			}
			//			 generate random positions to debug the same line positions
			if ((GstartX == Clyde.position.x) || (GstartY == Clyde.position.y)) {
				GhostcountX = rand() % (4);
				if ((int) GhostcountX == 0
						&& (Clyde.GOSX != -1 || Clyde.GOSY != -1)) {
					Clyde.vx = Clyde.Speed;
					Clyde.vy = Clyde.Speed;

				} else if ((int) GhostcountX == 1
						&& (Clyde.GOSX != 1 || Clyde.GOSY != 1)) {
					Clyde.vx = -Clyde.Speed;
					Clyde.vy = -Clyde.Speed;
				} else if ((int) GhostcountX == 2
						&& (Clyde.GOSX != -1 || Clyde.GOSY != 1)) {
					Clyde.vx = Clyde.Speed;
					Clyde.vy = -Clyde.Speed;

				} else if ((int) GhostcountX == 3
						&& (Clyde.GOSX != 1 || Clyde.GOSY != -1)) {
					Clyde.vx = -Clyde.Speed;
					Clyde.vy = Clyde.Speed;

				}
				z++;
				Clyde.GOSX = Clyde.vx;
				Clyde.GOSY = Clyde.vy;
			}
			if (Field1[(GhostY + (3 * Clyde.vy)) / 8][Clyde.position.x / 8] == 'V'
					|| Field1[(GhostY + (4 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== '#'
					|| Field1[(GhostY + (4 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== 'D'
					|| Field1[(GhostY + (4 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== 'b'
					|| Field1[(GhostY + (4 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== 'B'
					|| Field1[(GhostY + (4 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== 'R'
					|| Field1[(GhostY + (4 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== 'L'
					|| Field1[(GhostY + (4 * Clyde.vy)) / 8][Clyde.position.x / 8]
							== 'S') {
				Clyde.vy = 0;
			}
			GhostX = Clyde.position.x + Clyde.vx;
			if (Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8] == 'V'
					|| Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8]
							== '#'
					|| Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8]
							== 'D'
					|| Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8]
							== 'b'
					|| Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8]
							== 'B'
					|| Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8]
							== 'R'
					|| Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8]
							== 'L'
					|| Field1[Clyde.position.y / 8][(GhostX + (4 * Clyde.vx)) / 8]
							== 'S') {
				Clyde.vx = 0;
			}

			diffGhostX = Clyde.position.x - GstartX;
			diffGhostY = Clyde.position.y - GstartY;
			Clyde.nx = Clyde.position.x + Clyde.vx;
			Clyde.ny = Clyde.position.y + Clyde.vy;
			int m = abs(diffGhostX);
			int n = abs(diffGhostY);
			int nextX = Clyde.nx - GstartX;
			int nextY = Clyde.ny - GstartY;
			int a = abs(nextX);
			int b = abs(nextY);
			if (z >= 50) {

				if (m < a) {
					Clyde.vx = 0;
				}
				if (n < b) {
					Clyde.vy = 0;
				}

				Clyde.GOSX = Clyde.vx;
				Clyde.GOSY = Clyde.vy;
				z = 0;

			}

			if (b < n && a > m && Clyde.vy != 0) {
				Clyde.vx = 0;

			}

			if (Clyde.vx != 0 && Clyde.vy != 0) {
				if (b > a) {
					Clyde.vx = 0;
				}
				if (a > b) {
					Clyde.vy = 0;
				}

			}

			if ((Clyde.position.x + (1 * Clyde.vx) == GstartX
					&& Clyde.position.y + (1 * Clyde.vy) == GstartY)) {
				Garrived = true;
			}
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == ' ') {
				Field1[Clyde.position.y / 8][Clyde.position.x / 8] = 'k';
			}
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == 'M') {
				Field1[Clyde.position.y / 8][Clyde.position.x / 8] = 'K';
			}
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == '.') {
				Field1[Clyde.position.y / 8][Clyde.position.x / 8] = 'C';
			}
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == '!') {
				gdispFillArea(Clyde.position.x - 10, Clyde.position.y - 10, 12,
						12, Black);
			}
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == 'J') {
				gdispFillArea(Clyde.position.x - 10, Clyde.position.y - 10, 12,
						12, Black);
			}
			Clyde.position.x += Clyde.vx;
			Clyde.position.y += Clyde.vy;
			gdispFillCircle(Clyde.position.x, Clyde.position.y, 2, Red);

		}

		if (Garrived == true) {

			Clyde.vx = 1;
			Clyde.vy = 1;
			Clyde.position.x += Clyde.vx;
			Clyde.position.y += Clyde.vy;
			if (Clyde.position.x > 120) {
				Clyde.vx = 0;
			}
			if (Clyde.position.y > 110) {
				Clyde.vy = 0;
			}

			if (Clyde.position.x >= 120 && Clyde.position.y >= 110) {
				escapeMode = false;
				Gchasing = false;
				Garrived = false;
				GhostSwitcher = false;
				Clyde.vx = 0;
				Clyde.vy = 0;
			}
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == ' ') {
				Field1[Clyde.position.y / 8][Clyde.position.x / 8] = 'k';
			}
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == 'M') {
				Field1[Clyde.position.y / 8][Clyde.position.x / 8] = 'K';
			}
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == '.') {
				Field1[Clyde.position.y / 8][Clyde.position.x / 8] = 'C';
			}
			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == '!') {
				gdispFillArea(Clyde.position.x - 10, Clyde.position.y - 10, 12,
						12, Black);
			}

			if (Field1[Clyde.position.y / 8][Clyde.position.x / 8] == 'J') {
				gdispFillArea(Clyde.position.x - 10, Clyde.position.y - 10, 12,
						12, Black);
			}
			Clyde.position.x += Clyde.vx;
			Clyde.position.y += Clyde.vy;
			gdispFillCircle(Clyde.position.x, Clyde.position.y, 2, Blue);
		}

	}

}
