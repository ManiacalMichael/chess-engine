#include <stdint.h>
#include "headers/chess.h"

const struct position_t START_POSITION = { 
	.pieces = {
		{
			0x000000000000ffffull,
			0x000000000000ff00ull,
			0x0000000000000042ull,
			0x0000000000000024ull,
			0x0000000000000081ull,
			0x0000000000000008ull,
			0x0000000000000010ull
		},
		{
			0xffff000000000000ull,
			0x00ff000000000000ull,
			0x4200000000000000ull,
			0x2400000000000000ull,
			0x8100000000000000ull,
			0x0800000000000000ull,
			0x1000000000000000ull
		}
	},
	.occupied = 0xffff00000000ffffull,
	.empty = 0x0000ffffffff0000ull,
	.captures = { 0 },
	.kingpos = { 4, 60 },
	.ep_history = { { 0 } },
	.castles = { 0, 0, 0, 0 },
	.flags = 0x8780u,	
	.moves = 0,	
	.fiftymove = 0
};


void push_ep(struct position_t *posPtr, int sq)
{
	posPtr->ep_history[0][++posPtr->ep_history[0][0]] = sq;
	posPtr->ep_history[1][posPtr->ep_history[0][0]] = posPtr->moves;
}

int pop_ep(struct position_t *posPtr)
{
	if (posPtr->ep_history[1][posPtr->ep_history[0][0]] != posPtr->moves) 
		return 0;
	return posPtr->ep_history[0][posPtr->ep_history[0][0]--];
}

void push_capture(struct position_t *posPtr, int pt)
{
	++posPtr->captures[0];
	posPtr->captures[posPtr->captures[0]] = pt;
}

int pop_capture(struct position_t *posPtr)
{
	return posPtr->captures[posPtr->captures[0]--];
}

int popcount(uint64_t bb)
{
	int x = 0;
	while (bb != 0ull) {
		bb &= bb - 1;
		++x;
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
	bb ^= (bb & (bb - 1));
	return arr[bb % 67];
}

void make_move(struct position_t *posPtr, uint16_t mv)
{
	int start = mv & START_SQUARE;
	uint64_t startbb = 1ull << start;
	int end = (mv & END_SQUARE) >> 6;
	uint64_t endbb = 1ull << end;
	int color = (posPtr->flags & WHITE_TO_MOVE) ? (WHITE) : (BLACK);
	int piece;
	++posPtr->moves;
	for (int i = PAWN; i <= KING; ++i) {
		if (posPtr->pieces[color][i] & startbb) {
			piece = i;
			posPtr->pieces[color][piece] ^= startbb;
			posPtr->pieces[color][0] ^= startbb;
			break;
		}
	}
	if (piece == PAWN)
		posPtr->fiftymove = 0;
	else if (piece == KING)
		posPtr->kingpos[color] = end;
	if (mv & CAPTURE_MOVE) {
		posPtr->fiftymove = 0;
		/* loop *should* do nothing for e.p. captures */
		for (int i = PAWN; i <= KING; ++i) {
			if (posPtr->pieces[BLACK - color][i] & endbb) {
				posPtr->pieces[BLACK - color][i] ^= endbb;
				posPtr->pieces[BLACK - color][0] ^= endbb;
				push_capture(posPtr, i);
				break;
			}
		}
		switch (end) {
		case S_A1:
			posPtr->flags &= ~WHITE_QUEENSIDE_CASTLE;
			if (!(posPtr->castles[WQ_CASTLE]))
				posPtr->castles[WQ_CASTLE] = posPtr->moves;
			break;
		case S_H1:
			posPtr->flags &= ~WHITE_KINGSIDE_CASTLE;
			if (!(posPtr->castles[WK_CASTLE]))
				posPtr->castles[WK_CASTLE] = posPtr->moves;
			break;
		case S_A8:
			posPtr->flags &= ~BLACK_QUEENSIDE_CASTLE;
			if (!(posPtr->castles[BQ_CASTLE]))
				posPtr->castles[BQ_CASTLE] = posPtr->moves;
			break;
		case S_H8:
			posPtr->flags &= ~BLACK_KINGSIDE_CASTLE;
			if (!(posPtr->castles[BK_CASTLE]))
				posPtr->castles[BK_CASTLE] = posPtr->moves;
			break;
		}
	}
	posPtr->flags &= ~EN_PASSANT | ~EP_SQUARE;
	switch (mv & QUEEN_CAPTURE_PROMOTION) {
	case DOUBLE_PAWN_PUSH:
		posPtr->flags |= EN_PASSANT;
		posPtr->flags |= color ? (end + 8) : (end - 8);
		push_ep(posPtr, color ? (end + 8) : (end - 8) );
		break;
	case KINGSIDE_CASTLE:
		posPtr->pieces[color][ROOK] ^= 5ull << (start + 1);
		posPtr->pieces[color][0] ^= 5ull << start;
		break;
	case QUEENSIDE_CASTLE:
		/* if black, shifts these values up to the eighth rank */
		posPtr->pieces[color][ROOK] ^= 9ull << (color * S_A8);
		posPtr->pieces[color][0] ^= 9ull << (color * S_A8);
		break;
	case EP_CAPTURE:
		posPtr->pieces[BLACK - color][PAWN] ^= 1ull << ((color) ?
				(end + 8) : (end - 8));
		posPtr->pieces[BLACK - color][0] ^= 1ull << ((color) ?
				(end + 8) : (end - 8));
		push_capture(posPtr, PAWN);
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
			posPtr->flags &= ~WHITE_QUEENSIDE_CASTLE;
			if (!(posPtr->castles[WQ_CASTLE]))
				posPtr->castles[WQ_CASTLE] = posPtr->moves;
			break;
		case S_E1:
			posPtr->flags &= ~WHITE_BOTH_CASTLE;
			if (!(posPtr->castles[WK_CASTLE]))
				posPtr->castles[WK_CASTLE] = posPtr->moves;
			if (!(posPtr->castles[WQ_CASTLE]))
				posPtr->castles[WQ_CASTLE] = posPtr->moves;
			break;
		case S_H1:
			posPtr->flags &= ~WHITE_KINGSIDE_CASTLE;
			if (!(posPtr->castles[WK_CASTLE]))
				posPtr->castles[WK_CASTLE] = posPtr->moves;
		case S_A8:
			posPtr->flags &= ~BLACK_QUEENSIDE_CASTLE;
			if (!(posPtr->castles[BQ_CASTLE]))
				posPtr->castles[BQ_CASTLE] = posPtr->moves;
			break;
		case S_E8:
			posPtr->flags &= ~BLACK_BOTH_CASTLE;
			if (!(posPtr->castles[BK_CASTLE]))
				posPtr->castles[BK_CASTLE] = posPtr->moves;
			if (!(posPtr->castles[BQ_CASTLE]))
				posPtr->castles[BQ_CASTLE] = posPtr->moves;
			break;
		case S_H8:
			posPtr->flags &= ~BLACK_KINGSIDE_CASTLE;
			if (!(posPtr->castles[BK_CASTLE]))
				posPtr->castles[BK_CASTLE] = posPtr->moves;
			break;
		}
	}
	posPtr->pieces[color][piece] ^= endbb;
	posPtr->pieces[color][0] ^= endbb;
	posPtr->occupied = posPtr->pieces[WHITE][0] | posPtr->pieces[BLACK][0];
	posPtr->empty = ~posPtr->occupied;
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
		posPtr->pieces[color][ROOK] ^= 5ull << (start + 1);
		posPtr->pieces[color][0] ^= 5ull << start;
		break;
	case QUEENSIDE_CASTLE:
		posPtr->pieces[color][ROOK] ^= 9ull << (color * S_A8);
		posPtr->pieces[color][0] ^= 9ull << (color * S_A8);
		break;
	case EP_CAPTURE:
		/* fix restored capture */
		posPtr->pieces[BLACK - color][PAWN] ^= endbb & (1ull <<
				(color ? (end + 8) : (end - 8)));
		posPtr->pieces[BLACK - color][0] ^= endbb & (1ull <<
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

