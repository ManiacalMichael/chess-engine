#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "headers/chess.h"
#include "headers/utility.h"

/*
 * Returns the indice of the ls1b in a bitboard with exactly one bit set
 */
/*
 * Trust the magic array, it's one of the few things that worked
 * exactly as intended in the last engine
 */
int bitindice(uint64_t bb)
{
	static const int arr[67] = {
		-1, 0, 1, 39, 2, 15, 40, 23, 3, 12, 16, 59, 41, 19, 24,
		54, 4, -1, 13, 10, 17, 62, 60, 28, 42, 30, 20, 51, 25,
		44, 55, 47, 5, 32, -1, 38, 14, 22, 11, 58, 18, 53, 63,
		9, 61, 27, 29, 50, 43, 46, 31, 37, 21, 57, 52, 8, 26,
		49, 45, 36, 56, 7, 48, 35, 6, 34, 33
	};
	return arr[bb % 67];
}

/*
 * Returns the number of bits set in a bitboard
 */
int popcount(uint64_t bb)
{
	int x = 0;
	while (bb != 0ull) {
		bb &= bb - 1;
		x++;
	}
	return x;
}


/*
 * Allocates and initializes a movenode
 */
struct movenode_t *allocnode(void)
{
	struct movenode_t *p = malloc(sizeof(struct movenode_t));
	if (p != NULL) {
		p->move = 0;
		p->nxt = NULL;
	} else {
		fprintf(stderr, "FATAL ERROR: Allocation failed\n");
		abort();
	}
	return p;
}

/*
 * Adds a movenode with ->move = mv at the end of a linked list
 */
void add_node(struct movenode_t *p, uint32_t mv)
{
	while(p->nxt != NULL) 
		p = p->nxt;
	p->nxt = allocnode();
	p = p->nxt;
	p->move = mv;
	return;
}

/*
 * Removes a node from list and frees memory
 */
void remove_node(struct movenode_t *p)
{
	struct movenode_t *temp = p->nxt;
	if (temp != NULL) {
		p->move = p->nxt->move;
		p->nxt = p->nxt->nxt;
		free(temp);
	} else {
		free(p);
	}
}

/* 
 * Removes tail node of a list
 */
void remove_tail_node(struct movelist_t *listPtr)
{
	struct movenode_t *p = listPtr->root;
	if (p->nxt == NULL) { /* Only node in list */
		listPtr->root = NULL;
		free(p);
	} else {
		while (p->nxt->nxt != NULL)
			p = p->nxt;
		free(p->nxt);
		p->nxt = NULL;
	}
}

/*
 * Frees memory for every node in a linked list
 * Breaks link to list
 */
void delete_list(struct movelist_t *ls)
{
	struct movenode_t *p = ls->root;
	while (p != NULL) {
		ls->root = p->nxt;
		free(p);
		p = ls->root;
	}
	return;
}

/*
 * Converts an attack set to a headless list of moves
 * DOES NOT SET:
 * 	- CAUSES_CHECK
 * 	- CAPTURES_EP
 */
struct movenode_t *serialize_moves(int sq, uint64_t attk, 
		struct board_t *boardPtr)
{
	struct movenode_t *root, *p, *q = NULL;
	uint32_t mv = 0;
	uint32_t prmv = 0;
	int dest = 0;
	int sqt = 0, destt = 0;
	uint64_t sqboard = 1ull << sq;
	uint64_t destboard = 1ull << sq;
	if (attk == 0x0000000000000000ull)
		return NULL;
	/* set piece type */
	if (boardPtr->black_pieces & sqboard)
		sqt += 1;
	if (boardPtr->pawns & sqboard)
		sqt += WHITE_PAWN;
	else if (boardPtr->rooks & sqboard)
		sqt += WHITE_ROOK;
	else if (boardPtr->knights & sqboard)
		sqt += WHITE_KNIGHT;
	else if (boardPtr->bishops & sqboard)
		sqt += WHITE_BISHOP;
	else if (boardPtr->kings & sqboard)
		sqt += WHITE_KING;
	else if (boardPtr->queens & sqboard)
		sqt += WHITE_QUEEN;
	root = allocnode();
	p = root;
	/* create moves for every bit in attack set */
	while (attk != 0x0000000000000000ull) {
		mv = sq;
		/* x ^ (x & (x - 1)) = ls1b */
		dest = bitindice(attk ^ (attk & (attk - 1)));
		mv |= dest << 6;
		destboard = 1ull << dest;
		/* captures */
		if (boardPtr->black_pieces & destboard)
			mv |= CAPTURES_BLACK;
		if (boardPtr->pawns & destboard)
			mv |= CAPTURES_PAWN;
		else if (boardPtr->rooks & destboard)
			mv |= CAPTURES_ROOK;
		else if (boardPtr->knights & destboard)
			mv |= CAPTURES_KNIGHT;
		else if (boardPtr->bishops & destboard)
			mv |= CAPTURES_BISHOP;
		else if (boardPtr->queens & destboard)
			mv |= CAPTURES_QUEEN;
		if (mv & CAPTURED_PIECE)
			mv |= CAPTURE_MOVE;
		/* check for castling */
		if ((sqt / 2) == 6) {
			if (dest == (sq + 2)) {
				mv |= CASTLE_MOVE;
				mv |= CASTLES_KINGSIDE;
			} else if (dest == (sq - 2)) {
				mv |= CASTLE_MOVE;
			}
		/* check for promotions */
		/* two moves for promotions: to queen, to knight */
		} else if ((sqt / 2) == 1) {
			if (sqt % 2) { 
				if ((dest / 8) == 0) {
					mv |= PROMO_MOVE;
					prmv = mv;
					mv |= PROMO_TO_Q;
					p->move = mv;
					p->nxt = allocnode();
					p = p->nxt;
					mv = prmv;
				}
			} else {
				if ((dest / 8) == 7) {
					mv |= PROMO_MOVE;
					prmv = mv;
					mv |= PROMO_TO_Q;
					p->move = mv;
					p->nxt = allocnode();
					p = p->nxt;
					mv = prmv;
				}
			}
		}
		p->move = mv;
		p->nxt = allocnode();
		/* advance list, reset ls1b */
		q = p;
		p = p->nxt;
		attk &= attk - 1;
	}
	free(p); /* remove empty node at end of list */
	q->nxt = NULL;
	return root;
}

/*
 * Adds headless movelist to the end of ls
 */
void cat_lists(struct movelist_t *ls, struct movenode_t *headless)
{
	struct movenode_t *p = ls->root;
	if (headless == NULL)
		return;
	if (p == NULL) {
		ls->root = headless;
		return;
	}
	while (p->nxt != NULL)
		p = p->nxt;
	p->nxt = headless;
	return;
}

/*
 * Makes move mv on position pos, sets proper flags, end piece, etc.
 */
void make_move(struct position_t* posPtr, uint32_t mv)
{
	struct board_t *boardPtr = &posPtr->board;
	int sq = 0, dest = 0;
	int sqt = 0;
	uint64_t sqboard = 0, destboard = 0;
	sq = mv & START_SQUARE;
	dest = (mv & END_SQUARE) >> 6;
	sqboard = 1ull << sq;
	destboard = 1ull << dest;
	/* set piece type */
	if (boardPtr->black_pieces & sqboard)
		sqt += 1;
	if (boardPtr->pawns & sqboard)
		sqt += WHITE_PAWN;
	else if (boardPtr->rooks & sqboard)
		sqt += WHITE_ROOK;
	else if (boardPtr->knights & sqboard)
		sqt += WHITE_KNIGHT;
	else if (boardPtr->bishops & sqboard)
		sqt += WHITE_BISHOP;
	else if (boardPtr->kings & sqboard)
		sqt += WHITE_KING;
	else if (boardPtr->queens & sqboard)
		sqt += WHITE_QUEEN;
	/* set/clear e.p. square */
	posPtr->flags &= ~EP_SQUARE;
	if ((sqt / 2) == 1) {
		posPtr->fiftymove = 0;
		if (dest == (sq - 16)) {
			posPtr->flags |= EN_PASSANT;
			posPtr->flags |= (dest + 8) << 1;
		} else if (dest == (sq + 16)) {
			posPtr->flags |= EN_PASSANT;
			posPtr->flags |= (dest - 8) << 1;
		} else {
			posPtr->flags &= ~EN_PASSANT;
		}
	} else {
		posPtr->fiftymove++;
		posPtr->flags &= ~EN_PASSANT;
	}
	/* perform e.p. captures */
	if (mv & EP_CAPTURE) {
		if (sqt % 2) {
			boardPtr->occupied ^= 1ull << (dest + 8);
			boardPtr->pawns ^= 1ull << (dest + 8);
		} else {
			boardPtr->occupied ^= 1ull << (dest - 8);
			boardPtr->pawns ^= 1ull << (dest - 8);
			boardPtr->black_pieces ^= 1ull << (dest - 8);
		}
	/* perform other captures */
	} else if (mv & CAPTURE_MOVE) {
		posPtr->fiftymove = 0;
		boardPtr->occupied ^= destboard;
		switch (mv & CAPTURED_PIECE) {
			case CAPTURES_PAWN:
				boardPtr->pawns ^= destboard;
				break;
			case CAPTURES_KNIGHT:
				boardPtr->knights ^= destboard;
				break;
			case CAPTURES_BISHOP:
				boardPtr->bishops ^= destboard;
				break;
			case CAPTURES_ROOK:
				boardPtr->rooks ^= destboard;
				break;
			case CAPTURES_QUEEN:
				boardPtr->queens ^= destboard;
				break;
		}
		if (mv & CAPTURES_BLACK)
			boardPtr->black_pieces ^= destboard;
	}
	/* set destination and start squares */
	if (mv & PROMO_MOVE) {
		if (mv & PROMO_TO_Q)
			boardPtr->queens |= destboard;
		else
			boardPtr->knights |= destboard;
		if (sqt % 2) {
			boardPtr->pawns ^= sqboard;
			boardPtr->black_pieces |= destboard;
			boardPtr->black_pieces ^= sqboard;
		} else {
			boardPtr->pawns ^= sqboard;
		}
	} else if (mv & CASTLE_MOVE) {
		if (mv & CASTLES_KINGSIDE) {
			boardPtr->kings ^= sqboard;
			boardPtr->kings ^= destboard;
			boardPtr->occupied ^= 1ull << (dest + 1);
			boardPtr->occupied ^= 1ull << (dest - 1);
			boardPtr->rooks ^= 1ull << (dest + 1);
			boardPtr->rooks ^= 1ull << (dest - 1);
			if (sqt % 2) {
				boardPtr->black_pieces ^= sqboard;
				boardPtr->black_pieces ^= destboard;
				boardPtr->black_pieces ^= 1ull << (dest + 1);
				boardPtr->black_pieces ^= 1ull << (dest - 1);
			}
		} else {
			boardPtr->kings ^= sqboard;
			boardPtr->kings ^= destboard;
			boardPtr->occupied ^= 1ull << (dest - 2);
			boardPtr->occupied ^= 1ull << (dest + 1);
			boardPtr->rooks ^= 1ull << (dest - 2);
			boardPtr->rooks ^= 1ull << (dest + 1);
			if (sqt % 2) {
				boardPtr->black_pieces ^= sqboard;
				boardPtr->black_pieces ^= destboard;
				boardPtr->black_pieces ^= 1ull << (dest - 2);
				boardPtr->black_pieces ^= 1ull << (dest + 1);
			}
		}
	} else {
		switch (sqt / 2) {
			case 1:
				boardPtr->pawns ^= sqboard;
				boardPtr->pawns |= destboard;
				break;
			case 2:
				boardPtr->knights ^= sqboard;
				boardPtr->knights |= destboard;
				break;
			case 3:
				boardPtr->bishops ^= sqboard;
				boardPtr->bishops |= destboard;
				break;
			case 4:
				boardPtr->rooks ^= sqboard;
				boardPtr->rooks |= destboard;
				break;
			case 5:
				boardPtr->queens ^= sqboard;
				boardPtr->queens |= destboard;
				break;
			case 6:
				boardPtr->kings ^= sqboard;
				boardPtr->kings |= destboard;
				break;
		}
		if (sqt % 2) {
			boardPtr->black_pieces ^= sqboard;
			boardPtr->black_pieces |= destboard;
		}
	}
	boardPtr->occupied ^= sqboard;
	boardPtr->occupied ^= destboard;
	/* if anyone still has castle rights */
	if (posPtr->flags & BOTH_BOTH_CASTLE) {
		/* remove castle rights */
		switch (sq) {
			case S_A1:
				posPtr->flags &= ~WHITE_KINGSIDE_CASTLE;
				break;
			case S_E1:
				posPtr->flags &= ~WHITE_BOTH_CASTLE;
				break;
			case S_H1:
				posPtr->flags &= ~WHITE_QUEENSIDE_CASTLE;
				break;
			case S_A8:
				posPtr->flags &= ~BLACK_KINGSIDE_CASTLE;
				break;
			case S_E8:
				posPtr->flags &= ~BLACK_BOTH_CASTLE;
				break;
			case S_H8:
				posPtr->flags &= ~BLACK_QUEENSIDE_CASTLE;
				break;
		}
	}
	/* set check status */
	posPtr->flags &= ~(WHITE_CHECK | BLACK_CHECK);
	if (mv & CAUSES_CHECK) {
		if (mv & WHITE_TO_MOVE)
			posPtr->flags |= BLACK_CHECK;
		else
			posPtr->flags |= WHITE_CHECK;
	}
	posPtr->moves++;
	posPtr->flags ^= WHITE_TO_MOVE;
	return;
}
