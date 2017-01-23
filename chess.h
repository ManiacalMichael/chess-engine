/*
 * * * chess.h
 * Data and structures for representing pieces, board, games
 */

#ifndef INCLUDE_CHESS_H
#define INCLUDE_CHESS_H

#include <stdint.h>

#define FILL_MULTIPLIER 0x0101010101010101ull
#define MAIN_DIAGONAL 0x8040201008040201ull
#define MAIN_ANTIDIAGONAL 0x0102040810204080ull

/*
 * Position flags:
 * 0		- En Passant available
 * 123456	- En Passant square
 * 789A		- Castling status
 * BC		- Check status
 * D		- Game is over
 * E		- Game is drawn
 * F		- White to move
 */
#define EN_PASSANT 0x0001u
#define EP_SQUARE 0x007eu
#define WHITE_KINGSIDE_CASTLE 0x0080u
#define WHITE_QUEENSIDE_CASTLE 0x0100u
#define WHITE_BOTH_CASTLE 0x0180u
#define BLACK_KINGSIDE_CASTLE 0x0200u
#define BLACK_QUEENSIDE_CASTLE 0x0400u
#define BLACK_BOTH_CASTLE 0x0600u
#define BOTH_BOTH_CASTLE 0x0780u
#define BLACK_CHECK 0x0800u
#define WHITE_CHECK 0x1000u
#define GAME_OVER 0x2000u
#define GAME_DRAWN 0x4000u
#define WHITE_TO_MOVE 0x8000u

/*
 * White pieces are even numbered, black pieces are odd numbered
 *  piece_t / 2 produces the same value for pieces of the same type
 */
enum piece_t {
	NO_PIECE,
	WHITE_PAWN = 2, BLACK_PAWN,
	WHITE_KNIGHT, BLACK_KNIGHT,
	WHITE_BISHOP, BLACK_BISHOP,
	WHITE_ROOK, BLACK_ROOK,
	WHITE_QUEEN, BLACK_QUEEN,
	WHITE_KING, BLACK_KING
};

/*
 * Uses little endian square numbering; 0 = a1, 63 = h8
 * Rank = square_t / 8
 * File = square_1 % 8
 */
enum square_t {
	S_A1, S_B1, S_C1, S_D1, S_E1, S_F1, S_G1, S_H1,
	S_A2, S_B2, S_C2, S_D2, S_E2, S_F2, S_G2, S_H2,
	S_A3, S_B3, S_C3, S_D3, S_E3, S_F3, S_G3, S_H3,
	S_A4, S_B4, S_C4, S_D4, S_E4, S_F4, S_G4, S_H4,
	S_A5, S_B5, S_C5, S_D5, S_E5, S_F5, S_G5, S_H5,
	S_A6, S_B6, S_C6, S_D6, S_E6, S_F6, S_G6, S_H6,
	S_A7, S_B7, S_C7, S_D7, S_E7, S_F7, S_G7, S_H7,
	S_A8, S_B8, S_C8, S_D8, S_E8, S_F8, S_G8, S_H8
};

/*
 * 64-bit values have one bit for every square on the board
 * A sieres of 64-bit numbers can be used to represent the positions
 * of the various pieces and their piece types
 */
/*
 * Technically, only four bitboards are needed to represent a full board.
 * The previous iteration of this project used this to reduce memory usage,
 * and roughly %70 - %80 of the instructions executed during movegen ended
 * up being operations to derive piece types from the more compact struct
 */
struct board_t {
	uint64_t occupied;
	uint64_t black_pieces;
	uint64_t kings;
	uint64_t queens;
	uint64_t rooks;
	uint64_t bishops;
	uint64_t knights;
	uint64_t pawns;
};

const struct board_t START_BOARD = {
	0xffff00000000ffffull,
	0xffff000000000000ull,
	0x1000000000000010ull,
	0x0800000000000008ull,
	0x8100000000000081ull,
	0x2400000000000024ull,
	0x4200000000000042ull,
	0x00ff00000000ff00ull
};

struct position_t {
	struct board_t board;

	uint16_t flags;
	int moves;
	int fiftymove;
};

const struct position_t START_POSITION {
	{
		0xffff00000000ffffull,
		0xffff000000000000ull,
		0x1000000000000010ull,
		0x0800000000000008ull,
		0x8100000000000081ull,
		0x2400000000000024ull,
		0x4200000000000042ull,
		0x00ff00000000ff00ull
	},
	0x01e1u,
	0,
	0
};

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

/*
 * southwest-northeast diagonals:
 *
 * 0 0 0 0 0 0 0 0  00
 * 0 0 0 0 0 0 0 0  00
 * 0 0 0 0 0 0 0 1  80
 * 0 0 0 0 0 0 1 0  40
 * 0 0 0 0 0 1 0 0  20
 * 0 0 0 0 1 0 0 0  10
 * 0 0 0 1 0 0 0 0  08
 * 0 0 1 0 0 0 0 0  04
 * 
 * 0x0000804020100804ull
 * S_E3 -> (7 + 2) - 4 = 5
 */
const uint64_t diagonal_masks[15] = {
	/* Indice = (7 + rank) - file */
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

/*
 * northwest-southeast antidiagonals:
 *
 * 0 1 0 0 0 0 0 0  02
 * 0 0 1 0 0 0 0 0  04
 * 0 0 0 1 0 0 0 0  08
 * 0 0 0 0 1 0 0 0  10
 * 0 0 0 0 0 1 0 0  20
 * 0 0 0 0 0 0 1 0  40
 * 0 0 0 0 0 0 0 1  80
 * 0 0 0 0 0 0 0 0  00
 *
 * 0x0204081020408000ull
 * S_D6 -> (5 + 3) = 8
 */
const uint64_t antidiagonal_masks[15] = {
	/* Indice = rank + file */
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
