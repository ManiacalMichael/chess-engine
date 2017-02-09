#include <stdint.h>
#include "chess.h"


const uint64_t file_masks[8] = {
	0x0101010101010101ull,
	0x0202020202020202ull,
	0x0404040404040404ull,
	0x0808080808080808ull,
	0x1010101010101010ull,
	0x2020202020202020ull,
	0x4040404040404040ull,
	0x8080808080808080ull
};

const uint64_t rank_masks[8] = {
	0x00000000000000ffull,
	0x000000000000ff00ull,
	0x0000000000ff0000ull,
	0x00000000FF000000ull,
	0x000000FF00000000ull,
	0x0000ff0000000000ull,
	0x00ff000000000000ull,
	0xff00000000000000ull
};

const uint64_t diagonal_masks[15] = {
	0x0000000000000080ull,
	0x0000000000008040ull,
	0x0000000000804020ull,
	0x0000000080402010ull,
	0x0000008040201008ull,
	0x0000804020100804ull,
	0x0080402010080402ull,
	0x8040201008040201ull,
	0x4020100804020100ull,
	0x2010080402010000ull,
	0x1008040201000000ull,
	0x0804020100000000ull,
	0x0402010000000000ull,
	0x0201000000000000ull,
	0x0100000000000000ull
};

const uint64_t antidiagonal_masks[15] = {
	0x0000000000000001ull,
	0x0000000000000102ull,
	0x0000000000010204ull,
	0x0000000001020408ull,
	0x0000000102040810ull,
	0x0000010204081020ull,
	0x0001020408102040ull,
	0x0102040810204080ull,
	0x0204081020408000ull,
	0x0408102040800000ull,
	0x0810204080000000ull,
	0x1020408000000000ull,
	0x2040800000000000ull,
	0x4080000000000000ull,
	0x8000000000000000ull
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

uint64_t pawn_moves(uint64_t enemy, uint64_t empty, int color, int sq)
{
	uint64_t r = 0ull;
	if (color) {
		/* lookup tables are written for white pawns, so shift */
		if ((sq / 8) == 6) {
			if (!((pawn_twosquare[sq % 8] << 16) & ~empty))
				r |= pawn_movement[sq - 24];
		}
		r |= (pawn_movement[sq] >> 16) & empty;
		r |= (pawn_captures[sq] >> 16) & enemy;
	} else {
		if ((sq / 8) == 1) {
			if (!(pawn_twosquare[sq % 8] & ~empty))
				r |= pawn_movement[sq + 8];
		}
		r |= pawn_movement[sq] & empty;
		r |= pawn_captures[sq] & enemy;
	}
	return r;
}

uint64_t bishop_moves(uint64_t occupied, int rank, int file)
{
	uint64_t r = 0ull;
	uint64_t diagonal = diagonal_masks[(7 + rank) - file];
	uint64_t antidiagonal = antidiagonal_masks[rank + file];
	/* 
	 * Actually, shift down by 56 gives the key, but the key is then
	 * multiplied by 8, which is the same as shifting back up by 3, and the
	 * key is shifted down by one to ignore the occupancy of the edge square
	 */
	uint64_t key = (((diagonal & occupied) * FILL_MULTIPLIER) >> 54) & OUTER_SQ_MASK;
	r |= (sliding_attack_lookups[key + file] * FILL_MULTIPLIER) & diagonal;
	key = (((antidiagonal & occupied) * FILL_MULTIPLIER) >> 54) & OUTER_SQ_MASK;
	r |= (sliding_attack_lookups[key + file] * FILL_MULTIPLIER) & antidiagonal;
	return r;
}

uint64_t rook_moves(uint64_t occupied, int rank, int file)
{
	uint64_t r = 0ull;
	uint64_t key = (occupied & rank_masks[rank]) >> (8 * rank);
	key <<= 2;
	key &= OUTER_SQ_MASK;
	/* processor will do 8-bit shift if no cast */
	r |= (uint64_t)sliding_attack_lookups[key + file] << (8 * rank);
	key = ((occupied & file_masks[file]) >> file) * MAIN_ANTIDIAGONAL;
	key >>= 54;
	key &= OUTER_SQ_MASK;
	/* but does 64-bit multiply here because of the 64-bit immediate */
	key = sliding_attack_lookups[key + rank] * MAIN_ANTIDIAGONAL;
	r |= (key >> (7 - file)) & file_masks[file];
	return r;
}

uint64_t queen_moves(uint64_t occupied, int rank, int file)
{
	uint64_t r = 0ull;
	r |= bishop_moves(occupied, rank, file);
	r |= rook_moves(occupied, rank, file);
	return r;
}

uint16_t check_status(struct board_t *boardPtr)
{
	int bking, wking, brank, bfile, wrank, wfile;
	uint64_t occupied = boardPtr->occupied;
	uint64_t blackp = boardPtr->black_pieces;
	uint64_t whitep = ~blackp & boardPtr->occupied;
	uint16_t ret = 0;
	bking = bitindice(blackp & boardPtr->kings);
	wking = bitindice(boardPtr->kings ^ (1ull << bking));
	brank = bking / 8;
	bfile = bking % 8;
	wrank = wking / 8;
	wfile = wking % 8;
	if (rook_moves(occupied, wrank, wfile) & ((blackp & boardPtr->rooks) | (
				blackp & boardPtr->queens))) {
		ret |= WHITE_CHECK;
		goto test_black;
	} else if (bishop_moves(occupied, wrank, wfile) & ((blackp & 
				boardPtr->bishops) | (blackp & boardPtr->queens))) {
		ret |= WHITE_CHECK;
		goto test_black;
	} else if (knight_attack_lookups[wking] & (blackp & boardPtr->knights)) {
		ret |= WHITE_CHECK;
		goto test_black;
	} else if (pawn_captures[wking] & (blackp & boardPtr->pawns)) {
		ret |= WHITE_CHECK;
		goto test_black;
	} else if (king_attack_lookups[wking] & (blackp & boardPtr->kings)) {
		ret |= WHITE_CHECK;
	}
	test_black:
	if (rook_moves(occupied, brank, bfile) & ((whitep & boardPtr->rooks) |
			(whitep & boardPtr->queens))) {
		ret |= BLACK_CHECK;
		return ret;
	} else if (bishop_moves(occupied, brank, bfile) & ((whitep & 
				boardPtr->bishops) | (whitep & boardPtr->queens))) {
		ret |= BLACK_CHECK;
		return ret;
	} else if (knight_attack_lookups[bking] & (whitep & boardPtr->knights)) {
		ret |= BLACK_CHECK;
		return ret;
	} else if (pawn_captures[bking] & (whitep & boardPtr->pawns)) {
		ret |= BLACK_CHECK;
		return ret;
	} else if (king_attack_lookups[bking] & (whitep & boardPtr->kings)) {
		ret |= BLACK_CHECK;
	}
	return ret;
}

void make_move(struct position_t *posPtr, uint16_t mv)
{
}

void unmake_move(struct position_t *posPtr, uint16_t mv)
{
}

