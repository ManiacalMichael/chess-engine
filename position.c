#include <stdint.h>
#include <limits.h>
#include "headers/chess.h"

const uint64_t file_masks[8] = {
	0x0101010101010101,
	0x0202020202020202,
	0x0404040404040404,
	0x0808080808080808,
	0x1010101010101010,
	0x2020202020202020,
	0x4040404040404040,
	0x8080808080808080
};

const uint64_t rank_masks[8] = {
	0x00000000000000ff,
	0x000000000000ff00,
	0x0000000000ff0000,
	0x00000000ff000000,
	0x000000ff00000000,
	0x0000ff0000000000,
	0x00ff000000000000,
	0xff00000000000000
};

const position_t START_POSITION = { 
	.bboards = {
		/* WHITE */ 0x000000000000ffff,
		/* BLACK */ 0xffff000000000000,
		/* WHITE_PAWN */ 0x000000000000ff00,
		/* BLACK_PAWN */ 0x00ff000000000000,
		/* WHITE_KNIGHT */ 0x0000000000000042,
		/* BLACK_KNIGHT */ 0x4200000000000000,
		/* WHITE_BISHOP */ 0x0000000000000024,
		/* BLACK_BISHOP */ 0x2400000000000000,
		/* WHITE_ROOK */ 0x0000000000000081,
		/* BLACK_ROOK */ 0x8100000000000000,
		/* WHITE_QUEEN */ 0x0000000000000008,
		/* BLACK_QUEEN */ 0x0800000000000000,
		/* WHTIE_KING */ 0x0000000000000010,
		/* BLACK_KING */ 0x1000000000000000,
		/* OCCUPIED */ 0xffff00000000ffff,
		/* EMPTY */ 0x0000ffffffff0000
	},
	.capturestack = { 0 },
	.capturetop = INT_MAX,
	.epstack = { { 0 }, { 0 } },
	.eptop = INT_MAX,
	.castles = { 0 },
	.fiftymovestack = { 0 },
	.fiftymovetop = INT_MAX,
	.flags = 0x8780u,
	.halfmove = 0,
	.fiftymove = 0
};


int popcount(uint64_t bb)
{
	int x = 0;
	for (x = 0; bb; ++x, bb &= bb - 1);
	return x;
}

const int index64[64] = {
	0,  1, 48,  2, 57, 49, 28,  3,
	61, 58, 50, 42, 38, 29, 17,  4,
	62, 55, 59, 36, 53, 51, 43, 22,
	45, 39, 33, 30, 24, 18, 12,  5,
	63, 47, 56, 27, 60, 41, 37, 16,
	54, 35, 52, 21, 44, 32, 23, 11,
	46, 26, 40, 15, 34, 20, 31, 10,
	25, 14, 19,  9, 13,  8,  7,  6
};

int popfirst(uint64_t *bb)
{
	static const uint64_t mul = 0x03f79d71b4cb0a89;
	uint64_t b = (*bb ^ (*bb - 1)) & *bb;
	*bb &= (*bb - 1);
	return index64[(b * mul) >> 58];
}

int ls1bindice(uint64_t bb)
{
	static const uint64_t mul = 0x03f79d71b4cb0a89;
	bb &= bb ^ (bb - 1);
	return index64[(bb * mul) >> 58];
}


void pushcapture(position_t* posPtr, enum PIECES capture)
{
	if (ISEMPTY(posPtr->capturetop))
		posPtr->capturetop = 0;
	else
		++posPtr->capturetop;
	posPtr->capturestack[posPtr->capturetop] = capture;
}

enum PIECES popcapture(position_t* posPtr)
{
	enum PIECES r = posPtr->capturestack[posPtr->capturetop];
	if (!posPtr->capturetop)
		SETEMPTY(posPtr->capturetop);
	else
		--posPtr->capturetop;
	return r;
}

void pushep(position_t* posPtr, enum SQUARES ep)
{
	if (ISEMPTY(posPtr->eptop))
		posPtr->eptop = 0;
	else
		++posPtr->eptop;
	posPtr->epstack[posPtr->eptop] = ep;
}

enum SQUARES popep(position_t* posPtr)
{
	enum SQUARES r = posPtr->epstack[posPtr->eptop];
	if (!posPtr->eptop)
		SETEMPTY(posPtr->eptop);
	else
		--posPtr->eptop;
	return r;
}

void pushfifty(position_t* posPtr, int counter)
{
	if (ISEMPTY(posPtr->fiftymovetop))
		posPtr->fiftymovetop = 0;
	else
		++posPtr->fiftymovetop;
	return posPtr->fiftymovestack[posPtr->fiftymovetop];
}

int popfifty(position_t* posPtr)
{
	int r = posPtr->fiftymovestack[posPtr->fiftymovetop];
	if (!posPtr->fiftymovetop)
		SETEMPTY(posPtr->fiftymovetop);
	else
		--posPtr->fiftymovetop;
	return r;
}

void make_move(position_t *posPtr, uint16_t mv)
{
	enum SQUARES start = mv & START_SQUARE;
	uint64_t startbb = 1ull << start;
	enum SQUARES end = (mv & END_SQUARE) >> 6;
	uint64_t endbb = 1ull << end;
	enum PIECES color = (posPtr->flags & WHITE_TO_MOVE) ? (WHITE) : (BLACK);
	enum PIECES piece;
	for (int i = PAWN; i <= KING; ++i) {
		if (posPtr->bboards[color + (2 * i)] & startbb) {
			piece = color + (2 * i);
			posPtr->pieces[piece] ^= startbb;
			break;
		}
	}
	if (piece == PAWN) {
		pushfifty(posPtr, posPtr->fiftymove);
		posPtr->fiftymove = 0;
	}
	switch (mv & MOVE_FLAGS) {
	case CAPTURE_MOVE:
		pushfifty(posPtr, posPtr->fiftymove);
		posPtr->fiftymove = 0;
		for (int i = PAWN; i <= KING; ++i) {
			if (posPtr->bboards[(BLACK - color) + (2 * i)] & endbb) {
				posPtr->bboards[(BLACK - color) + (2 * i)] ^= endbb;
				push_capture(posPtr, (BLACK - color) + (2 * i));
				break;
			}
		}
		switch (end) {
		case S_A1:
			posPtr->flags &= ~WHITE_LONG_CASTLE;
			if (!(posPtr->castles[WL_CASTLE]))
				posPtr->castles[WL_CASTLE] = posPtr->moves;
			break;
		case S_H1:
			posPtr->flags &= ~WHITE_SHORT_CASTLE;
			if (!(posPtr->castles[WS_CASTLE]))
				posPtr->castles[WS_CASTLE] = posPtr->moves;
			break;
		case S_A8:
			posPtr->flags &= ~BLACK_LONG_CASTLE;
			if (!(posPtr->castles[BL_CASTLE]))
				posPtr->castles[BL_CASTLE] = posPtr->moves;
			break;
		case S_H8:
			posPtr->flags &= ~BLACK_SHORT_CASTLE;
			if (!(posPtr->castles[BS_CASTLE]))
				posPtr->castles[BS_CASTLE] = posPtr->moves;
			break;
		}
		break;
	case DOUBLE_PAWN_PUSH:
		posPtr->flags |= EN_PASSANT;
		posPtr->flags |= color ? ((end + 8) << 1) : ((end - 8) << 1);
		push_ep(posPtr, color ? ((end + 8) << 1) : ((end - 8) << 1));
		break;
	case SHORT_CASTLE:
		posPtr->bboards[color + (2 * ROOK)] ^= 10ull << start;
		break;
	case LONG_CASTLE:
		/* if black, shifts these values up to the eighth rank */
		posPtr->bboards[color + (2 * ROOK)] ^= 9ull << (color * S_A8);
		break;
	case EP_CAPTURE:
		posPtr->bboards[(BLACK - color) + (2 * PAWN)] ^= 1ull << ((color) ?
				(end + 8) : (end - 8));
		push_capture(posPtr, (BLACK - color) + (2 * PAWN));
		break;
	case KNIGHT_CAPTURE_PROMOTION:
	case KNIGHT_PROMOTION:
		piece = KNIGHT;
		break;
	case BISHOP_CAPTURE_PROMOTION:
	case BISHOP_PROMOTION:
		piece = BISHOP;
		break;
	case ROOK_CAPTURE_PROMOTION:
	case ROOK_PROMOTION:
		piece = ROOK;
		break;
	case QUEEN_CAPTURE_PROMOTION:
	case QUEEN_PROMOTION:
		piece = QUEEN;
		break;
	}
	if (posPtr->flags & BOTH_BOTH_CASTLE) {
		switch (start) {
		case S_A1:
			posPtr->flags &= ~WHITE_LONG_CASTLE;
			if (!(posPtr->castles[WL_CASTLE]))
				posPtr->castles[WL_CASTLE] = posPtr->moves;
			break;
		case S_E1:
			posPtr->flags &= ~WHITE_BOTH_CASTLE;
			if (!(posPtr->castles[WS_CASTLE]))
				posPtr->castles[WS_CASTLE] = posPtr->moves;
			if (!(posPtr->castles[WL_CASTLE]))
				posPtr->castles[WL_CASTLE] = posPtr->moves;
			break;
		case S_H1:
			posPtr->flags &= ~WHITE_SHORT_CASTLE;
			if (!(posPtr->castles[WS_CASTLE]))
				posPtr->castles[WS_CASTLE] = posPtr->moves;
			break;
		case S_A8:
			posPtr->flags &= ~BLACK_LONG_CASTLE;
			if (!(posPtr->castles[BL_CASTLE]))
				posPtr->castles[BL_CASTLE] = posPtr->moves;
			break;
		case S_E8:
			posPtr->flags &= ~BLACK_BOTH_CASTLE;
			if (!(posPtr->castles[BS_CASTLE]))
				posPtr->castles[BS_CASTLE] = posPtr->moves;
			if (!(posPtr->castles[BL_CASTLE]))
				posPtr->castles[BL_CASTLE] = posPtr->moves;
			break;
		case S_H8:
			posPtr->flags &= ~BLACK_SHORT_CASTLE;
			if (!(posPtr->castles[BS_CASTLE]))
				posPtr->castles[BS_CASTLE] = posPtr->moves;
			break;
		}
	}
	posPtr->bboards[color + (2 * piece)] ^= endbb;
	posPtr->bboards[WHITE] = 
		posPtr->bboards[WHITE_PAWN] | posPtr->bboards[WHITE_KNIGHT] |
		posPtr->bboards[WHITE_BISHOP] | posPtr->bboards[WHITE_ROOK] |
		posPtr->bboards[WHITE_QUEEN] | posPtr->bboards[WHITE_KING];
	posPtr->bboards[BLACK] = 
		posPtr->bboards[BLACK_PAWN] | posPtr->bboards[BLACK_KNIGHT] |
		posPtr->bboards[BLACK_BISHOP] | posPtr->bboards[BLACK_ROOK] |
		posPtr->bboards[BLACK_QUEEN] | posPtr->bboards[BLACK_KING];
	posPtr->bboards[OCCUPIED] = 
		posPtr->bboards[WHITE] | posPtr->bboards[BLACK];
	posPtr->bboards[EMPTY] = ~posPtr->bboards[OCCUPIED];
	posPtr->flags &= ~EN_PASSANT;
	posPtr->flags &= ~EP_SQUARE;
	posPtr->flags &= ~(WHITE_CHECK | BLACK_CHECK);
	posPtr->flags |= check_status(*posPtr);
	posPtr->flags ^= WHITE_TO_MOVE;
}

void unmake_move(struct position_t *posPtr, uint16_t mv)
{
	int start = mv & START_SQUARE;
	uint64_t startbb = 1ull << start;
	int end = (mv & END_SQUARE) >> 6;
	uint64_t endbb = 1ull << end;
	/* these are supposed to be backwards; it's the player to 'unmove' */
	int color = (posPtr->flags & WHITE_TO_MOVE) ? (BLACK) : (WHITE);
	int piece;
	int epsq;
	assert((mv != 0) && (mv != ERROR_MOVE));
	assert(endbb & posPtr->pieces[color][0]);
	for (int i = PAWN; i <= KING; ++i) {
		if (posPtr->pieces[color][i] & endbb) {
			piece = i;
			posPtr->pieces[color][piece] ^= endbb;
			posPtr->pieces[color][0] ^= endbb;
			break;
		}
	}
	if (piece == KING)
		posPtr->kingpos[color] = start;
	if (posPtr->fiftymove != 0)
		--posPtr->fiftymove;
	if (mv & CAPTURE_MOVE) {
		/* toggles pawn on wrong square for ep */
		posPtr->pieces[BLACK - color][pop_capture(posPtr)] |= endbb;
		posPtr->pieces[BLACK - color][0] |= endbb;
	}
	if (epsq = pop_ep(posPtr)) {
		posPtr->flags |= EN_PASSANT;
		posPtr->flags |= epsq;
	} else {
		posPtr->flags &= ~EN_PASSANT | ~EP_SQUARE;
	}
	switch (mv & QUEEN_CAPTURE_PROMOTION) {
	case KINGSIDE_CASTLE:
		posPtr->pieces[color][ROOK] ^= 10ull << start;
		posPtr->pieces[color][0] ^= 10ull << start;
		break;
	case QUEENSIDE_CASTLE:
		posPtr->pieces[color][ROOK] ^= 9ull << (color * S_A8);
		posPtr->pieces[color][0] ^= 9ull << (color * S_A8);
		break;
	case EP_CAPTURE:
		/* fix restored capture */
		posPtr->pieces[BLACK - color][PAWN] ^= endbb | (1ull <<
				(color ? (end + 8) : (end - 8)));
		posPtr->pieces[BLACK - color][0] ^= endbb | (1ull <<
				(color ? (end + 8) : (end - 8)));
		break;
	case KNIGHT_CAPTURE_PROMOTION:
	case KNIGHT_PROMOTION:
	case BISHOP_CAPTURE_PROMOTION:
	case BISHOP_PROMOTION:
	case ROOK_CAPTURE_PROMOTION:
	case ROOK_PROMOTION:
	case QUEEN_CAPTURE_PROMOTION:
	case QUEEN_PROMOTION:
		piece = PAWN;
		break;
	}
	for (int i = 0; i <= BQ_CASTLE; ++i) {
		if (posPtr->castles[i] == posPtr->moves) {
			posPtr->flags |= 1 << (7 + i);
			posPtr->castles[i] = 0;
		}
	}
	--posPtr->moves;
	posPtr->pieces[color][piece] ^= startbb;
	posPtr->pieces[color][0] ^= startbb;
	posPtr->occupied = posPtr->pieces[WHITE][0] | posPtr->pieces[BLACK][0];
	posPtr->empty = ~posPtr->occupied;
	posPtr->flags &= ~(WHITE_CHECK | BLACK_CHECK);
	posPtr->flags |= check_status(*posPtr);
	posPtr->flags ^= WHITE_TO_MOVE;
}

