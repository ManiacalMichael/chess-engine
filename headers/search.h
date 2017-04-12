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

extern const uint64_t file_masks[8];

extern const uint64_t rank_masks[8];

/* Indice = (7 + rank) - file */
extern const uint64_t diagonal_masks[15];

/* Indice = rank + file */
extern const uint64_t antidiagonal_masks[15];

/* Indice = (6bit key * 8) + file */
extern const uint8_t sliding_attack_lookups[512];

extern const uint64_t king_attack_lookups[64];

extern const uint64_t knight_attack_lookups[64];

extern const uint64_t pawn_doublepush[2][8];

extern const uint64_t pawn_movement[2][64];

extern const uint64_t pawn_attacks[2][64];



/*
 * uint64_t pawn_moves()
 * Returns an attack set for a pawn
 * 	@enemy - Bitboard of squares occupied by enemy pieces
 * 	@empty - Bitboard of empty squares
 * 	@color - Color of the pawn in COLORS
 * 	@sq - Square the pawn is on
 */
uint64_t pawn_moves(uint64_t enemy, uint64_t empty, int color, int sq);

/*
 * uint64_t bishop_moves()
 * Returns an attack set for a bishop
 * 	@occupied - Bitboard of occupied squares
 * 	@rank - Rank the piece is on
 * 	@file - File the piece is on
 */
uint64_t bishop_moves(uint64_t occupied, int rank, int file);

/*
 * uint64_t rook_moves()
 * Returns an attack set for a rook
 * 	@occupied - Bitboard of occupied squares
 * 	@rank - Rank the piece is on
 * 	@file - File the piece is on
 */
uint64_t rook_moves(uint64_t occupied, int rank, int file);

/*
 * uint64_t queen_moves()
 * Returns an attack set for a queen
 * 	@occupied - Bitboard of occupied squares
 * 	@rank - Rank the piece is on
 * 	@file - File the piece is on
 */
uint64_t queen_moves(uint64_t occupied, int rank, int file);

/*
 * uint16_t check_status()
 * Returns check status of a position
 * 	@pos - Position to check
 */
uint16_t check_status(const struct position_t pos);

/*
 * uint64_t castle_moves()
 * Returns attack set of a king for castling only
 * 	@pos - Position to generate moves for
 */
uint64_t castle_moves(struct position_t pos);

/*
 * void serialize_moves()
 * Converts an attack set to moves and adds them to a movelist
 * 	@start - Square the piece is on
 * 	@attk - Attack set of the piece on square `sq`
 * 	@pos - position the attack set was generated from
 * 	@lsPtr - pointer to the list the moves will be added to
 * Assertions:
 * 	- @start matches a piece of the side to move
 */
void serialize_moves(int start, uint64_t attk, const struct position_t pos,
		uint16_t *lsPtr);

/*
 * void generate_moves()
 * Populates a movelist with possible, but not nessicarily legal, moves 
 * possible from a position
 * 	@pos - Position to generate moves for
 * 	@lsPtr - Pointer to the movelist to use
 */
void generate_moves(const struct position_t pos, uint16_t *lsPtr);

/*
 * unsigned long long perft()
 * Recursively calculates the number of valid movepaths at a certain depth
 * 	@posPtr - Position to test
 * 	@depth - Depth to search
 */
unsigned long long perft(struct position_t *posPtr, int depth);

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
