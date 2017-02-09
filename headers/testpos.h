#ifndef TESTPOS_H_INCLUDED
#define TESTPOS_H_INCLUDED

#include <stdint.h>
#include "chess.h"

/*
 * Start position perft values:
 *            Moves   Captures  EP  Castles  Promo
 * Perft(0) 1
 * Perft(1) 20 
 * Perft(2) 400 
 * Perft(3) 8902      34
 * Perft(4) 197281    1576
 * Perft(5) 4865609   82719     258
 * Perft(6) 119060324 2812008   5248
 */

/*
 * Perft values for 'kiwipete'
 *          Moves      Captures    EP    Castles  Promo
 * Perft(1) 48         8                 2
 * Perft(2) 2039       351         1     91
 * Perft(3) 97862      17102       45    3162
 * Perft(4) 4085603    757163      1929  128013   15172
 * Perft(5) 193690690  35043416    73365 4993637  
 */
static const struct position_t perft1 = {
	{
		0x917d731812a4ff91ull,
		0x917d730002800000ull,
		0x1000000000000010ull,
		0x0010000000200000ull,
		0x8100000000000081ull,
		0x0040010000001800ull,
		0x0000221000040000ull,
		0x002d50081280e700ull,
		0x0000000000000000ull,
		0x0000000000000000ull
	},
	0x8780u,
	0,
	0
};

#endif
