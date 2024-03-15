/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * 3D "snake" program for ATmega16-driven 5x5x5 LED cube.
 * Lubomir Rintel <lkundrak@v3.sk>
 *
 * This program is free software.
 * Repository: <https://github.com/hackerspace/led5x5x5>
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <avr/io.h>

#define F_CPU 1000000UL
#include <util/delay.h>

static inline uint8_t
bitfield (uint8_t a, uint8_t b, uint8_t c, uint8_t d,
	uint8_t e, uint8_t f, uint8_t g, uint8_t h)
{
	return	 (a ? 0x01 : 0) |
		 (b ? 0x02 : 0) |
		 (c ? 0x04 : 0) |
		 (d ? 0x08 : 0) |
		 (e ? 0x10 : 0) |
		 (f ? 0x20 : 0) |
		 (g ? 0x40 : 0) |
		 (h ? 0x80 : 0);
}

/*
 * Field buffer. Non-zero number is the "age" of the position that decays
 * towards zero on every update.
 */
static uint8_t field[5][5][5] = { 0, };

/*
 * Draw one row from the field buffer.
 */
static inline void
zrow (int z)
{
	PORTA = bitfield (
		field[0][3][z],	field[0][1][z],	field[0][4][z],	field[1][3][z],
		field[1][2][z],	field[1][1][z],	field[1][0][z],	field[2][0][z]);

	PORTB = bitfield (
		field[1][4][z],	field[0][2][z],	field[0][0][z],	z == 1,
		z == 4,		z == 0,		0,		0);

	PORTC = bitfield (
		field[4][4][z],	field[3][4][z],	field[3][2][z],	field[3][0][z],
		field[2][4][z],	field[2][3][z],	field[2][2][z],	field[2][1][z]);

	PORTD = bitfield (
		z == 3,		z == 2,		field[3][1][z],	field[3][3][z],
		field[4][2][z],	field[4][1][z],	field[4][0][z],	field[4][3][z]);

	_delay_ms (3);
	PORTA = PORTB = PORTC = PORTD = 0;
}

int
main (int argc, char *argv[])
{
	struct { uint8_t x, y, z; } tgt = { 0, }, cur = { 0, };
	uint8_t x, y, z;
	uint8_t len = 0;
	uint8_t mcucsr = MCUCSR;

	/*
	 * Nuke JTAG, so that we can use its ports.
	 * Hardware requires that this register is wrote twice.
	 */
	mcucsr |= 0x80;
	MCUCSR = mcucsr;
	MCUCSR = mcucsr;

	/*
	 * Pin directions.
	 */
	DDRA = 0xff;
	DDRB = 0xff;
	DDRC = 0xff;
	DDRD = 0xff;

	while (1) {
		/*
		 * Age all tiles.
		 */
		for (x = 0; x < 5; x++) {
			for (y = 0; y < 5; y++) {
				for (z = 0; z < 5; z++) {
					if (field[x][y][z] > 0)
						field[x][y][z]--;
				}
			}
		}

		/*
		 * Add a new one at head.
		 */
		field[cur.x][cur.y][cur.z] = len;

		/*
		 * Successfully hit the target?
		 */
		if (tgt.x == cur.x && tgt.y == cur.y && tgt.z == cur.z) {
			len++;
			do {
				tgt.x = rand () % 5;
				tgt.y = rand () % 5;
				tgt.z = rand () % 5;
			} while (field[tgt.x][tgt.y][tgt.z] != 0);

		/*
		 * Can move in a good direction?
		 */
		} else if (tgt.x > cur.x && field[cur.x + 1][cur.y][cur.z] == 0) {
			cur.x++;
		} else if (tgt.x < cur.x && field[cur.x - 1][cur.y][cur.z] == 0) {
			cur.x--;
		} else if (tgt.y > cur.y && field[cur.x][cur.y + 1][cur.z] == 0) {
			cur.y++;
		} else if (tgt.y < cur.y && field[cur.x][cur.y - 1][cur.z] == 0) {
			cur.y--;
		} else if (tgt.z > cur.z && field[cur.x][cur.y][cur.z + 1] == 0) {
			cur.z++;
		} else if (tgt.z < cur.z && field[cur.x][cur.y][cur.z - 1] == 0) {
			cur.z--;

		/*
		 * Can move anywhere?
		 */
		} else if (cur.x < 4 && field[cur.x + 1][cur.y][cur.z] == 0) {
			cur.x++;
		} else if (cur.x > 0 && field[cur.x - 1][cur.y][cur.z] == 0) {
			cur.x--;
		} else if (cur.y < 4 && field[cur.x][cur.y + 1][cur.z] == 0) {
			cur.y++;
		} else if (cur.y > 0 && field[cur.x][cur.y - 1][cur.z] == 0) {
			cur.y--;
		} else if (cur.z < 4 && field[cur.x][cur.y][cur.z + 1] == 0) {
			cur.z++;
		} else if (cur.z > 0 && field[cur.x][cur.y][cur.z - 1] == 0) {
			cur.z--;

		/*
		 * Stuck?
		 */
		} else {
			cur.x = rand () % 5;
			cur.y = rand () % 5;
			cur.z = rand () % 5;

			len = 1;
			for (x = 0; x < 5; x++) {
				for (y = 0; y < 5; y++) {
					for (z = 0; z < 5; z++) {
						field[x][y][z] = 0;
					}
				}
			}
		}

		/*
		 * Show target.
		 */
		field[tgt.x][tgt.y][tgt.z] = 1;

		/*
		 * Output (interlaced)
		 */
		for (x = 0; x < 10; x++) {
			zrow (0);
			zrow (2);
			zrow (4);
			zrow (1);
			zrow (3);
		}
	}
}
