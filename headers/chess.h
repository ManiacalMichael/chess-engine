/*
 * * * chess.h
 * Representation for the game of chess e.g. boards, pieces, moves, etc.
 */

#ifndef INCLUDE_CHESS_H
#define INCLUDE_CHESS_H

#include <stdint.h>



#define UNIVERSAL_SET 0xffffffffffffffffull
#define EMPTY_SET 0x0000000000000000ull
#define FILL_MULTIPLIER 0x0101010101010101ull
#define MAIN_DIAGONAL 0x8040201008040201ull
#define MAIN_ANTIDIAGONAL 0x0102040810204080ull



enum COLORS {
	WHITE,
	BLACK
};

enum PIECETYPES {
	PAWN = 1,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING
};

enum PIECES {
	NO_PIECE,
	WHITE_PAWN = 2, BLACK_PAWN,
	WHITE_KNIGHT, BLACK_KNIGHT,
	WHITE_BISHOP, BLACK_BISHOP,
	WHITE_ROOK, BLACK_ROOK,
	WHITE_QUEEN, BLACK_QUEEN,
	WHITE_KING, BLACK_KING
};

enum CASTLETYPES {
	WK_CASTLE, WQ_CASTLE, BK_CASTLE, BQ_CASTLE
};

/*
 * Little Endian Rank File numbering
 */
enum SQUARES {
	S_A1, S_B1, S_C1, S_D1, S_E1, S_F1, S_G1, S_H1,
	S_A2, S_B2, S_C2, S_D2, S_E2, S_F2, S_G2, S_H2,
	S_A3, S_B3, S_C3, S_D3, S_E3, S_F3, S_G3, S_H3,
	S_A4, S_B4, S_C4, S_D4, S_E4, S_F4, S_G4, S_H4,
	S_A5, S_B5, S_C5, S_D5, S_E5, S_F5, S_G5, S_H5,
	S_A6, S_B6, S_C6, S_D6, S_E6, S_F6, S_G6, S_H6,
	S_A7, S_B7, S_C7, S_D7, S_E7, S_F7, S_G7, S_H7,
	S_A8, S_B8, S_C8, S_D8, S_E8, S_F8, S_G8, S_H8
};

enum RANKS {
	RANK_1,
	RANK_2,
	RANK_3,
	RANK_4,
	RANK_5,
	RANK_6,
	RANK_7,
	RANK_8
};

enum FILES {
	A_FILE,
	B_FILE,
	C_FILE,
	D_FILE,
	E_FILE,
	F_FILE,
	G_FILE,
	H_FILE
};


/*
 * struct position_t
 * 	pieces: Array of bitboards, index by COLORS and PIECETYPES
 * 	occupied:
 * 	empty: Bitboards, occupied and empty squares of both colors
 * 	captures: Tracks the most recently captured pieces by type,
 * 	          first element is the number of captured pieces
 * 	kingpos: position of kings on board, index by COLORS
 * 	ep_history: record of when and where e.p. squares were set
 * 	            [0][0] = number of e.p. squares recorded
 * 	            [0][x] = square where e.p. was set
 * 	            [1][x] = halfmove when e.p. was set
 * 	castles: record of when castle rights were lost
 * 	flags: Position flags, see #defines for more information
 * 	moves: Age of position, in halfmoves from start position
 * 	fiftymove: Number of halfmoves since an irreversible move took place
 */
struct position_t {
	uint64_t pieces[2][7];
	uint64_t occupied;
	uint64_t empty;
	unsigned char captures[31];
	unsigned char kingpos[2];
	unsigned short ep_history[2][17];
	unsigned short castles[4];
	uint16_t flags;
	int moves;
	int fiftymove;
};

/*
 * Position flags:
 * 012345	- En Passant square
 * 6		- En Passant available
 * 789a		- Castling status
 * bc		- Check status
 * d		- Game is over
 * e		- Game is drawn
 * f		- White to move
 */
#define EP_SQUARE 0x003fu
#define EN_PASSANT 0x0040u
#define WHITE_KINGSIDE_CASTLE 0x0080u
#define WHITE_QUEENSIDE_CASTLE 0x0100u
#define WHITE_BOTH_CASTLE 0x0180u
#define BLACK_KINGSIDE_CASTLE 0x0200u
#define BLACK_QUEENSIDE_CASTLE 0x0400u
#define BLACK_BOTH_CASTLE 0x0600u
#define BOTH_KINGSIDE_CASTLE 0x0280u
#define BOTH_QUEENSIDE_CASTLE 0x0500u
#define BOTH_BOTH_CASTLE 0x0780u
#define BLACK_CHECK 0x0800u
#define WHITE_CHECK 0x1000u
#define GAME_OVER 0x2000u
#define GAME_DRAWN 0x4000u
#define WHITE_TO_MOVE 0x8000u


extern const struct position_t START_POSITION;


/*
 * Move encoding:
 * 012345	- Start square 
 * 6789ab	- End square 
 * cdef		- Move flags:
 * 0: quiet move
 * 1: double pawn push
 * 2: queenside castle
 * 3: kingside castle
 * 4: capture move
 * 5: e.p. capture
 * 6:
 * 7: 
 * 8: promotion to knight
 * 9: promotion to bishop
 * 10: promotion to rook
 * 11: promotion to queen
 * 12: capture promotion to knight
 * 13: capture promotion to bishop
 * 14: capture promotion to rook
 * 15: capture promotion to queen
 */
#define START_SQUARE 0x03fu
#define END_SQUARE 0x0fc0u
#define DOUBLE_PAWN_PUSH 0x1000u
#define KINGSIDE_CASTLE 0x2000u
#define QUEENSIDE_CASTLE 0x3000u
#define CAPTURE_MOVE 0x4000u
#define EP_CAPTURE 0x5000u
#define KNIGHT_PROMOTION 0x8000u
#define BISHOP_PROMOTION 0x9000u
#define ROOK_PROMOTION 0xa000u
#define QUEEN_PROMOTION 0xb000u
#define KNIGHT_CAPTURE_PROMOTION 0xc000u
#define BISHOP_CAPTURE_PROMOTION 0xd000u
#define ROOK_CAPTURE_PROMOTION 0xe000u
#define QUEEN_CAPTURE_PROMOTION 0xf000u

#define ERROR_MOVE 0xffffu

/*
 * Maximum number of moves in a legal position (described below) 
 * A list of moves would have size MAX_MOVES + 1
 * (Actually there is no proof that 218 is the max number of moves, but no one
 *  has been able to find a legal position with at least 218 since 1968)
 */
#define MAX_MOVES 218
/* R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1 */



 #ifdef NO_SEARCH_LINKAGE

/*
 * uint16_t check_status() nop
 */
uint16_t check_status(const struct position_t pos) {;}

 #endif

/*
 * int popcount()
 * Returns the number of set bits in a 64-bit value
 * 	@bb - value to count population of
 */
int popcount(uint64_t bb);

/*
 * int ls1bindice()
 * Returns the indice of the ls1b in a 64-bit value
 * 	@bb - value to find ls1b of
 */
int ls1bindice(uint64_t bb);

/*
 * void push_ep()
 * Adds ep to position history
 * 	@posPtr - pointer to position to add ep to
 * 	@sq - square of ep to add
 */
void push_ep(struct position_t *posPtr, int sq);

/*
 * int pop_ep()
 * Pops ep from ep history if position age is the most recent ep position
 * 	@posPtr - pointer to position to use
 */
int pop_ep(struct position_t *posPtr);

/*
 * void push_capture()
 * Adds capture to capture history
 * 	@posPtr - pointer to position to use
 * 	@pt - piecetype of captured piece
 */
void push_capture(struct position_t *posPtr, int pt);

/*
 * int pop_capture()
 * Pops most recent capture from capture history
 * 	@posPtr - pointer to position to use
 */
int pop_capture(struct position_t *posPtr);

/*
 * void make_move()
 * Makes a move on a position
 * 	@posPtr - pointer to the position to make the move on
 * 	@mv - move to make
 */
void make_move(struct position_t *posPtr, uint16_t mv);

/*
 * void unmake_move()
 * Unmakes a move on a position
 * NOTE: fiftymove counter cannot be restored after it has been reset to 0
 * 	@posPtr - pointer to the position to unmake the move on
 * 	@mv - move to unmake
 */
void unmake_move(struct position_t *posPtr, uint16_t mv);


#endif
