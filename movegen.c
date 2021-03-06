#include <assert.h>
#include <stdint.h>
#include "headers/chess.h"
#include "headers/search.h"

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
	0x00000000ff000000ull,
	0x000000ff00000000ull,
	0x0000ff0000000000ull,
	0x00ff000000000000ull,
	0xff00000000000000ull
};

const uint64_t diagonal_masks[15] = {
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

const uint64_t antidiagonal_masks[15] = {
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

const uint64_t pawn_doublepush[2][8] = {
	{
		0x1010000ull,
		0x2020000ull,
		0x4040000ull,
		0x8080000ull,
		0x10100000ull,
		0x20200000ull,
		0x40400000ull,
		0x80800000ull
	},
	{
		0x10100000000ull,
		0x20200000000ull,
		0x40400000000ull,
		0x80800000000ull,
		0x101000000000ull,
		0x202000000000ull,
		0x404000000000ull,
		0x808000000000ull
	}
};

const uint64_t pawn_movement[2][64] = {
	{
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
	},
	{
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x1ull,
		0x2ull,
		0x4ull,
		0x8ull,
		0x10ull,
		0x20ull,
		0x40ull,
		0x80ull,
		0x100ull,
		0x200ull,
		0x400ull,
		0x800ull,
		0x1000ull,
		0x2000ull,
		0x4000ull,
		0x8000ull,
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
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull
	}
};

const uint64_t pawn_attacks[2][64] = {
	{
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
	},
	{
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x2ull,
		0x5ull,
		0xaull,
		0x14ull,
		0x28ull,
		0x50ull,
		0xa0ull,
		0x40ull,
		0x200ull,
		0x500ull,
		0xa00ull,
		0x1400ull,
		0x2800ull,
		0x5000ull,
		0xa000ull,
		0x4000ull,
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
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull,
		0x0ull
	}
};


uint64_t pawn_moves(uint64_t enemy, uint64_t empty, int color, int sq)
{
	uint64_t r = 0ull;
	if ((sq / 8) == (color ? 6 : 1)) {
		if (!(pawn_doublepush[color][sq % 8] & ~empty))
			r |= pawn_movement[color][color ? (sq - 8) : (sq + 8)];
	}
	r |= pawn_movement[color][sq] & empty;
	r |= pawn_attacks[color][sq] & enemy;
	return r;
}

uint64_t bishop_moves(uint64_t occupied, int rank, int file)
{
	uint64_t r = 0ull;
	uint64_t diagonal = diagonal_masks[(7 + rank) - file];
	uint64_t antidiagonal = antidiagonal_masks[rank + file];
	/* 
	 * Actually, shift down by 56 gives the key, but the key is then
	 * multiplied by 8, and shifted down by one to ignore the occupancy 
	 * of the edge square
	 */
	uint64_t key = (((diagonal & occupied) * FILL_MULTIPLIER) >> 54) & OUTER_SQ_MASK;
	r |= (sliding_attack_lookups[key + file] * FILL_MULTIPLIER) & diagonal;
	key = (((antidiagonal & occupied) * FILL_MULTIPLIER) >> 54) & OUTER_SQ_MASK;
	r |= (sliding_attack_lookups[key + file] * FILL_MULTIPLIER) & antidiagonal;
	return r;
}

uint64_t rook_moves(uint64_t occupied, int rank, int file)
{
	uint64_t r = 0ull;
	uint64_t key = (occupied & rank_masks[rank]) >> (8 * rank);
	key <<= 2;
	key &= OUTER_SQ_MASK;
	/* processor will do 8-bit shift if no cast */
	r |= (uint64_t)sliding_attack_lookups[key + file] << (8 * rank);
	key = ((occupied & file_masks[file]) >> file) * MAIN_ANTIDIAGONAL;
	key >>= 54;
	key &= OUTER_SQ_MASK;
	key = sliding_attack_lookups[key + rank] * MAIN_ANTIDIAGONAL;
	r |= (key >> (7 - file)) & file_masks[file];
	return r;
}

uint64_t queen_moves(uint64_t occupied, int rank, int file)
{
	uint64_t r = 0ull;
	r |= bishop_moves(occupied, rank, file);
	r |= rook_moves(occupied, rank, file);
	return r;
}

uint16_t check_status(const struct position_t pos)
{
	uint64_t occupied = pos.occupied;
	uint16_t ret = 0;
	int brank = pos.kingpos[BLACK] / 8;
	int bfile = pos.kingpos[BLACK] % 8;
	int wrank = pos.kingpos[WHITE] / 8;
	int wfile = pos.kingpos[WHITE] % 8;
	if (rook_moves(occupied, wrank, wfile) & (pos.pieces[BLACK][ROOK]
				| pos.pieces[BLACK][QUEEN])) {
		ret |= WHITE_CHECK;
		goto test_black;
	} else if (bishop_moves(occupied, wrank, wfile) & (pos.pieces[BLACK][BISHOP]
				| pos.pieces[BLACK][QUEEN])) {
		ret |= WHITE_CHECK;
		goto test_black;
	} else if (knight_attack_lookups[pos.kingpos[WHITE]] &
				pos.pieces[BLACK][KNIGHT]) {
		ret |= WHITE_CHECK;
		goto test_black;
	} else if (pawn_attacks[WHITE][pos.kingpos[WHITE]]
				& pos.pieces[BLACK][PAWN]) {
		ret |= WHITE_CHECK;
		goto test_black;
	} else if (king_attack_lookups[pos.kingpos[WHITE]]
				& pos.pieces[BLACK][KING]) {
		ret |= WHITE_CHECK;
	}
test_black:
	if (rook_moves(occupied, brank, bfile) & (pos.pieces[WHITE][ROOK]
				| pos.pieces[WHITE][QUEEN])) {
		ret |= BLACK_CHECK;
		return ret;
	} else if (bishop_moves(occupied, brank, bfile) & (pos.pieces[WHITE][BISHOP]
				| pos.pieces[WHITE][QUEEN])) {
		ret |= BLACK_CHECK;
		return ret;
	} else if (knight_attack_lookups[pos.kingpos[BLACK]]
				& pos.pieces[WHITE][KNIGHT]) {
		ret |= BLACK_CHECK;
		return ret;
	} else if (pawn_attacks[BLACK][pos.kingpos[BLACK]]
				& pos.pieces[WHITE][PAWN]) {
		ret |= BLACK_CHECK;
		return ret;
	} else if (king_attack_lookups[pos.kingpos[BLACK]]
				& pos.pieces[WHITE][KING]) {
		ret |= BLACK_CHECK;
	}
	return ret;
}

uint64_t castle_moves(struct position_t pos)
{
	uint64_t attk = 0ull;
	uint64_t empty = pos.empty;
	int color = (pos.flags & WHITE_TO_MOVE) ? WHITE : BLACK;
	uint16_t friendly_check = color ? BLACK_CHECK : WHITE_CHECK;
	int kingpos = color ? S_E8 : S_E1;
	uint16_t kflag = color ? BLACK_KINGSIDE_CASTLE : WHITE_KINGSIDE_CASTLE;
	uint16_t qflag = color ? BLACK_QUEENSIDE_CASTLE : WHITE_QUEENSIDE_CASTLE;
	if (pos.flags & friendly_check)
		return 0;
	if ((pos.flags & kflag) && !(pos.occupied &
				(6ull << kingpos))) {
		make_move(&pos, color ? 0x0f7c : 0x0144);
		if (!(pos.flags & friendly_check))
			attk |= 1ull << (color ? S_G8 : S_G1 );
		unmake_move(&pos, color ? 0x0f7c : 0x0144);
	}
	if ((pos.flags & qflag) && !(pos.occupied &
				(14ull << (color * S_A8)))) {
		make_move(&pos, color ? 0x0efc: 0x00c4);
		if (!(pos.flags & friendly_check))
			attk |= 1ull << (color ? S_C8 : S_C1 );
	}
	return attk;
}

void serialize_moves(int start, uint64_t attk, const struct position_t pos,
		uint16_t *lsPtr)
{
	int color = (pos.flags & WHITE_TO_MOVE) ? WHITE : BLACK;
	int length = lsPtr[0];
	uint64_t startbb = 1ull << start;
	int end;
	uint64_t endbb;
	int pt;
	uint16_t tmp;
	assert(startbb & pos.pieces[color][0]);
	if (attk == 0)
		return;
	for (int i = PAWN; i <= KING; ++i) {
		if (pos.pieces[color][i] & startbb) {
			pt = i;
			break;
		}
	}
	while (attk != 0) {
		end = ls1bindice(attk);
		endbb = 1ull << end;
		attk &= attk - 1;
		++length;
		tmp = start | (end << 6);
		if (pos.occupied & endbb)
			tmp |= CAPTURE_MOVE;
		switch (pt) {
		case PAWN:
			if (end == (color ? (start - 16) : (start + 16))) {
				tmp |= DOUBLE_PAWN_PUSH;
				break;
			}
			if (end == (pos.flags & EP_SQUARE)) {
				tmp |= CAPTURE_MOVE;
				tmp |= EP_CAPTURE;
				break;
			}
			if ((end / 8) == (color ? RANK_1 : RANK_8)) {
				lsPtr[length++] = tmp | KNIGHT_PROMOTION;
				lsPtr[length++] = tmp | BISHOP_PROMOTION;
				lsPtr[length++] = tmp | ROOK_PROMOTION;
				tmp |= QUEEN_PROMOTION;
				break;
			}
			break;
		case KING:
			if (end == (start + 2)) {
				tmp |= KINGSIDE_CASTLE;
				break;
			}
			if (end == (start - 2)) 
				tmp |= QUEENSIDE_CASTLE;
			break;
		}
		lsPtr[length] = tmp;
	}
	lsPtr[0] = length;
}

void generate_moves(const struct position_t pos, uint16_t *lsPtr)
{
	int color = (pos.flags & WHITE_TO_MOVE) ? WHITE : BLACK;
	int sq = 0;
	uint64_t pbb = 0;
	uint64_t attk = 0;
	uint64_t enemy = pos.pieces[BLACK - color][0];
	uint64_t friendly = pos.pieces[color][0];
	pbb = pos.pieces[color][PAWN];
	if (pos.flags & EN_PASSANT)
		enemy ^= 1ull << (pos.flags & EP_SQUARE);
	while (pbb != 0) {
		sq = ls1bindice(pbb);
		attk = pawn_moves(enemy, pos.empty, color, sq);
		attk &= ~friendly;
		serialize_moves(sq, attk, pos, lsPtr);
		pbb &= pbb - 1;
	}
	if (pos.flags & EN_PASSANT)
		enemy ^= 1ull << (pos.flags & EP_SQUARE);
	pbb = pos.pieces[color][BISHOP];
	while (pbb != 0) {
		sq = ls1bindice(pbb);
		attk = bishop_moves(pos.occupied, sq / 8, sq % 8);
		attk &= ~friendly;
		serialize_moves(sq, attk, pos, lsPtr);
		pbb &= pbb - 1;
	}
	pbb = pos.pieces[color][KNIGHT];
	while (pbb != 0) {
		sq = ls1bindice(pbb);
		attk = knight_attack_lookups[sq];
		attk &= ~friendly;
		serialize_moves(sq, attk, pos, lsPtr);
		pbb &= pbb - 1;
	}
	pbb = pos.pieces[color][ROOK];
	while (pbb != 0) {
		sq = ls1bindice(pbb);
		attk = rook_moves(pos.occupied, sq / 8, sq % 8);
		attk &= ~friendly;
		serialize_moves(sq, attk, pos, lsPtr);
		pbb &= pbb - 1;
	}
	pbb = pos.pieces[color][QUEEN];
	while (pbb != 0) {
		sq = ls1bindice(pbb);
		attk = queen_moves(pos.occupied, sq / 8, sq % 8);
		attk &= ~friendly;
		serialize_moves(sq, attk, pos, lsPtr);
		pbb &= pbb - 1;
	}
	sq = pos.kingpos[color];
	attk = king_attack_lookups[sq];
	attk &= ~friendly;
	serialize_moves(sq, attk, pos, lsPtr);
	if (pos.flags & BOTH_BOTH_CASTLE)
		serialize_moves(sq, castle_moves(pos), pos, lsPtr);
}
