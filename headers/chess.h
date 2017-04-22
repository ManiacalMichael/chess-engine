#ifndef INCLUDE_CHESS_H
#define INCLUDE_CHESS_H

#include <stdint.h>
#include <limits.h>

#define UNIVERSAL_SET 0xffffffffffffffffull
#define EMPTY_SET 0x0000000000000000ull

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

enum PIECETYPES {
	PAWN = 1,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING
};

enum PIECES {
	WHITE, BLACK,
	WHITE_PAWN, BLACK_PAWN,
	WHITE_KNIGHT, BLACK_KNIGHT,
	WHITE_BISHOP, BLACK_BISHOP,
	WHITE_ROOK, BLACK_ROOK,
	WHITE_QUEEN, BLACK_QUEEN,
	WHITE_KING, BLACK_KING,
	OCCUPIED, EMPTY
};

enum CASTLETYPES {
	WS_CASTLE, WL_CASTLE, BS_CASTLE, BL_CASTLE
};

#define ISEMPTY(val) (val == INT_MAX)

#define SETEMPTY(val) (val = INT_MAX)

typedef struct {
	uint64_t bboards[16];
	int capturestack[30];
	int capturetop;
	int epstack[2][16];
	int eptop;
	int castles[4];
	int fiftymovestack[126];
	int fiftymovetop;
	uint16_t flags;
	int halfmove;
	int fiftymove;
} position_t;

#define EP_AVAILABLE 0x0001u
#define EP_SQUARE 0x007eu
#define WHITE_SHORT_CASTLE 0x0080u
#define WHITE_LONG_CASTLE 0x0100u
#define WHITE_BOTH_CASTLE 0x0180u
#define BLACK_SHORT_CASTLE 0x0200u
#define BLACK_LONG_CASTLE 0x0400u
#define BLACK_BOTH_CASTLE 0x0600u
#define BOTH_BOTH_CASTLE 0x0780u
#define BLACK_CHECK 0x0800u
#define WHITE_CHECK 0x1000u
#define GAME_OVER 0x2000u
#define GAME_DRAWN 0x4000u
#define WHITE_TO_MOVE 0x8000u

#define START_SQUARE 0x003fu
#define END_SQUARE 0x0fc0u
#define MOVE_FLAGS 0xf000u
#define PAWN_MOVE 0x1000u
#define CAPTURE_MOVE 0x3000u
#define SHORT_CASTLE 0x4000u
#define DOUBLE_PAWN_PUSH 0x5000u
#define LONG_CASTLE 0x6000u
#define PROMO_TO_KNIGHT 0x8000u
#define PROMO_TO_BISHOP 0x9000u
#define PROMO_TO_ROOK 0xa000u
#define PROMO_TO_QUEEN 0xb000u
#define CAPTURE_PROMO_TO_KNIGHT 0xc000u
#define CAPTURE_PROMO_TO_BISHOP 0xd000u
#define CAPTURE_PROMO_TO_ROOK 0xe000u
#define CAPTURE_PROO_TO_QUEEN 0xf000u
#define IRREVERSIBLE_MOVE 0x9000u

typedef struct {
	int top;
	uint16_t list[218];
	int priority[218];
} movels_t;

extern const uint64_t file_masks[8];

extern const uint64_t rank_masks[8];

extern const uint64_t rook_masks[64];

extern const uint64_t bishop_masks[64];

extern const uint64_t rook_magics[64];

extern const uint64_t bishop_magics[64];

extern const uint64_t attack_table[82688];

extern const int rook_index[64];

extern const int bishop_index[64];

extern const uint64_t king_attack[64];

extern const uint64_t knight_attack[64];

extern const uint64_t pawn_doublepush[2][8];

extern const uint64_t pawn_movement[2][64];

extern const uint64_t pawn_attack[2][64];

extern const position_t START_POSITION;


/* Returns the number of set bits in a bitboard */
int popcount(uint64_t);

/* Resets the LS1B of a bitboard and returns the incice of that bit */
int popfirst(uint64_t*);

/* Returns the LS1B of a bitboard */
int ls1bindice(uint64_t);


/* Pushes a piece onto a position's capture stack */
void pushcapture(position_t*, enum PIECES);

/* Pops a piece from a position's capture stack */
enum PIECES popcapture(position_t*);

/* Pushes an square onto a position's e.p. stack */
void pushep(position_t*, enum SQUARES);

/* Pops a square from a position's e.p. stack */
enum SQUARES popep(position_t*);

/* Pushes a counter onto a position's fiftymove stack */
void pushfifty(position_t*, int);

/* Pops a counter from a position's fiftymove stack */
int popfifty(position_t*);

/* Makes a move on a position */
void makemove(position_t*, uint16_t);

/* Unmakes a move on a position */
void unmakemove(position_t*, uint16_t);

/* Returns the attack set of a pawn
 * Arguments: <enemy pieces>, <empty squares>, <color of pawn>, <pawns position>
 */
uint64_t pawn_moves(uint64_t, uint64_t, enum PIECES, enum SQUARES);

/* Returns the attack set of a bishop */
uint64_t bishop_moves(uint64_t, enum SQUARES);

/* Returns the attack set of a rook */
uint64_t rook_moves(uint64_t, enum SQUARES);

/* Returns the attack set of a queen */
uint64_t queen_moves(uint64_t, enum SQUARES);

/* Returns the check status of a position */
uint16_t check_status(const position_t);

/* Returns an attack set for long castles */
uint64_t castle_long(const position_t);

/* Returns an attack set for short castles */
uint64_t castle_short(const position_t);

/* Serializes moves from an attack set to 16-bit moves and puts them in a movelist
 * Arguments: <attack set>, <piece to move>, <start square>, <movelist>
 */
void addmoves(uint64_t, enum PIECES, enum SQUARES, movels_t*);

/* Generates psuedolegal moves for a position and stores them in a movelist */
void movegen(position_t, movels_t*);

/* Initializes attack_table[] */
void init_attack_table();



#endif
