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

/* Declared in: movegen.c */
extern const uint64_t file_masks[8];

extern const uint64_t rank_masks[8];

extern const uint64_t diagonal_masks[15];

extern const uint64_t antidiagonal_masks[15];

extern const uint8_t sliding_attack_lookups[512];

extern const uint64_t king_attack_lookups[64];

extern const uint64_t knight_attack_lookups[64];

extern const uint64_t pawn_twosquare[8];

extern const uint64_t pawn_movement[64];

extern const uint64_t pawn_captures[64];



struct movelist_t generate_moves(struct position_t *);



signed evaluate(struct position_t *);

signed negamax(struct position_t *, int, signed, signed);

#endif
