#include <stdint.h>
#include "chess.h"
#include "utility.h"
#include "search.h"

/* 
 * Masks off the outer squares from the lookup key for sliding piece attacks 
 * and garbage in the last three bits from shifting down only 54 spaces
 */
#define OUTER_SQ_MASK 0x00000000000001f8ull

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

/*
 * Sliding attack lookups:
 *  Attacks along any rank, file, or diagonal can be visualized as an
 *  attack along the back rank:
 * 0 0 0 0 0 0 0 a
 * 0 0 0 0 0 0 0 b
 * 0 0 0 0 0 0 0 c
 * 0 0 0 0 0 0 0 d
 * 0 0 0 0 0 0 0 e
 * 0 0 0 0 0 0 0  <- Rook
 * 0 0 0 0 0 0 0 g
 * 0 0 0 0 0 0 0 h
 *
 * 	=
 * 	     |
 * a b c d e   g h
 * 0 0 0 0 0 0 0 0
 * 0 0 0 0 ...
 *
 * What squares on the back rank are attacked depends on where the rook is
 * on the rank and what the occupancy of that rank is:
 * Occupancy :
 * 0 1 0 0 0 1 0 0
 *           R
 *      =
 * 0 1 1 1 1 0 1 1	<- Attack set
 *
 * So instead of calculating the attack set every time, it can be
 * generated before hand as an array of 8bit values indexed by:
 * [position_on_rank][occupancy_of_rank]
 * 8 * 256 = 2048 bytes
 *
 * This can be further reduced because the occupancy of the 0 and 7 squares
 * is irrelevant; whether or not they are attacked depends purely on the
 * occupancy of the other six squares and the position of the attacker:
 *
 * 1 0 0 1 0 1 0 0
 *       R
 *    =
 * 1 1 1 0 1 1 0 0	<- Attack set
 *
 * AND
 *
 * 0 0 0 1 0 1 0 1
 *       R
 *    =
 * 1 1 1 0 1 1 0 0	<- Attack set
 *
 * By only considering the occupancy of the center squares we can reduce
 * the size of the lookup table fourfold:
 * [position_on_rank][occupancy_of_center]
 * [8][64]
 * 8 * 64 = 512 bytes
 */
/*
 * How to use lookup:
 * As an example, consider diagonal of the bishop on C1
 * 0 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0 1
 * 0 0 0 0 0 0 1 0
 * 0 0 0 0 0 1 0 0
 * 0 0 0 0 1 0 0 0
 * 0 0 0 1 0 0 0 0
 * 0 0 1 0 0 0 0 0
 * This can be turned into a lookup by first using fill multiplication:
 * 0x0000804020100804ull * 0x0101010101010101ull =
 * 0 0 1 1 1 1 1 1
 * 0 0 1 1 1 1 1 1
 * 0 0 1 1 1 1 1 1
 * 0 0 1 1 1 1 1 0
 * 0 0 1 1 1 1 0 0
 * 0 0 1 1 1 0 0 0
 * 0 0 1 1 0 0 0 0
 * 0 0 1 0 0 0 0 0
 * And shifting this down by 56:
 * 0 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0 0
 * 0 0 1 1 1 1 1 1
 *
 * After lookup the resulting value can be multiplied by 0x0101010101010101
 * and then & with the original diagonal will yield the attack set
 */
const uint8_t sliding_attack_lookups[512] = {
	/* Indice = (6bit key * 8) + file */
	/* 00000000 */
	0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f, 
	/* 01000000 */
	0x2, 0xfd, 0xfa, 0xf6, 0xee, 0xde, 0xbe, 0x7e, 
	/* 00100000 */
	0x6, 0x5, 0xfb, 0xf4, 0xec, 0xdc, 0xbc, 0x7c, 
	/* 01100000 */
	0x2, 0x5, 0xfa, 0xf4, 0xec, 0xdc, 0xbc, 0x7c, 
	/* 00010000 */
	0xe, 0xd, 0xb, 0xf7, 0xe8, 0xd8, 0xb8, 0x78, 
	/* 01010000 */
	0x2, 0xd, 0xa, 0xf6, 0xe8, 0xd8, 0xb8, 0x78, 
	/* 00110000 */
	0x6, 0x5, 0xb, 0xf4, 0xe8, 0xd8, 0xb8, 0x78, 
	/* 01110000 */
	0x2, 0x5, 0xa, 0xf4, 0xe8, 0xd8, 0xb8, 0x78, 
	/* 00001000 */
	0x1e, 0x1d, 0x1b, 0x17, 0xef, 0xd0, 0xb0, 0x70, 
	/* 01001000 */
	0x2, 0x1d, 0x1a, 0x16, 0xee, 0xd0, 0xb0, 0x70, 
	/* 00101000 */
	0x6, 0x5, 0x1b, 0x14, 0xec, 0xd0, 0xb0, 0x70, 
	/* 01101000 */
	0x2, 0x5, 0x1a, 0x14, 0xec, 0xd0, 0xb0, 0x70, 
	/* 00011000 */
	0xe, 0xd, 0xb, 0x17, 0xe8, 0xd0, 0xb0, 0x70, 
	/* 01011000 */
	0x2, 0xd, 0xa, 0x16, 0xe8, 0xd0, 0xb0, 0x70, 
	/* 00111000 */
	0x6, 0x5, 0xb, 0x14, 0xe8, 0xd0, 0xb0, 0x70, 
	/* 01111000 */
	0x2, 0x5, 0xa, 0x14, 0xe8, 0xd0, 0xb0, 0x70, 
	/* 00000100 */
	0x3e, 0x3d, 0x3b, 0x37, 0x2f, 0xdf, 0xa0, 0x60, 
	/* 01000100 */
	0x2, 0x3d, 0x3a, 0x36, 0x2e, 0xde, 0xa0, 0x60, 
	/* 00100100 */
	0x6, 0x5, 0x3b, 0x34, 0x2c, 0xdc, 0xa0, 0x60, 
	/* 01100100 */
	0x2, 0x5, 0x3a, 0x34, 0x2c, 0xdc, 0xa0, 0x60, 
	/* 00010100 */
	0xe, 0xd, 0xb, 0x37, 0x28, 0xd8, 0xa0, 0x60, 
	/* 01010100 */
	0x2, 0xd, 0xa, 0x36, 0x28, 0xd8, 0xa0, 0x60, 
	/* 00110100 */
	0x6, 0x5, 0xb, 0x34, 0x28, 0xd8, 0xa0, 0x60, 
	/* 01110100 */
	0x2, 0x5, 0xa, 0x34, 0x28, 0xd8, 0xa0, 0x60, 
	/* 00001100 */
	0x1e, 0x1d, 0x1b, 0x17, 0x2f, 0xd0, 0xa0, 0x60, 
	/* 01001100 */
	0x2, 0x1d, 0x1a, 0x16, 0x2e, 0xd0, 0xa0, 0x60, 
	/* 00101100 */
	0x6, 0x5, 0x1b, 0x14, 0x2c, 0xd0, 0xa0, 0x60, 
	/* 01101100 */
	0x2, 0x5, 0x1a, 0x14, 0x2c, 0xd0, 0xa0, 0x60, 
	/* 00011100 */
	0xe, 0xd, 0xb, 0x17, 0x28, 0xd0, 0xa0, 0x60, 
	/* 01011100 */
	0x2, 0xd, 0xa, 0x16, 0x28, 0xd0, 0xa0, 0x60, 
	/* 00111100 */
	0x6, 0x5, 0xb, 0x14, 0x28, 0xd0, 0xa0, 0x60, 
	/* 01111100 */
	0x2, 0x5, 0xa, 0x14, 0x28, 0xd0, 0xa0, 0x60, 
	/* 00000010 */
	0x7e, 0x7d, 0x7b, 0x77, 0x6f, 0x5f, 0xbf, 0x40, 
	/* 01000010 */
	0x2, 0x7d, 0x7a, 0x76, 0x6e, 0x5e, 0xbe, 0x40, 
	/* 00100010 */
	0x6, 0x5, 0x7b, 0x74, 0x6c, 0x5c, 0xbc, 0x40, 
	/* 01100010 */
	0x2, 0x5, 0x7a, 0x74, 0x6c, 0x5c, 0xbc, 0x40, 
	/* 00010010 */
	0xe, 0xd, 0xb, 0x77, 0x68, 0x58, 0xb8, 0x40, 
	/* 01010010 */
	0x2, 0xd, 0xa, 0x76, 0x68, 0x58, 0xb8, 0x40, 
	/* 00110010 */
	0x6, 0x5, 0xb, 0x74, 0x68, 0x58, 0xb8, 0x40, 
	/* 01110010 */
	0x2, 0x5, 0xa, 0x74, 0x68, 0x58, 0xb8, 0x40, 
	/* 00001010 */
	0x1e, 0x1d, 0x1b, 0x17, 0x6f, 0x50, 0xb0, 0x40, 
	/* 01001010 */
	0x2, 0x1d, 0x1a, 0x16, 0x6e, 0x50, 0xb0, 0x40, 
	/* 00101010 */
	0x6, 0x5, 0x1b, 0x14, 0x6c, 0x50, 0xb0, 0x40, 
	/* 01101010 */
	0x2, 0x5, 0x1a, 0x14, 0x6c, 0x50, 0xb0, 0x40, 
	/* 00011010 */
	0xe, 0xd, 0xb, 0x17, 0x68, 0x50, 0xb0, 0x40, 
	/* 01011010 */
	0x2, 0xd, 0xa, 0x16, 0x68, 0x50, 0xb0, 0x40, 
	/* 00111010 */
	0x6, 0x5, 0xb, 0x14, 0x68, 0x50, 0xb0, 0x40, 
	/* 01111010 */
	0x2, 0x5, 0xa, 0x14, 0x68, 0x50, 0xb0, 0x40, 
	/* 00000110 */
	0x3e, 0x3d, 0x3b, 0x37, 0x2f, 0x5f, 0xa0, 0x40, 
	/* 01000110 */
	0x2, 0x3d, 0x3a, 0x36, 0x2e, 0x5e, 0xa0, 0x40, 
	/* 00100110 */
	0x6, 0x5, 0x3b, 0x34, 0x2c, 0x5c, 0xa0, 0x40, 
	/* 01100110 */
	0x2, 0x5, 0x3a, 0x34, 0x2c, 0x5c, 0xa0, 0x40, 
	/* 00010110 */
	0xe, 0xd, 0xb, 0x37, 0x28, 0x58, 0xa0, 0x40, 
	/* 01010110 */
	0x2, 0xd, 0xa, 0x36, 0x28, 0x58, 0xa0, 0x40, 
	/* 00110110 */
	0x6, 0x5, 0xb, 0x34, 0x28, 0x58, 0xa0, 0x40, 
	/* 01110110 */
	0x2, 0x5, 0xa, 0x34, 0x28, 0x58, 0xa0, 0x40, 
	/* 00001110 */
	0x1e, 0x1d, 0x1b, 0x17, 0x2f, 0x50, 0xa0, 0x40, 
	/* 01001110 */
	0x2, 0x1d, 0x1a, 0x16, 0x2e, 0x50, 0xa0, 0x40, 
	/* 00101110 */
	0x6, 0x5, 0x1b, 0x14, 0x2c, 0x50, 0xa0, 0x40, 
	/* 01101110 */
	0x2, 0x5, 0x1a, 0x14, 0x2c, 0x50, 0xa0, 0x40, 
	/* 00011110 */
	0xe, 0xd, 0xb, 0x17, 0x28, 0x50, 0xa0, 0x40, 
	/* 01011110 */
	0x2, 0xd, 0xa, 0x16, 0x28, 0x50, 0xa0, 0x40, 
	/* 00111110 */
	0x6, 0x5, 0xb, 0x14, 0x28, 0x50, 0xa0, 0x40, 
	/* 01111110 */
	0x2, 0x5, 0xa, 0x14, 0x28, 0x50, 0xa0, 0x40
};

/*
 * Attack sets for kings, by square
 */
const uint64_t king_attack_lookups[64] = {
        0x302ull, 
        0x705ull, 
        0xe0aull, 
        0x1c14ull, 
        0x3828ull, 
        0x7050ull, 
        0xe0a0ull, 
        0xc040ull, 
        0x30203ull, 
        0x70507ull, 
        0xe0a0eull, 
        0x1c141cull, 
        0x382838ull, 
        0x705070ull, 
        0xe0a0e0ull, 
        0xc040c0ull, 
        0x3020300ull, 
        0x7050700ull, 
        0xe0a0e00ull, 
        0x1c141c00ull, 
        0x38283800ull, 
        0x70507000ull, 
        0xe0a0e000ull, 
        0xc040c000ull, 
        0x302030000ull, 
        0x705070000ull, 
        0xe0a0e0000ull, 
        0x1c141c0000ull, 
        0x3828380000ull, 
        0x7050700000ull, 
        0xe0a0e00000ull, 
        0xc040c00000ull, 
        0x30203000000ull, 
        0x70507000000ull, 
        0xe0a0e000000ull, 
        0x1c141c000000ull, 
        0x382838000000ull, 
        0x705070000000ull, 
        0xe0a0e0000000ull, 
        0xc040c0000000ull, 
        0x3020300000000ull, 
        0x7050700000000ull, 
        0xe0a0e00000000ull, 
        0x1c141c00000000ull, 
        0x38283800000000ull, 
        0x70507000000000ull, 
        0xe0a0e000000000ull, 
        0xc040c000000000ull, 
        0x302030000000000ull, 
        0x705070000000000ull, 
        0xe0a0e0000000000ull, 
        0x1c141c0000000000ull, 
        0x3828380000000000ull, 
        0x7050700000000000ull, 
        0xe0a0e00000000000ull, 
        0xc040c00000000000ull, 
        0x203000000000000ull, 
        0x507000000000000ull, 
        0xa0e000000000000ull, 
        0x141c000000000000ull, 
        0x2838000000000000ull, 
        0x5070000000000000ull, 
        0xa0e0000000000000ull, 
        0x40c0000000000000ull
};

/*
 * Attack sets for knights, by square
 */
const uint64_t knight_attack_lookups[64] = {
	0x20400ull,
	0x50800ull,
	0xa1100ull,
	0x142200ull,
	0x284400ull,
	0x508800ull,
	0xa01000ull,
	0x402000ull,
	0x2040004ull,
	0x5080008ull,
	0xa110011ull,
	0x14220022ull,
	0x28440044ull,
	0x50880088ull,
	0xa0100010ull,
	0x40200020ull,
	0x204000402ull,
	0x508000805ull,
	0xa1100110aull,
	0x1422002214ull,
	0x2844004428ull,
	0x5088008850ull,
	0xa0100010a0ull,
	0x4020002040ull,
	0x20400040200ull,
	0x50800080500ull,
	0xa1100110a00ull,
	0x142200221400ull,
	0x284400442800ull,
	0x508800885000ull,
	0xa0100010a000ull,
	0x402000204000ull,
	0x2040004020000ull,
	0x5080008050000ull,
	0xa1100110a0000ull,
	0x14220022140000ull,
	0x28440044280000ull,
	0x50880088500000ull,
	0xa0100010a00000ull,
	0x40200020400000ull,
	0x204000402000000ull,
	0x508000805000000ull,
	0xa1100110a000000ull,
	0x1422002214000000ull,
	0x2844004428000000ull,
	0x5088008850000000ull,
	0xa0100010a0000000ull,
	0x4020002040000000ull,
	0x400040200000000ull,
	0x800080500000000ull,
	0x1100110a00000000ull,
	0x2200221400000000ull,
	0x4400442800000000ull,
	0x8800885000000000ull,
	0x100010a000000000ull,
	0x2000204000000000ull,
	0x4020000000000ull,
	0x8050000000000ull,
	0x110a0000000000ull,
	0x22140000000000ull,
	0x44280000000000ull,
	0x88500000000000ull,
	0x10a00000000000ull,
	0x20400000000000ull
};

/* table for two square jumps for pawns on starting rank */
const uint64_t pawn_twosquare[8] = {
	0x1010000ull,
	0x2020000ull,
	0x4040000ull,
	0x8080000ull,
	0x10100000ull,
	0x20200000ull,
	0x40400000ull,
	0x80800000ull
};

/*
 * Movement sets for pawns, by square 
 */
const uint64_t pawn_movement[64] = {
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x10000ull,
	0x20000ull,
	0x40000ull,
	0x80000ull,
	0x100000ull,
	0x200000ull,
	0x400000ull,
	0x800000ull,
	0x1000000ull,
	0x2000000ull,
	0x4000000ull,
	0x8000000ull,
	0x10000000ull,
	0x20000000ull,
	0x40000000ull,
	0x80000000ull,
	0x100000000ull,
	0x200000000ull,
	0x400000000ull,
	0x800000000ull,
	0x1000000000ull,
	0x2000000000ull,
	0x4000000000ull,
	0x8000000000ull,
	0x10000000000ull,
	0x20000000000ull,
	0x40000000000ull,
	0x80000000000ull,
	0x100000000000ull,
	0x200000000000ull,
	0x400000000000ull,
	0x800000000000ull,
	0x1000000000000ull,
	0x2000000000000ull,
	0x4000000000000ull,
	0x8000000000000ull,
	0x10000000000000ull,
	0x20000000000000ull,
	0x40000000000000ull,
	0x80000000000000ull,
	0x100000000000000ull,
	0x200000000000000ull,
	0x400000000000000ull,
	0x800000000000000ull,
	0x1000000000000000ull,
	0x2000000000000000ull,
	0x4000000000000000ull,
	0x8000000000000000ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull
};

/*
 * Pawn attacks, by square
 */
const uint64_t pawn_captures[64] = {
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x20000ull,
	0x50000ull,
	0xa0000ull,
	0x140000ull,
	0x280000ull,
	0x500000ull,
	0xa00000ull,
	0x400000ull,
	0x2000000ull,
	0x5000000ull,
	0xa000000ull,
	0x14000000ull,
	0x28000000ull,
	0x50000000ull,
	0xa0000000ull,
	0x40000000ull,
	0x200000000ull,
	0x500000000ull,
	0xa00000000ull,
	0x1400000000ull,
	0x2800000000ull,
	0x5000000000ull,
	0xa000000000ull,
	0x4000000000ull,
	0x20000000000ull,
	0x50000000000ull,
	0xa0000000000ull,
	0x140000000000ull,
	0x280000000000ull,
	0x500000000000ull,
	0xa00000000000ull,
	0x400000000000ull,
	0x2000000000000ull,
	0x5000000000000ull,
	0xa000000000000ull,
	0x14000000000000ull,
	0x28000000000000ull,
	0x50000000000000ull,
	0xa0000000000000ull,
	0x40000000000000ull,
	0x200000000000000ull,
	0x500000000000000ull,
	0xa00000000000000ull,
	0x1400000000000000ull,
	0x2800000000000000ull,
	0x5000000000000000ull,
	0xa000000000000000ull,
	0x4000000000000000ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull,
	0x0ull
};


/* Returns attack set for pawns */
uint64_t pawn_moves(uint64_t enemy, uint64_t empty, int color, int sq)
{
	uint64_t r = 0ull;
	if (color) {
		/* lookup tables are written for white pawns, so shift */
		if ((sq / 8) == 6) {
			if (!((pawn_twosquare[sq % 8] << 16) & ~empty))
				r |= pawn_movement[sq - 24];
		}
		r |= (pawn_movement[sq] >> 16) & empty;
		r |= (pawn_captures[sq] >> 16) & enemy;
	} else {
		if ((sq / 8) == 1) {
			if (!(pawn_twosquare[sq % 8] & ~empty))
				r |= pawn_movement[sq + 8];
		}
		r |= pawn_movement[sq] & empty;
		r |= pawn_captures[sq] & enemy;
	}
	return r;
}

/* Returns attack set for bishops */
uint64_t bishop_moves(uint64_t occupied, int rank, int file)
{
	uint64_t r = 0ull;
	uint64_t diagonal = diagonal_masks[(7 + rank) - file];
	uint64_t antidiagonal = antidiagonal_masks[rank + file];
	/* 
	 * Actually, shift down by 56 gives the key, but the key is then
	 * multiplied by 8, which is the same as shifting back up by 3, and the
	 * key is shifted down by one to ignore the occupancy of the edge square
	 */
	uint64_t key = (((diagonal & occupied) * FILL_MULTIPLIER) >> 54) & OUTER_SQ_MASK;
	r |= (sliding_attack_lookups[key + file] * FILL_MULTIPLIER) & diagonal;
	key = (((antidiagonal & occupied) * FILL_MULTIPLIER) >> 54) & OUTER_SQ_MASK;
	r |= (sliding_attack_lookups[key + file] * FILL_MULTIPLIER) & antidiagonal;
	return r;
}

/* Returns attack set for rook */
uint64_t rook_moves(uint64_t occupied, int rank, int file)
{
	uint64_t r = 0ull;
	uint64_t key = (occupied & rank_masks[rank]) >> (8 * rank);
	key <<= 2;
	key &= OUTER_SQ_MASK;
	/* processor will do 8-bit shift if no cast */
	r |= (uint64_t)sliding_attack_lookups[key + file] << (8 * rank);
	key = ((occupied & file_masks[file]) >> file) * MAIN_DIAGONAL;
	key >>= 54;
	key &= OUTER_SQ_MASK;
	/* but does 64-bit multiply here because of the 64-bit immediate */
	key = sliding_attack_lookups[key + rank] * MAIN_ANTIDIAGONAL;
	r |= (key >> (7 - file)) & file_masks[file];
	return r;
}

/* Returns attack set for queen */
uint64_t queen_moves(uint64_t occupied, int rank, int file)
{
	uint64_t r = 0ull;
	r |= bishop_moves(occupied, rank, file);
	r |= rook_moves(occupied, rank, file);
	return r;
}

/* 
 * Returns check status for board.
 * Note: although only one player can be in check at any given moment,
 * this function will be called in movegen and will probably be given boards
 * after illegal moves which cause both players to be in check
 */
uint16_t check_status(struct board_t *boardPtr)
{
	int bking, wking, brank, bfile, wrank, wfile;
	uint64_t occupied = boardPtr->occupied;
	uint64_t blackp = boardPtr->black_pieces;
	uint64_t whitep = ~blackp;
	uint16_t ret = 0;
	bking = bitindice(blackp & boardPtr->kings);
	wking = bitindice(boardPtr->kings ^ (1ull << bking));
	brank = bking / 8;
	bfile = bking % 8;
	wrank = wking / 8;
	wfile = wking % 8;
	if (rook_moves(occupied, wrank, wfile) & (blackp & boardPtr->rooks) & (
				blackp & boardPtr->queens)) {
		ret |= WHITE_CHECK;
		goto test_black;
	} else if (bishop_moves(occupied, wrank, wfile) & (blackp & 
				boardPtr->bishops) & (blackp & boardPtr->queens)) {
		ret |= WHITE_CHECK;
		goto test_black;
	} else if (knight_attack_lookups[wking] & (blackp & boardPtr->knights)) {
		ret |= WHITE_CHECK;
		goto test_black;
	} else if (pawn_captures[wking] & (blackp & boardPtr->pawns)) {
		ret |= WHITE_CHECK;
	}
	test_black:
	if (rook_moves(occupied, brank, bfile) & (whitep & boardPtr->rooks) &
			(whitep & boardPtr->queens)) {
		ret |= BLACK_CHECK;
		return ret;
	} else if (bishop_moves(occupied, brank, bfile) & (whitep & 
				boardPtr->bishops) & (whitep & boardPtr->queens)) {
		ret |= BLACK_CHECK;
		return ret;
	} else if (knight_attack_lookups[bking] & (whitep & boardPtr->knights)) {
		ret |= BLACK_CHECK;
		return ret;
	} else if (pawn_captures[bking] & (whitep & boardPtr->pawns)) {
		ret |= BLACK_CHECK;
	}
	return ret;
}

struct movenode_t *piece_moves(struct board_t *boardPtr, uint64_t friendly,
		int color, int mode, int ep)
{
	struct movelist_t ls = { NULL, 0 };
	int i;
	uint64_t unfriendly = ~friendly;
	uint64_t pieces;
	uint64_t attk;
	uint64_t occupied = boardPtr->occupied;
	uint64_t empty = ~occupied;
	uint64_t enemy = occupied ^ friendly;
	switch(mode) {
		case 1:
			pieces = boardPtr->pawns;
			if (ep != -1) {
				enemy |= 1ull << ep;
				boardPtr->pawns |= 1ull << ep;
			}
			break;
		case 2:
			pieces = boardPtr->knights;
			break;
		case 3:
			pieces = boardPtr->bishops;
			break;
		case 4:
			pieces = boardPtr->rooks;
			break;
		case 5:
			pieces = boardPtr->queens;
			break;
		case 6:
			pieces = boardPtr->kings;
			break;
	}
	while (pieces != 0x0000000000000000ull) {
		i = bitindice(pieces ^ (pieces & (pieces - 1)));
		switch(mode) {
			case 1:
				if ((1ull << i) & friendly)
					attk = pawn_moves(enemy, empty, color, i);
				else
					attk = pawn_moves(friendly, empty, 1-color, i);
				break;
			case 2:
				attk = knight_attack_lookups[i];
				break;
			case 3:
				attk = bishop_moves(occupied, i / 8, i % 8);
				break;
			case 4:
				attk = rook_moves(occupied, i / 8, i % 8);
				break;
			case 5:
				attk = queen_moves(occupied, i / 8, i % 8);
				break;
			case 6:
				attk = king_attack_lookups[i];
				break;
		}
		if ((1ull << i) & friendly) {
			attk &= unfriendly;
			cat_lists(&ls, serialize_moves(i, attk, boardPtr));
			color ? (boardPtr->black_attacks |= attk) :
				(boardPtr->white_attacks |= attk);
		} else {
			attk &= friendly & empty;
			color ? (boardPtr->white_attacks |= attk) :
				(boardPtr->black_attacks |= attk);
		}
		pieces &= pieces - 1;
	}
	if (ep != -1)
		boardPtr->pawns ^= (1ull << ep);
	return (ls.root);
}

struct movenode_t *castle_moves(struct position_t *posPtr)
{
	struct board_t testboard = posPtr->board;
	struct movenode_t *r = NULL;
	uint64_t attk = 0ull;
	uint64_t empty = ~testboard.occupied;
	int color = (posPtr->flags & WHITE_TO_MOVE) ? 0 : 1;
	uint16_t friendly_check = color ? BLACK_CHECK : WHITE_CHECK;
	int kingpos = color ? S_E8 : S_E1;
	if (posPtr->flags & BOTH_KINGSIDE_CASTLE) {
		if (empty & (3ull << (kingpos + 1))) {
			testboard.kings ^= 3ull << (kingpos + 1);
			testboard.occupied ^= 3ull << (kingpos + 1);
			if (!(check_status(&testboard) & friendly_check))
				attk |= 1ull << (kingpos + 2);
			testboard.kings ^= 3ull << (kingpos + 1);
			testboard.occupied ^= 3ull << (kingpos + 1);
		}
		if (empty & (7ull << (kingpos - 1))) {
			testboard.kings ^= 3ull << (kingpos - 1);
			testboard.occupied ^= 3ull << (kingpos - 1);
			if (!(check_status(&testboard) & friendly_check))
				attk |= 1ull << (kingpos + 2);
			testboard.kings ^= 3ull << (kingpos - 1);
			testboard.occupied ^= 3ull << (kingpos - 1);
		}
	}
	r = color ? (serialize_moves(S_E8, attk, &testboard)) :
		(serialize_moves(S_E1, attk, &testboard));
	return r;
}

struct movelist_t generate_moves(struct position_t *posPtr)
{
	struct movelist_t ls = { NULL, 0 };
	struct movenode_t *p;
	struct position_t testpos = *posPtr;
	int color = (posPtr->flags & WHITE_TO_MOVE) ? 0 : 1;
	int ep = (posPtr->flags & EN_PASSANT) ? ((posPtr->flags & EP_SQUARE) >> 1) : -1;
	uint64_t friendly;
	uint16_t friendly_check = color ? BLACK_CHECK : WHITE_CHECK;
	uint16_t friendly_castle = color ? BLACK_BOTH_CASTLE : WHITE_BOTH_CASTLE;
	uint16_t enemy_check = color ? WHITE_CHECK : BLACK_CHECK;
	posPtr->board.black_attacks = 0;
	posPtr->board.white_attacks = 0;
	friendly = color ? (posPtr->board.black_pieces) : (posPtr->board.occupied 
			^ posPtr->board.black_pieces);
	for (int i = 1; i < 7; i++)
		cat_lists(&ls, piece_moves(&posPtr->board, friendly, color, i, ep));
	if (posPtr->flags & friendly_castle)
		cat_lists(&ls, castle_moves(posPtr));
	p = ls.root;
	if (p == NULL) {
		posPtr->flags |= GAME_OVER;
		return ls;
	}
	while (p->nxt != NULL) {
		make_move(&testpos, p->nxt->move);
		testpos.flags |= check_status(&testpos.board);
		if (testpos.flags & friendly_check) {
			p->nxt = remove_node(p->nxt);
			p = p->nxt;
			continue;
		} else if (testpos.flags & enemy_check) {
			p->move |= CAUSES_CHECK;
		}
		if (ep != -1) {
			if ((p->move & CAPTURES_PAWN) & (((p->move & EP_SQUARE)
							>> 1) == ep))
				p->move |= EP_CAPTURE;
		}
		testpos = *posPtr;
		ls.nodes++;
		p = p->nxt;
	}
	/* test root node of list */
	if (ls.nodes != 0) {
		p = ls.root;
		make_move(&testpos, p->move);
		testpos.flags |= check_status(&testpos.board);
		if (testpos.flags & friendly_check) {
			ls.root = p->nxt;
		} else if (testpos.flags & enemy_check) {
			p->move |= CAUSES_CHECK;
		}
		if (ep != -1) {
			if ((p->move & CAPTURES_PAWN) & (((p->move & EP_SQUARE)
							>> 1) == ep))
				p->move |= EP_CAPTURE;
		}
	} else {
		posPtr->flags |= GAME_OVER;
	}
	return ls;
}
