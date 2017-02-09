#include <stdint.h>
#include "chess.h"

static const struct position_t START_POSITION = { 
	.black_pieces = {
		0xffff000000000000ull,
		0x00ff000000000000ull,
		0x4200000000000000ull,
		0x2400000000000000ull,
		0x8100000000000000ull,
		0x0800000000000000ull,
		0x1000000000000000ull
	},			
	.white_pieces = {
		0x000000000000ffffull,
		0x000000000000ff00ull,
		0x0000000000000042ull,
		0x0000000000000024ull,
		0x0000000000000081ull,
		0x0000000000000008ull,
		0x0000000000000001ull
	},
	.occupied = 0xffff00000000ffffull,
	.empty = 0x0000ffffffff0000ull,
	.captures = { 0 };
	.kingpos = { 4, 60 };
	.flags = 0x8780u,	
	.moves = 0,	
	.fiftymove = 0
};


int popcount(uint64_t bb)
{
	int x = 0;
	while (bb != 0ull) {
		bb &= bb - 1;
		x++;
	}
	return x;
}

int ls1bindice(uint64_t bb)
{
	static const int arr[67] = {
		-1, 0, 1, 39, 2, 15, 40, 23, 3, 12, 16, 59, 41, 19, 24,
		54, 4, -1, 13, 10, 17, 62, 60, 28, 42, 30, 20, 51, 25,
		44, 55, 47, 5, 32, -1, 38, 14, 22, 11, 58, 18, 53, 63,
		9, 61, 27, 29, 50, 43, 46, 31, 37, 21, 57, 52, 8, 26,
		49, 45, 36, 56, 7, 48, 35, 6, 34, 33
	};
	bb ^= (bb & (bb - 1))
	return arr[bb % 67];
}

void make_move(struct position_t *posPtr, uint16_t mv)
{
}

void unmake_move(struct position_t *posPtr, uint16_t mv)
{
}

