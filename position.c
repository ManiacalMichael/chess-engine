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
	.capturetop = -1,
	.epstack = { { 0 }, { 0 } },
	.eptop = -1,
	.castles = { 0 },
	.fiftymovestack = { 0 },
	.fiftymovetop = -1,
	.flags = 0x8780u,
	.halfmove = 0,
	.fiftymove = 0
};


inline int popcount(uint64_t bb)
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

const uint64_t mul = 0x03f79d71b4cb0a89;

inline int popfirst(uint64_t *bb)
{
	uint64_t b = (*bb ^ (*bb - 1)) & *bb;
	*bb &= (*bb - 1);
	return index64[(b * mul) >> 58];
}

inline int ls1bindice(uint64_t bb)
{
	bb &= bb ^ (bb - 1);
	return index64[(bb * mul) >> 58];
}

inline void pushcapture(position_t *posPtr, enum PIECES capture)
{
	posPtr->capturestack[++posPtr->capturetop] = capture;
}

inline enum PIECES popcapture(position_t *posPtr)
{
	return posPtr->capturestack[posPtr->capturetop--];
}

inline void pushep(position_t *posPtr, enum SQUARES ep)
{
	posPtr->epstack[1][++posPtr->eptop] = ep;
	posPtr->epstack[0][posPtr->eptop] = posPtr->halfmove;
}

inline enum SQUARES popep(position_t *posPtr)
{
	return posPtr->epstack[1][posPtr->eptop--];
}

inline void pushfifty(position_t *posPtr, int counter)
{
	posPtr->fiftymovestack[++posPtr->fiftymovetop] = counter;
}

inline int popfifty(position_t *posPtr)
{
	return posPtr->fiftymovestack[posPtr->fiftymovetop--];
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
			posPtr->bboards[piece] ^= startbb;
			break;
		}
	}
	if (piece == PAWN) {
		pushfifty(posPtr, posPtr->fiftymove);
		posPtr->fiftymove = 0;
	}
	if (((mv & MOVE_FLAGS) >= CAPTURE_PROMO_TO_KNIGHT) ||
	    ((mv & MOVE_FLAGS) == CAPTURE_MOVE)) {
		pushfifty(posPtr, posPtr->fiftymove);
		posPtr->fiftymove = 0;
		for (int i = PAWN; i <= KING; ++i) {
			if (posPtr->bboards[(BLACK - color) + (2 * i)] & endbb) {
				posPtr->bboards[(BLACK - color) + (2 * i)] ^= endbb;
				pushcapture(posPtr, (BLACK - color) + (2 * i));
				break;
			}
		}
		switch (end) {
		case S_A1:
			posPtr->flags &= ~WHITE_LONG_CASTLE;
			if (!(posPtr->castles[WL_CASTLE]))
				posPtr->castles[WL_CASTLE] = posPtr->halfmove;
			break;
		case S_H1:
			posPtr->flags &= ~WHITE_SHORT_CASTLE;
			if (!(posPtr->castles[WS_CASTLE]))
				posPtr->castles[WS_CASTLE] = posPtr->halfmove;
			break;
		case S_A8:
			posPtr->flags &= ~BLACK_LONG_CASTLE;
			if (!(posPtr->castles[BL_CASTLE]))
				posPtr->castles[BL_CASTLE] = posPtr->halfmove;
			break;
		case S_H8:
			posPtr->flags &= ~BLACK_SHORT_CASTLE;
			if (!(posPtr->castles[BS_CASTLE]))
				posPtr->castles[BS_CASTLE] = posPtr->halfmove;
			break;
		}
	}
	switch (mv & MOVE_FLAGS) {
	case DOUBLE_PAWN_PUSH:
		posPtr->flags |= EP_AVAILABLE;
		posPtr->flags |= color ? ((end + 8) << 1) : ((end - 8) << 1);
		pushep(posPtr, color ? ((end + 8) << 1) : ((end - 8) << 1));
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
		pushcapture(posPtr, (BLACK - color) + (2 * PAWN));
		break;
	case CAPTURE_PROMO_TO_KNIGHT:
	case PROMO_TO_KNIGHT:
		piece = KNIGHT;
		break;
	case CAPTURE_PROMO_TO_BISHOP:
	case PROMO_TO_BISHOP:
		piece = BISHOP;
		break;
	case CAPTURE_PROMO_TO_ROOK:
	case PROMO_TO_ROOK:
		piece = ROOK;
		break;
	case CAPTURE_PROMO_TO_QUEEN:
	case PROMO_TO_QUEEN:
		piece = QUEEN;
		break;
	}
	if (posPtr->flags & BOTH_BOTH_CASTLE) {
		switch (start) {
		case S_A1:
			posPtr->flags &= ~WHITE_LONG_CASTLE;
			if (!posPtr->castles[WL_CASTLE])
				posPtr->castles[WL_CASTLE] = posPtr->halfmove;
			break;
		case S_E1:
			posPtr->flags &= ~WHITE_BOTH_CASTLE;
			if (!posPtr->castles[WS_CASTLE])
				posPtr->castles[WS_CASTLE] = posPtr->halfmove;
			if (!posPtr->castles[WL_CASTLE])
				posPtr->castles[WL_CASTLE] = posPtr->halfmove;
			break;
		case S_H1:
			posPtr->flags &= ~WHITE_SHORT_CASTLE;
			if (!posPtr->castles[WS_CASTLE])
				posPtr->castles[WS_CASTLE] = posPtr->halfmove;
			break;
		case S_A8:
			posPtr->flags &= ~BLACK_LONG_CASTLE;
			if (!posPtr->castles[BL_CASTLE])
				posPtr->castles[BL_CASTLE] = posPtr->halfmove;
			break;
		case S_E8:
			posPtr->flags &= ~BLACK_BOTH_CASTLE;
			if (!posPtr->castles[BS_CASTLE])
				posPtr->castles[BS_CASTLE] = posPtr->halfmove;
			if (!posPtr->castles[BL_CASTLE])
				posPtr->castles[BL_CASTLE] = posPtr->halfmove;
			break;
		case S_H8:
			posPtr->flags &= ~BLACK_SHORT_CASTLE;
			if (!posPtr->castles[BS_CASTLE])
				posPtr->castles[BS_CASTLE] = posPtr->halfmove;
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
	posPtr->flags &= ~EP_AVAILABLE;
	posPtr->flags &= ~EP_SQUARE;
	posPtr->flags &= ~(WHITE_CHECK | BLACK_CHECK);
	posPtr->flags |= check_status(*posPtr);
	posPtr->flags ^= WHITE_TO_MOVE;
}

void unmake_move(position_t *posPtr, uint16_t mv)
{
	enum SQUARES start = mv & START_SQUARE;
	uint64_t startbb = 1ull << start;
	enum SQUARES end = (mv & END_SQUARE) >> 6;
	uint64_t endbb = 1ull << end;
	/* these are supposed to be backwards; it's the player to 'unmove' */
	enum PIECES color = (posPtr->flags & WHITE_TO_MOVE) ? (BLACK) : (WHITE);
	enum PIECES piece;
	int fiftyzeroed = 0;
	--posPtr->halfmove;
	for (int i = PAWN; i <= KING; ++i) {
		if (posPtr->bboards[color + (2 * i)] & startbb) {
			piece = color + (2 * i);
			posPtr->bboards[piece] ^= endbb;
			break;
		}
	}
	switch (mv & MOVE_FLAGS) {
	case CAPTURE_MOVE:
		fiftyzeroed = 1;
		posPtr->bboards[popcapture(posPtr)] ^= endbb;
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
		fiftyzeroed = 1;
		popcapture(posPtr);
		break;
	case CAPTURE_PROMO_TO_KNIGHT:
	case CAPTURE_PROMO_TO_BISHOP:
	case CAPTURE_PROMO_TO_ROOK:
	case CAPTURE_PROMO_TO_QUEEN:
		fiftyzeroed = 1;
		posPtr->bboards[popcapture(posPtr)] ^= endbb;
	case PROMO_TO_QUEEN:
	case PROMO_TO_KNIGHT:
	case PROMO_TO_BISHOP:
	case PROMO_TO_ROOK:
		piece = PAWN;
		break;
	}
	if ((piece == PAWN) || fiftyzeroed)
		posPtr->fiftymove = popfifty(posPtr);
	if (posPtr->epstack[0][posPtr->eptop] == posPtr->halfmove)
		posPtr->flags |= popep(posPtr) << 1;
	for (int i = WS_CASTLE; i <= BL_CASTLE; ++i) {
		if (posPtr->castles[i] == posPtr->halfmove) {
			posPtr->castles[i] = 0;
			posPtr->flags |= 1 << 7 + i;
		}
	}
	posPtr->bboards[color + (2 * piece)] ^= startbb;
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
	posPtr->flags &= ~(WHITE_CHECK | BLACK_CHECK);
	posPtr->flags |= check_status(*posPtr);
	posPtr->flags ^= WHITE_TO_MOVE;
}
