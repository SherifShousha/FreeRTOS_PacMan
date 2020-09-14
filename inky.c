#include "includes.h"
#include <math.h>
#include <stdbool.h>
#include "string.h"
#include "stdlib.h"
#include "FieldOne.h"
#include "Demo.h"
#include "buttons.h"
#include "moveLogic.h"

struct myPacMan Inky = { { .x = 100, .y = 100, }, .vx = 0, .vy = 0, .lives = 3,
		.chasing = false, .Speed = 1, .food_collected = 0, .animation = 0,
		.color = Cyan, .startMove = false, .GOSX = 0, .GOSY = 0, .nx = 0, .ny =
				0 };

void inky(int (*food_collected), int *lives, int position_x, int position_y,
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
		if (Inky.startMove == false) {

			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == ' ') {
				Field1[Inky.position.y / 8][Inky.position.x / 8] = 'k';
			}
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == 'M') {
				Field1[Inky.position.y / 8][Inky.position.x / 8] = 'K';
			}

			Inky.vx = 1;
			GhostX = Inky.position.x + Inky.vx;
			GhostY = Inky.position.y + Inky.vy;
			if (*lives == 0) {
				Inky.vx = 0;
				Inky.vy = 0;
			}

			if (GhostX > Inky.position.x
					&& Field1[(GhostY) / 8][(GhostX) / 8] == 'V') {
				switcher = true;
			}
			if (switcher == true) {
				Inky.vx = -1;
				Inky.position.x += Inky.vx;
				gdispFillCircle(Inky.position.x, Inky.position.y, 2,
						Inky.color);
				GhostX = Inky.position.x + Inky.vx;
				GhostY = Inky.position.y + Inky.vy;
				if (GhostX < Inky.position.x
						&& Field1[(GhostY) / 8][(GhostX) / 8] == 'V') {
					switcher = false;
				}
			}

			if (switcher == false) {
				Inky.vx = 1;
				Inky.position.x += Inky.vx;
				gdispFillCircle(Inky.position.x, Inky.position.y, 2,
						Inky.color);
			}

			// Delete and open the door when move the PacMan
			for (int i = 0; i < Field_Height; i++) {
				for (int j = 0; j < Field_Width; j++) {
					if ((*food_collected) >= 1500) {

						DoorOpen = true;
						Inky.startMove = true;
					}
					if (Field1[i][j] == 'D' && DoorOpen == true) {
						Field1[i][j] = 'S';

					}

				}
			}
		}

		// Draw the Red very aggressive Ghost (Inky)
		if (Inky.startMove == true) {

			// move the ghost to out the box and set the manual move
			// set start point x

			if (GhostSwitcher == false) {

				if (Inky.position.x > 108) {
					if (*lives == 0) {
						Inky.vx = 0;
						Inky.vy = 0;
					} else {
						if (Field1[Inky.position.y / 8][Inky.position.x / 8]
								== ' ') {
							Field1[Inky.position.y / 8][Inky.position.x / 8] =
									'k';
						}
						if (Field1[Inky.position.y / 8][Inky.position.x / 8]
								== 'M') {
							Field1[Inky.position.y / 8][Inky.position.x / 8] =
									'K';
						}
						if (Field1[Inky.position.y / 8][Inky.position.x / 8]
								== 'S') {
							gdispFillArea(Inky.position.x - 6,
									Inky.position.y - 6, 12, 12, Black);
						}
						Inky.vx = -1;
						Inky.position.x += Inky.vx;
					}
					gdispFillCircle(Inky.position.x, Inky.position.y, 2,
							Inky.color);
				} else if (Inky.position.x <= 108) {
					if (*lives == 0) {
						Inky.vx = 0;
						Inky.vy = 0;
					} else {
						if (Field1[Inky.position.y / 8][Inky.position.x / 8]
								== ' ') {
							Field1[Inky.position.y / 8][Inky.position.x / 8] =
									'k';
						}
						if (Field1[Inky.position.y / 8][Inky.position.x / 8]
								== 'M') {
							Field1[Inky.position.y / 8][Inky.position.x / 8] =
									'K';
						}
						if (Field1[Inky.position.y / 8][Inky.position.x / 8]
								== 'S') {
							gdispFillArea(Inky.position.x - 6,
									Inky.position.y - 6, 12, 12, Black);
						}
						if (Field1[Inky.position.y / 8][Inky.position.x / 8]
								== '.') {
							Field1[Inky.position.y / 8][Inky.position.x / 8] =
									'C';
						}
						Inky.vx = 1;
						Inky.position.x += Inky.vx;
					}
				}
				gdispFillCircle(Inky.position.x, Inky.position.y, 2,
						Inky.color);
				//set start point y
				if (Inky.position.y >= 83) {
					if (*lives == 0) {
						Inky.vx = 0;
						Inky.vy = 0;
					} else {
						if (Field1[Inky.position.y / 8][Inky.position.x / 8]
								== ' ') {
							Field1[Inky.position.y / 8][Inky.position.x / 8] =
									'k';
						}
						if (Field1[Inky.position.y / 8][Inky.position.x / 8]
								== 'M') {
							Field1[Inky.position.y / 8][Inky.position.x / 8] =
									'K';
						}
						if (Field1[Inky.position.y / 8][Inky.position.x / 8]
								== 'S') {
							gdispFillArea(Inky.position.x - 6,
									Inky.position.y - 6, 12, 12, Black);
						}
						if (Field1[Inky.position.y / 8][Inky.position.x / 8]
								== '.') {
							Field1[Inky.position.y / 8][Inky.position.x / 8] =
									'C';
						}
						Inky.vy = -1;
						Inky.position.y += Inky.vy;
					}
					gdispFillCircle(Inky.position.x, Inky.position.y, 2,
							Inky.color);
				}
				if (Inky.position.y == 82
						&& (Inky.position.x == 109 || Inky.position.x == 108))
					GhostSwitcher = true;

			}

			// Find the random way in the field for the ghost and true out the box

			if (GhostSwitcher == true) {
				//Generate directoins
				// 1 bottom right
				if (position_x > Inky.position.x && position_y > Inky.position.y
						&& Inky.GOSX != -1 && Inky.GOSY != -1) {
					Inky.vx = Inky.Speed;
					Inky.vy = Inky.Speed;

				} else if (position_x < Inky.position.x
						&& position_y < Inky.position.y && Inky.GOSX != 1
						&& Inky.GOSY != 1) { // 2 Top left
					Inky.vx = -Inky.Speed;
					Inky.vy = -Inky.Speed;
				} else if (position_x > Inky.position.x
						&& position_y < Inky.position.y && Inky.GOSX != -1
						&& Inky.GOSY != 1) { // 3 Top right
					Inky.vx = Inky.Speed;
					Inky.vy = -Inky.Speed;
				} else if (position_x < Inky.position.x
						&& position_y > Inky.position.y && Inky.GOSX != 1
						&& Inky.GOSY != -1) { // 4 bottom left
					Inky.vx = -Inky.Speed;
					Inky.vy = Inky.Speed;
				}

				savedGX = Inky.vx;
				savedGY = Inky.vy;

				GhostY = Inky.position.y + Inky.vy;
				if (Field1[(GhostY + (3 * Inky.vy)) / 8][Inky.position.x / 8]
						== 'V'
						|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x)
								/ 8] == '#'
						|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x)
								/ 8] == 'D'
						|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x)
								/ 8] == 'b'
						|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x)
								/ 8] == 'B'
						|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x)
								/ 8] == 'R'
						|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x)
								/ 8] == 'L'
						|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x)
								/ 8] == 'S') {
					Inky.vy = 0;
				}
				GhostX = Inky.position.x + Inky.vx;
				if (Field1[(Inky.position.y) / 8][(GhostX + (3 * Inky.vx)) / 8]
						== 'V'
						|| Field1[(Inky.position.y) / 8][(GhostX + (3 * Inky.vx))
								/ 8] == '#'
						|| Field1[(Inky.position.y) / 8][(GhostX + (3 * Inky.vx))
								/ 8] == 'D'
						|| Field1[(Inky.position.y) / 8][(GhostX + (3 * Inky.vx))
								/ 8] == 'b'
						|| Field1[(Inky.position.y) / 8][(GhostX + (3 * Inky.vx))
								/ 8] == 'B'
						|| Field1[(Inky.position.y) / 8][(GhostX + (3 * Inky.vx))
								/ 8] == 'R'
						|| Field1[(Inky.position.y) / 8][(GhostX + (3 * Inky.vx))
								/ 8] == 'L'
						|| Field1[(Inky.position.y) / 8][(GhostX + (3 * Inky.vx))
								/ 8] == 'S') {
					Inky.vx = 0;
				}

				// Random to the debug in corners in the same side like PacMan and debug the same line positions
				if (Inky.vx == 0 && Inky.vy == 0) {

					if (position_x > Inky.position.x
							&& position_y > Inky.position.y) {
						Inky.vx = -Inky.Speed;
						Inky.vy = Inky.Speed;

					} else if (position_x < Inky.position.x
							&& position_y < Inky.position.y) { // 2 Top left
						Inky.vx = Inky.Speed;
						Inky.vy = -Inky.Speed;
					} else if (position_x > Inky.position.x
							&& position_y < Inky.position.y) { // 3 Top right
						Inky.vx = -Inky.Speed;
						Inky.vy = -Inky.Speed;
					} else if (position_x < Inky.position.x
							&& position_y > Inky.position.y) { // 4 bottom left
						Inky.vx = Inky.Speed;
						Inky.vy = Inky.Speed;
					}
					//			 generate random positions to debug the same line positions
					if ((position_x == Inky.position.x && Inky.vx == 0
							&& Inky.vy == 0)
							|| (position_y == Inky.position.y && Inky.vy == 0
									&& Inky.vx == 0)) {
						GhostcountX = rand() % (4);
						if ((int) GhostcountX == 0
								&& (Inky.GOSX != -1 || Inky.GOSY != -1)) {
							Inky.vx = Inky.Speed;
							Inky.vy = Inky.Speed;

						} else if ((int) GhostcountX == 1
								&& (Inky.GOSX != 1 || Inky.GOSY != 1)) {
							Inky.vx = -Inky.Speed;
							Inky.vy = -Inky.Speed;
						} else if ((int) GhostcountX == 2
								&& (Inky.GOSX != -1 || Inky.GOSY != 1)) {
							Inky.vx = Inky.Speed;
							Inky.vy = -Inky.Speed;

						} else if ((int) GhostcountX == 3
								&& (Inky.GOSX != 1 || Inky.GOSY != -1)) {
							Inky.vx = -Inky.Speed;
							Inky.vy = Inky.Speed;

						}
					}
					GhostY = Inky.position.y + Inky.vy;
					if (Field1[(GhostY + (3 * Inky.vy)) / 8][Inky.position.x / 8]
							== 'V'
							|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x)
									/ 8] == '#'
							|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x)
									/ 8] == 'D'
							|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x)
									/ 8] == 'b'
							|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x)
									/ 8] == 'B'
							|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x)
									/ 8] == 'R'
							|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x)
									/ 8] == 'L'
							|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x)
									/ 8] == 'S') {
						Inky.vy = 0;
					}
					GhostX = Inky.position.x + Inky.vx;
					if (Field1[(Inky.position.y) / 8][(GhostX + (3 * Inky.vx))
							/ 8] == 'V'
							|| Field1[(Inky.position.y) / 8][(GhostX
									+ (3 * Inky.vx)) / 8] == '#'
							|| Field1[(Inky.position.y) / 8][(GhostX
									+ (3 * Inky.vx)) / 8] == 'D'
							|| Field1[(Inky.position.y) / 8][(GhostX
									+ (3 * Inky.vx)) / 8] == 'b'
							|| Field1[(Inky.position.y) / 8][(GhostX
									+ (3 * Inky.vx)) / 8] == 'B'
							|| Field1[(Inky.position.y) / 8][(GhostX
									+ (3 * Inky.vx)) / 8] == 'R'
							|| Field1[(Inky.position.y) / 8][(GhostX
									+ (3 * Inky.vx)) / 8] == 'L'
							|| Field1[(Inky.position.y) / 8][(GhostX
									+ (3 * Inky.vx)) / 8] == 'S') {
						Inky.vx = 0;
					}
					Inky.GOSX = Inky.vx;
					Inky.GOSY = Inky.vy;

				}
				Inky.GOSX = Inky.vx;
				Inky.GOSY = Inky.vy;
				// tunnel right left for the ghost
				if (Field1[Inky.position.y / 8][Inky.position.x / 8] == 'r') {
					gdispFillArea(Inky.position.x - 10, Inky.position.y - 10,
							10, 10, Black);
					Inky.position.x = 210;

				} else if (Field1[Inky.position.y / 8][Inky.position.x / 8]
						== 'l') {
					gdispFillArea(Inky.position.x - 10, Inky.position.y - 10,
							10, 10, Black);
					Inky.position.x = 12;
				}

				if (*lives == 0) {
					Inky.vx = 0;
					Inky.vy = 0;
				}

				if (Field1[Inky.position.y / 8][Inky.position.x / 8] == ' ') {
					Field1[Inky.position.y / 8][Inky.position.x / 8] = 'k';
				}
				if (Field1[Inky.position.y / 8][Inky.position.x / 8] == 'M') {
					Field1[Inky.position.y / 8][Inky.position.x / 8] = 'K';
				}
				if (Field1[Inky.position.y / 8][Inky.position.x / 8] == '.') {
					Field1[Inky.position.y / 8][Inky.position.x / 8] = 'C';
				}

				// Draw the Ghost in the filtered position
				Inky.position.x += Inky.vx;
				Inky.position.y += Inky.vy;
				gdispFillCircle(Inky.position.x, Inky.position.y, 2,
						Inky.color);
			}

			if (Field1[posY / 8][posX / 8] == ':') {
				Field1[posY / 8][posX / 8] = 'J';
				escapeMode = true;
			}

		}
		if ((Inky.position.x + (4 * Inky.vx) == position_x + (4 * posVX)
				&& Inky.position.y + (4 * Inky.vy) == position_y + (4 * posVY))
				|| (Inky.position.x + (8 * Inky.vx) == position_x + (posVX)
						&& Inky.position.y + (8 * Inky.vy)
								== position_y + (posVY))
				|| (Inky.position.x + (1 * Inky.vx) == position_x + (1 * posVX)
						&& Inky.position.y + (1 * Inky.vy)
								== position_y + (1 * posVY))) {
			gdispFillArea(position_x - 6, position_y - 6, 13, 13, Black);
			if (*lives >= 0) {
				Inky.startMove = false;
				GhostSwitcher = false;
				DoorOpen = false;
				Inky.position.x = 110;
				Inky.position.y = 100;
				(*lives)--;
				(*chasing) = true;
			}

		}
		if (*lives == 0 || StartMOVE == true) {
			Inky.startMove = false;
			GhostSwitcher = false;
			DoorOpen = false;
			Inky.position.x = 110;
			Inky.position.y = 100;
		}

	} else if (escapeMode == true) { // make the Ghosts escape mode. move away from Pac man
		if (Gchasing == false) {

			// Find the random way in the field for the ghost and true out the box

			//Generate directoins
			// 1 bottom right
			if (position_x > Inky.position.x && position_y > Inky.position.y
					&& Inky.GOSX != -1 && Inky.GOSY != -1) {
				Inky.vx = Inky.Speed;
				Inky.vy = Inky.Speed;

			} else if (position_x < Inky.position.x
					&& position_y < Inky.position.y && Inky.GOSX != 1
					&& Inky.GOSY != 1) { // 2 Top left
				Inky.vx = -Inky.Speed;
				Inky.vy = -Inky.Speed;
			} else if (position_x > Inky.position.x
					&& position_y < Inky.position.y && Inky.GOSX != -1
					&& Inky.GOSY != 1) { // 3 Top right
				Inky.vx = Inky.Speed;
				Inky.vy = -Inky.Speed;
			} else if (position_x < Inky.position.x
					&& position_y > Inky.position.y && Inky.GOSX != 1
					&& Inky.GOSY != -1) { // 4 bottom left
				Inky.vx = -Inky.Speed;
				Inky.vy = Inky.Speed;
			}

			savedGX = Inky.vx;
			savedGY = Inky.vy;

			GhostY = Inky.position.y + Inky.vy;
			if (Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
					+ (3 * Inky.vx)) / 8] == 'V'
					|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
							+ (3 * Inky.vx)) / 8] == '#'
					|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
							+ (3 * Inky.vx)) / 8] == 'D'
					|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
							+ (3 * Inky.vx)) / 8] == 'b'
					|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
							+ (3 * Inky.vx)) / 8] == 'B'
					|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
							+ (3 * Inky.vx)) / 8] == 'R'
					|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
							+ (3 * Inky.vx)) / 8] == 'L'
					|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
							+ (3 * Inky.vx)) / 8] == 'S') {
				Inky.vy = 0;
			}
			GhostX = Inky.position.x + Inky.vx;
			if (Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
					+ (3 * Inky.vx)) / 8] == 'V'
					|| Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
							+ (3 * Inky.vx)) / 8] == '#'
					|| Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
							+ (3 * Inky.vx)) / 8] == 'D'
					|| Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
							+ (3 * Inky.vx)) / 8] == 'b'
					|| Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
							+ (3 * Inky.vx)) / 8] == 'B'
					|| Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
							+ (3 * Inky.vx)) / 8] == 'R'
					|| Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
							+ (3 * Inky.vx)) / 8] == 'L'
					|| Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
							+ (3 * Inky.vx)) / 8] == 'S') {
				Inky.vx = 0;
			}

			// Random to the debug in corners in the same side like PacMan and debug the same line positions
			if (Inky.vx == 0 && Inky.vy == 0) {

				if (position_x > Inky.position.x
						&& position_y > Inky.position.y) {
					Inky.vx = -Inky.Speed;
					Inky.vy = Inky.Speed;

				} else if (position_x < Inky.position.x
						&& position_y < Inky.position.y) { // 2 Top left
					Inky.vx = Inky.Speed;
					Inky.vy = -Inky.Speed;
				} else if (position_x > Inky.position.x
						&& position_y < Inky.position.y) { // 3 Top right
					Inky.vx = -Inky.Speed;
					Inky.vy = -Inky.Speed;
				} else if (position_x < Inky.position.x
						&& position_y > Inky.position.y) { // 4 bottom left
					Inky.vx = Inky.Speed;
					Inky.vy = Inky.Speed;
				}
				//			 generate random positions to debug the same line positions
				if ((position_x == Inky.position.x && Inky.vx == 0
						&& Inky.vy == 0)
						|| (position_y == Inky.position.y && Inky.vy == 0
								&& Inky.vx == 0)) {
					GhostcountX = rand() % (4);
					if ((int) GhostcountX == 0
							&& (Inky.GOSX != -1 || Inky.GOSY != -1)) {
						Inky.vx = Inky.Speed;
						Inky.vy = Inky.Speed;

					} else if ((int) GhostcountX == 1
							&& (Inky.GOSX != 1 || Inky.GOSY != 1)) {
						Inky.vx = -Inky.Speed;
						Inky.vy = -Inky.Speed;
					} else if ((int) GhostcountX == 2
							&& (Inky.GOSX != -1 || Inky.GOSY != 1)) {
						Inky.vx = Inky.Speed;
						Inky.vy = -Inky.Speed;

					} else if ((int) GhostcountX == 3
							&& (Inky.GOSX != 1 || Inky.GOSY != -1)) {
						Inky.vx = -Inky.Speed;
						Inky.vy = Inky.Speed;

					}
				}
				GhostY = Inky.position.y + Inky.vy;
				if (Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
						+ (3 * Inky.vx)) / 8] == 'V'
						|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
								+ (3 * Inky.vx)) / 8] == '#'
						|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
								+ (3 * Inky.vx)) / 8] == 'D'
						|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
								+ (3 * Inky.vx)) / 8] == 'b'
						|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
								+ (3 * Inky.vx)) / 8] == 'B'
						|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
								+ (3 * Inky.vx)) / 8] == 'R'
						|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
								+ (3 * Inky.vx)) / 8] == 'L'
						|| Field1[(GhostY + (3 * Inky.vy)) / 8][(Inky.position.x
								+ (3 * Inky.vx)) / 8] == 'S') {
					Inky.vy = 0;
				}
				GhostX = Inky.position.x + Inky.vx;
				if (Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
						+ (3 * Inky.vx)) / 8] == 'V'
						|| Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
								+ (3 * Inky.vx)) / 8] == '#'
						|| Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
								+ (3 * Inky.vx)) / 8] == 'D'
						|| Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
								+ (3 * Inky.vx)) / 8] == 'b'
						|| Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
								+ (3 * Inky.vx)) / 8] == 'B'
						|| Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
								+ (3 * Inky.vx)) / 8] == 'R'
						|| Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
								+ (3 * Inky.vx)) / 8] == 'L'
						|| Field1[(Inky.position.y + (3 * Inky.vy)) / 8][(GhostX
								+ (3 * Inky.vx)) / 8] == 'S') {
					Inky.vx = 0;
				}
				Inky.GOSX = Inky.vx;
				Inky.GOSY = Inky.vy;

			}
			Inky.GOSX = Inky.vx;
			Inky.GOSY = Inky.vy;
			// tunnel right left for the ghost
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == 'r') {
				Inky.position.x = 210;

			} else if (Field1[Inky.position.y / 8][Inky.position.x / 8]
					== 'l') {
				Inky.position.x = 12;
			}

			if (*lives == 0) {
				Inky.vx = 0;
				Inky.vy = 0;
			}

			// Draw the Ghost in the filtered position
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == ' ') {
				Field1[Inky.position.y / 8][Inky.position.x / 8] = 'k';
			}
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == 'M') {
				Field1[Inky.position.y / 8][Inky.position.x / 8] = 'K';
			}
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == '.') {
				Field1[Inky.position.y / 8][Inky.position.x / 8] = 'C';
			}

			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == '!') {
				gdispFillArea(Inky.position.x - 10, Inky.position.y - 10, 12,
						12, Black);
			}
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == 'J') {
				gdispFillArea(Inky.position.x - 10, Inky.position.y - 10, 12,
						12, Black);
			}
			Inky.position.x += Inky.vx;
			Inky.position.y += Inky.vy;
			gdispFillCircle(Inky.position.x, Inky.position.y, 2, Green);

			if ((Inky.position.x + (4 * Inky.vx) == position_x + (4 * posVX)
					&& Inky.position.y + (4 * Inky.vy)
							== position_y + (4 * posVY))
					|| (Inky.position.x + (8 * Inky.vx) == position_x + (posVX)
							&& Inky.position.y + (8 * Inky.vy)
									== position_y + (posVY))
					|| (Inky.position.x + (1 * Inky.vx)
							== position_x + (1 * posVX)
							&& Inky.position.y + (1 * Inky.vy)
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

			if (GstartX > Inky.position.x && GstartY > Inky.position.y
					&& Inky.GOSX != -1 && Inky.GOSY != -1) {
				Inky.vx = Inky.Speed;
				Inky.vy = Inky.Speed;

			} else if (GstartX < Inky.position.x && GstartY < Inky.position.y
					&& Inky.GOSX != 1 && Inky.GOSY != 1) { // 2 Top left
				Inky.vx = -Inky.Speed;
				Inky.vy = -Inky.Speed;

			} else if (GstartX > Inky.position.x && GstartY < Inky.position.y
					&& Inky.GOSX != -1 && Inky.GOSY != 1) { // 3 Top right
				Inky.vx = Inky.Speed;
				Inky.vy = -Inky.Speed;
				z = 0;
			} else if (GstartX < Inky.position.x && GstartY > Inky.position.y
					&& Inky.GOSX != 1 && Inky.GOSY != -1) { // 4 bottom left
				Inky.vx = -Inky.Speed;
				Inky.vy = Inky.Speed;

			} else {
				Inky.vx = 0;
				Inky.vy = 0;
			}

			savedGX = Inky.vx;
			savedGY = Inky.vy;

			GhostY = Inky.position.y + Inky.vy;
			if (Field1[(GhostY + (3 * Inky.vy)) / 8][Inky.position.x / 8] == 'V'
					|| Field1[(GhostY + (4 * Inky.vy)) / 8][Inky.position.x / 8]
							== '#'
					|| Field1[(GhostY + (4 * Inky.vy)) / 8][Inky.position.x / 8]
							== 'D'
					|| Field1[(GhostY + (4 * Inky.vy)) / 8][Inky.position.x / 8]
							== 'b'
					|| Field1[(GhostY + (4 * Inky.vy)) / 8][Inky.position.x / 8]
							== 'B'
					|| Field1[(GhostY + (4 * Inky.vy)) / 8][Inky.position.x / 8]
							== 'R'
					|| Field1[(GhostY + (4 * Inky.vy)) / 8][Inky.position.x / 8]
							== 'L'
					|| Field1[(GhostY + (4 * Inky.vy)) / 8][Inky.position.x / 8]
							== 'S') {
				Inky.vy = 0;
			}
			GhostX = Inky.position.x + Inky.vx;
			if (Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8] == 'V'
					|| Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8]
							== '#'
					|| Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8]
							== 'D'
					|| Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8]
							== 'b'
					|| Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8]
							== 'B'
					|| Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8]
							== 'R'
					|| Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8]
							== 'L'
					|| Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8]
							== 'S') {
				Inky.vx = 0;
			}
			// make random movement to debug in position
			if (Inky.vx == 0 && Inky.vy == 0 && Garrived == false) {

				if (GstartX > Inky.position.x && GstartY > Inky.position.y) {
					Inky.vx = -Inky.Speed;
					Inky.vy = Inky.Speed;

				} else if (GstartX < Inky.position.x
						&& GstartY < Inky.position.y) { // 2 Top left
					Inky.vx = Inky.Speed;
					Inky.vy = -Inky.Speed;
				} else if (GstartX > Inky.position.x
						&& GstartY < Inky.position.y) { // 3 Top right
					Inky.vx = -Inky.Speed;
					Inky.vy = -Inky.Speed;
				} else if (GstartX < Inky.position.x
						&& GstartY > Inky.position.y) { // 4 bottom left
					Inky.vx = Inky.Speed;
					Inky.vy = Inky.Speed;
				}

				// debug the same y move direction
				if (GstartX < Inky.position.x && GstartY < Inky.position.y
						&& Inky.GOSX != 0 && Inky.GOSY == 1) {
					Inky.vx = -Inky.Speed;
					Inky.vy = Inky.Speed;

				}

				if (GstartX > Inky.position.x && GstartY < Inky.position.y
						&& Inky.GOSX != 0 && Inky.GOSY == 1) {
					Inky.vx = Inky.Speed;
					Inky.vy = Inky.Speed;

				}

				// control the other conditions
				z++;
				Inky.GOSX = Inky.vx;
				Inky.GOSY = Inky.vy;
			}
			//			 generate random positions to debug the same line positions
			if ((GstartX == Inky.position.x) || (GstartY == Inky.position.y)) {
				GhostcountX = rand() % (4);
				if ((int) GhostcountX == 0
						&& (Inky.GOSX != -1 || Inky.GOSY != -1)) {
					Inky.vx = Inky.Speed;
					Inky.vy = Inky.Speed;

				} else if ((int) GhostcountX == 1
						&& (Inky.GOSX != 1 || Inky.GOSY != 1)) {
					Inky.vx = -Inky.Speed;
					Inky.vy = -Inky.Speed;
				} else if ((int) GhostcountX == 2
						&& (Inky.GOSX != -1 || Inky.GOSY != 1)) {
					Inky.vx = Inky.Speed;
					Inky.vy = -Inky.Speed;

				} else if ((int) GhostcountX == 3
						&& (Inky.GOSX != 1 || Inky.GOSY != -1)) {
					Inky.vx = -Inky.Speed;
					Inky.vy = Inky.Speed;

				}
				z++;
				Inky.GOSX = Inky.vx;
				Inky.GOSY = Inky.vy;
			}
			if (Field1[(GhostY + (3 * Inky.vy)) / 8][Inky.position.x / 8] == 'V'
					|| Field1[(GhostY + (4 * Inky.vy)) / 8][Inky.position.x / 8]
							== '#'
					|| Field1[(GhostY + (4 * Inky.vy)) / 8][Inky.position.x / 8]
							== 'D'
					|| Field1[(GhostY + (4 * Inky.vy)) / 8][Inky.position.x / 8]
							== 'b'
					|| Field1[(GhostY + (4 * Inky.vy)) / 8][Inky.position.x / 8]
							== 'B'
					|| Field1[(GhostY + (4 * Inky.vy)) / 8][Inky.position.x / 8]
							== 'R'
					|| Field1[(GhostY + (4 * Inky.vy)) / 8][Inky.position.x / 8]
							== 'L'
					|| Field1[(GhostY + (4 * Inky.vy)) / 8][Inky.position.x / 8]
							== 'S') {
				Inky.vy = 0;
			}
			GhostX = Inky.position.x + Inky.vx;
			if (Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8] == 'V'
					|| Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8]
							== '#'
					|| Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8]
							== 'D'
					|| Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8]
							== 'b'
					|| Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8]
							== 'B'
					|| Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8]
							== 'R'
					|| Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8]
							== 'L'
					|| Field1[Inky.position.y / 8][(GhostX + (4 * Inky.vx)) / 8]
							== 'S') {
				Inky.vx = 0;
			}

			diffGhostX = Inky.position.x - GstartX;
			diffGhostY = Inky.position.y - GstartY;
			Inky.nx = Inky.position.x + Inky.vx;
			Inky.ny = Inky.position.y + Inky.vy;
			int m = abs(diffGhostX);
			int n = abs(diffGhostY);
			int nextX = Inky.nx - GstartX;
			int nextY = Inky.ny - GstartY;
			int a = abs(nextX);
			int b = abs(nextY);
			if (z >= 50) {

				if (m < a) {
					Inky.vx = 0;
				}
				if (n < b) {
					Inky.vy = 0;
				}

				Inky.GOSX = Inky.vx;
				Inky.GOSY = Inky.vy;
				z = 0;

			}

			if (b < n && a > m && Inky.vy != 0) {
				Inky.vx = 0;

			}

			if (Inky.vx != 0 && Inky.vy != 0) {
				if (b > a) {
					Inky.vx = 0;
				}
				if (a > b) {
					Inky.vy = 0;
				}

			}

			if ((Inky.position.x + (1 * Inky.vx) == GstartX
					&& Inky.position.y + (1 * Inky.vy) == GstartY)) {
				Garrived = true;
			}
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == ' ') {
				Field1[Inky.position.y / 8][Inky.position.x / 8] = 'k';
			}
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == 'M') {
				Field1[Inky.position.y / 8][Inky.position.x / 8] = 'K';
			}
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == '.') {
				Field1[Inky.position.y / 8][Inky.position.x / 8] = 'C';
			}
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == '!') {
				gdispFillArea(Inky.position.x - 10, Inky.position.y - 10, 12,
						12, Black);
			}
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == 'J') {
				gdispFillArea(Inky.position.x - 10, Inky.position.y - 10, 12,
						12, Black);
			}
			Inky.position.x += Inky.vx;
			Inky.position.y += Inky.vy;
			gdispFillCircle(Inky.position.x, Inky.position.y, 2, Red);

		}

		if (Garrived == true) {

			Inky.vx = 1;
			Inky.vy = 1;
			Inky.position.x += Inky.vx;
			Inky.position.y += Inky.vy;
			if (Inky.position.x > 120) {
				Inky.vx = 0;
			}
			if (Inky.position.y > 110) {
				Inky.vy = 0;
			}

			if (Inky.position.x >= 120 && Inky.position.y >= 110) {
				escapeMode = false;
				Gchasing = false;
				Garrived = false;
				GhostSwitcher = false;
				Inky.vx = 0;
				Inky.vy = 0;
			}
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == ' ') {
				Field1[Inky.position.y / 8][Inky.position.x / 8] = 'k';
			}
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == 'M') {
				Field1[Inky.position.y / 8][Inky.position.x / 8] = 'K';
			}
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == '.') {
				Field1[Inky.position.y / 8][Inky.position.x / 8] = 'C';
			}
			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == '!') {
				gdispFillArea(Inky.position.x - 10, Inky.position.y - 10, 12,
						12, Black);
			}

			if (Field1[Inky.position.y / 8][Inky.position.x / 8] == 'J') {
				gdispFillArea(Inky.position.x - 10, Inky.position.y - 10, 12,
						12, Black);
			}
			Inky.position.x += Inky.vx;
			Inky.position.y += Inky.vy;
			gdispFillCircle(Inky.position.x, Inky.position.y, 2, Blue);
		}

	}

}
