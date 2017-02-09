/*
 * * * search.h
 * Attack & movement lookup tables
 * Piece-Square tables for eval
 * Function declarations for movegen and search
 */
#ifndef INCLUDE_SEARCH_H
#define INCLUDE_SEARCH_H

#include <stdint.h>

#define INFINITY -32768
#define NEG_INFINITY 32767


/* Indice = (6bit key * 8) + file */
extern const uint8_t sliding_attack_lookups[512];

extern const uint64_t king_attack_lookups[64];

extern const uint64_t knight_attack_lookups[64];

extern const uint64_t pawn_doublepush[2][8];

extern const uint64_t pawn_movement[2][64];

extern const uint64_t pawn_attacks[2][64];



/*
 * void serialize_moves()
 * Converts an attack set to moves and adds them to a movelist
 * 	@sq - Square the piece is on
 * 	@attk - Attack set of the piece on square `sq`
 * 	@posPtr - pointer to the position the attack set was generated from
 * 	@lsPtr - pointer to the list the moves will be added to
 */
void serialize_moves(int sq, uint64_t attk, struct position_t *posPtr,
		uint16_t *lsPtr);

/*
 * void generate_moves()
 * Populates a movelist with possible, but not nessicarily legal, moves 
 * possible from a position
 * 	@pos - Position to generate moves for
 * 	@lsPtr - Pointer to the movelist to use
 */
void generate_moves(struct position_t pos, uint16_t *lsPtr);

/*
 * int perft()
 * Recursively calculates the number of valid movepaths at a certain depth
 * 	@posPtr - Position to test
 * 	@depth - Depth to search
 */
int perft(struct position_t *posPtr, int depth);

/*
 * signed evaluate()
 * Returns the evaluation for a position, relative to the side to move
 * 	@pos - Position to evaluate
 */
signed evaluate(struct position_t pos);

/*
 * signed negamax()
 * Recursive negamax search function, returns evaluation of best child
 * 	@posPtr - pointer to position to search from
 * 	@depth - depth to search
 * 	@alpha - minimum score
 * 	@beta - maximum score
 */
signed negamax(struct position_t *posPtr, int depth, signed alpha, signed beta);



#endif
