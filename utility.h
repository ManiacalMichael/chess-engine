/*
 * * * utility.h
 * Lists, list management and bitboard tools
 */

#ifndef INCLUDE_UTILITY_H
#define INCLUDE_UTILITY_H

#include "chess.h"
#include <stdlib.h>
#include <stdint.h>

/*
 * Move bits:
 *
 * lower 16:
 * 0123456 	- Start square
 * 789abcd 	- End square
 * ef		- Unused
 *
 * higher 16:
 * 0		- Is a castle
 * 1		- Is a kingside castle
 * 2		- Is a promotion
 * 3		- Is a promotion to queen
 * 4		- Captures a piece
 * 5		- Captures a piece e.p.
 * 	Captured piece:
 * 6		- Is black
 * 7		- Is a pawn
 * 8		- Is a knight
 * 9		- Is a bishop
 * a		- Is a rook
 * b		- Is a queen
 * 
 * c		- Places opponent in check
 * def		- Unused
 */
#define START_SQUARE 0x0000007fu
#define END_SQUARE 0x00003f80u
#define CASTLE_MOVE 0x00010000u
#define CASTLES_KINGSIDE 0x00020000u
#define PROMO_MOVE 0x00040000u
#define PROMO_TO_Q 0x00080000u
#define CAPTURE_MOVE 0x00100000u
#define EP_CAPTURE 0x00200000u
#define CAPTURES_BLACK 0x00400000u
#define CAPTURES_PAWN 0x00800000u
#define CAPTURES_KNIGHT 0x01000000u
#define CAPTURES_BISHOP 0x02000000u
#define CAPTURES_ROOK 0x04000000u
#define CAPTURES_QUEEN 0x08000000u
#define CAPTURED_PIECE 0x0f800000u
#define CAUSES_CHECK 0x10000000u

struct movenode_t {
	uint32_t move;
	struct movenode_t *nxt;
};

/*
 * Wrapper struct for lists prevents accidental modification of root ptr
 */
struct movelist_t {
	struct movenode_t *root;
	int nodes;
};



/* Bitboard utilities */

/* Returns the indice of a bit in a bitboard with ONLY one bit set */
int bitindice(uint64_t);

/* Returns the number of set bits in a bitboard */
int popcount(uint64_t);



/* Movenode utilities */

/* Allocates and initializes struct, returns pointer to new struct */
struct movenode_t *allocnode(void);

/*
 * Inserts node with .move = uint32_t at end of list
 * struct movenode_t * may be anywhere in a linked list
 */
void add_node(struct movenode_t *, uint32_t);

/*
 * Removes node, returns node->nxt
 * use like
 * p->nxt = remove_node(p->nxt);
 */
struct movenode_t *remove_node(struct movenode_t *);



/* Movelist utilities */
/* NOTE: movelist_t structs should not be dynamically allocated */

/* Deletes a movelist */
void delete_list(struct movelist_t *);

/* Converts an attack set into a list of moves(without movelist_t head) */
/* Sets most flags but DOES NOT SET:
 * 	- CAUSES_CHECK
 * 	- CAPTURES_EP
 * These should be set by movegen, which has access to the attack sets
 * to determine check status, and the position flags for e.p. captures
 */
struct movenode_t *serialize_to_moves(int, uint64_t, struct board_t *);

/* Adds a headless movelist to a movelist */
/* e.g. cat_lists( moves, serialize_to_moves(sq, attk, board) ) */
void cat_lists(struct movelist_t *, struct movenode_t *);



/* Makes a move, properly setting all the relevant flags on the position */
void make_move(struct position_t *, uint32_t);



#endif
