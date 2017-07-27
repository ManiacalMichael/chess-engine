#include <assert.h>
#include <stdint.h>
#include "headers/chess.h"
#include "headers/search.h"

const uint64_t rook_masks[64] = {
	0x101010101017e,
	0x202020202027c,
	0x404040404047a,
	0x8080808080876,
	0x1010101010106e,
	0x2020202020205e,
	0x4040404040403e,
	0x8080808080807e,
	0x1010101017e00,
	0x2020202027c00,
	0x4040404047a00,
	0x8080808087600,
	0x10101010106e00,
	0x20202020205e00,
	0x40404040403e00,
	0x80808080807e00,
	0x10101017e0100,
	0x20202027c0200,
	0x40404047a0400,
	0x8080808760800,
	0x101010106e1000,
	0x202020205e2000,
	0x404040403e4000,
	0x808080807e8000,
	0x101017e010100,
	0x202027c020200,
	0x404047a040400,
	0x8080876080800,
	0x1010106e101000,
	0x2020205e202000,
	0x4040403e404000,
	0x8080807e808000,
	0x1017e01010100,
	0x2027c02020200,
	0x4047a04040400,
	0x8087608080800,
	0x10106e10101000,
	0x20205e20202000,
	0x40403e40404000,
	0x80807e80808000,
	0x17e0101010100,
	0x27c0202020200,
	0x47a0404040400,
	0x8760808080800,
	0x106e1010101000,
	0x205e2020202000,
	0x403e4040404000,
	0x807e8080808000,
	0x7e010101010100,
	0x7c020202020200,
	0x7a040404040400,
	0x76080808080800,
	0x6e101010101000,
	0x5e202020202000,
	0x3e404040404000,
	0x7e808080808000,
	0x7e01010101010100,
	0x7c02020202020200,
	0x7a04040404040400,
	0x7608080808080800,
	0x6e10101010101000,
	0x5e20202020202000,
	0x3e40404040404000,
	0x7e8080808080800l
};

const uint64_t bishop_masks[64] = {
	0x40201008040200,
	0x402010080400,
	0x4020100a00,
	0x40221400,
	0x2442800,
	0x204085000,
	0x20408102000,
	0x2040810204000,
	0x20100804020000,
	0x40201008040000,
	0x4020100a0000,
	0x4022140000,
	0x244280000,
	0x20408500000,
	0x2040810200000,
	0x4081020400000,
	0x10080402000200,
	0x20100804000400,
	0x4020100a000a00,
	0x402214001400,
	0x24428002800,
	0x2040850005000,
	0x4081020002000,
	0x8102040004000,
	0x8040200020400,
	0x10080400040800,
	0x20100a000a1000,
	0x40221400142200,
	0x2442800284400,
	0x4085000500800,
	0x8102000201000,
	0x10204000402000,
	0x4020002040800,
	0x8040004081000,
	0x100a000a102000,
	0x22140014224000,
	0x44280028440200,
	0x8500050080400,
	0x10200020100800,
	0x20400040201000,
	0x2000204081000,
	0x4000408102000,
	0xa000a10204000,
	0x14001422400000,
	0x28002844020000,
	0x50005008040200,
	0x20002010080400,
	0x40004020100800,
	0x20408102000,
	0x40810204000,
	0xa1020400000,
	0x142240000000,
	0x284402000000,
	0x500804020000,
	0x201008040200,
	0x402010080400,
	0x2040810204000,
	0x4081020400000,
	0xa102040000000,
	0x14224000000000,
	0x28440200000000,
	0x50080402000000,
	0x20100804020000,
	0x40201008040200
};

const uint64_t rook_magics[64] = {
        0x4080008211614000,
        0x8028004c00804002,
        0x10022010042000,
        0x80020800800410,
        0x8120100200111020,
        0x6200111018109200,
        0x40088009000840,
        0x880004024800100,
        0x400220011444,
        0x48100400021000c,
        0x800800914008004,
        0x10100006003002,
        0x2002701140100410,
        0x2000852c0020,
        0x282800040800082,
        0x200020104082,
        0x100988000884018,
        0x4000095000c400,
        0x42001004020490,
        0x60022c0084001000,
        0xe004004001022004,
        0x8a008810009820,
        0x2480008002140308,
        0x1100802000400020,
        0x6001001340014020,
        0x102009020801002,
        0x80048012080080,
        0x10004040060028,
        0x4400020021040001,
        0x2000200110081084,
        0x3850000840c2002,
        0x5040002e00040041,
        0x5002800080980010,
        0x40a063000100018,
        0x4840040020804,
        0x1000090013041000,
        0x80200200200560,
        0x8008200200100,
        0x2c0020001502001,
        0x280882000200040,
        0x208800050080400,
        0x4480900424000880,
        0x2060001002310800,
        0x18c01838060005,
        0x100080100030084,
        0x1000201008200,
        0x200008408020061,
        0x2202008011004,
        0x200018410040,
        0x489010080240,
        0x2002004400100010,
        0x4008280880101021,
        0x8092000100880010,
        0x230020080840080,
        0x1110010000800042,
        0x20880040200020,
        0x8828002104100a1,
        0x480201201000842,
        0x440048e462082,
        0x41201100846,
        0x100020800041001,
        0x1800011004008802,
        0x100004608008401,
        0xa000436400408112
};

const uint64_t bishop_magics[64] = {
        0x90004180884008,
        0x4000802040440200,
        0x400802024300040,
        0x40100400480c400,
        0x40440204002080,
        0x240402049000,
        0x4020244202c000,
        0x800208020100200,
        0x800020080a0a004,
        0x402040802040,
        0x53008080040901,
        0x1103008808020202,
        0x402008404c60800,
        0x20004100808000,
        0x801e01221040090,
        0x8222044020804020,
        0x2000400801010,
        0x2000840100111040,
        0x80010101400a,
        0x400280404100,
        0x8080800010042992,
        0x20100020101040,
        0x1400044810808a00,
        0x1000026010108220,
        0x1040100806041,
        0x8002018000a08014,
        0x8002006001004080,
        0x100208005404c028,
        0x8840031802010,
        0x2000408000420040,
        0x400188040402080,
        0x88848101018,
        0x42008088004100,
        0x4021610240008098,
        0x9810088004080,
        0x1c04200800410050,
        0x84a0108400188120,
        0xb402080008004020,
        0x400808808090,
        0x144008400980401a,
        0x400801080800114,
        0x8210804100680084,
        0x1080801481050080,
        0x42080408020,
        0x100009020840010,
        0x2000440400200011,
        0x202080220810,
        0x802004040420,
        0x10022c1004402,
        0x1402012008120d0,
        0x80100881808380,
        0x300192008404000,
        0x5001000500202100,
        0x400000404080210c,
        0xa04008010400800,
        0x3880204040800410,
        0x4100110101008150,
        0xa005080804020,
        0x20200420085200c0,
        0x4020000140084040,
        0x20001484004011,
        0x1040010300210010,
        0x8304804101001010,
        0x104082050105010
};

uint64_t attack_table[82688];

const int rook_index[64] = {
        38426,
        50647,
        24888,
        12604,
        82774,
        28732,
        78742,
        20262,
        55812,
        6176,
        20262,
        4096,
        10580,
        20262,
        44511,
        10580,
        28730,
        82774,
        78710,
        44568,
        57620,
        72000,
        24886,
        42520,
        44568,
        33088,
        60435,
        38429,
        67342,
        46568,
        18722,
        22310,
        42504,
        24758,
        62550,
        68490,
        75873,
        68494,
        44584,
        53824,
        0,
        30943,
        14638,
        68494,
        10578,
        14636,
        78710,
        44520,
        64954,
        10578,
        24886,
        82774,
        82774,
        6177,
        77686,
        74325,
        36377,
        68494,
        50655,
        75869,
        48862,
        6224,
        8272,
        20262
};

const int bishop_index[64] = {
        9296,
        9424,
        9457,
        19238,
        14994,
        15483,
        15991,
        16502,
        9505,
        15058,
        10612,
        19298,
        19355,
        19432,
        19473,
        15594,
        10677,
        19594,
        34008,
        34156,
        34316,
        34508,
        17022,
        19626,
        10997,
        23444,
        34733,
        34865,
        51776,
        47576,
        16099,
        10742,
        12620,
        23500,
        49631,
        69766,
        76405,
        50719,
        29791,
        33487,
        10933,
        17135,
        52288,
        51007,
        56068,
        52532,
        10868,
        11128,
        17534,
        18046,
        19498,
        34380,
        19690,
        11193,
        11258,
        12684,
        53183,
        18150,
        18567,
        35377,
        30948,
        33548,
        11384,
        35856
};

const uint64_t king_attack[64] = {
        0x302, 
        0x705, 
        0xe0a, 
        0x1c14, 
        0x3828, 
        0x7050, 
        0xe0a0, 
        0xc040, 
        0x30203, 
        0x70507, 
        0xe0a0e, 
        0x1c141c, 
        0x382838, 
        0x705070, 
        0xe0a0e0, 
        0xc040c0, 
        0x3020300, 
        0x7050700, 
        0xe0a0e00, 
        0x1c141c00, 
        0x38283800, 
        0x70507000, 
        0xe0a0e000, 
        0xc040c000, 
        0x302030000, 
        0x705070000, 
        0xe0a0e0000, 
        0x1c141c0000, 
        0x3828380000, 
        0x7050700000, 
        0xe0a0e00000, 
        0xc040c00000, 
        0x30203000000, 
        0x70507000000, 
        0xe0a0e000000, 
        0x1c141c000000, 
        0x382838000000, 
        0x705070000000, 
        0xe0a0e0000000, 
        0xc040c0000000, 
        0x3020300000000, 
        0x7050700000000, 
        0xe0a0e00000000, 
        0x1c141c00000000, 
        0x38283800000000, 
        0x70507000000000, 
        0xe0a0e000000000, 
        0xc040c000000000, 
        0x302030000000000, 
        0x705070000000000, 
        0xe0a0e0000000000, 
        0x1c141c0000000000, 
        0x3828380000000000, 
        0x7050700000000000, 
        0xe0a0e00000000000, 
        0xc040c00000000000, 
        0x203000000000000, 
        0x507000000000000, 
        0xa0e000000000000, 
        0x141c000000000000, 
        0x2838000000000000, 
        0x5070000000000000, 
        0xa0e0000000000000, 
        0x40c0000000000000
};

const uint64_t knight_attack[64] = {
	0x20400,
	0x50800,
	0xa1100,
	0x142200,
	0x284400,
	0x508800,
	0xa01000,
	0x402000,
	0x2040004,
	0x5080008,
	0xa110011,
	0x14220022,
	0x28440044,
	0x50880088,
	0xa0100010,
	0x40200020,
	0x204000402,
	0x508000805,
	0xa1100110a,
	0x1422002214,
	0x2844004428,
	0x5088008850,
	0xa0100010a0,
	0x4020002040,
	0x20400040200,
	0x50800080500,
	0xa1100110a00,
	0x142200221400,
	0x284400442800,
	0x508800885000,
	0xa0100010a000,
	0x402000204000,
	0x2040004020000,
	0x5080008050000,
	0xa1100110a0000,
	0x14220022140000,
	0x28440044280000,
	0x50880088500000,
	0xa0100010a00000,
	0x40200020400000,
	0x204000402000000,
	0x508000805000000,
	0xa1100110a000000,
	0x1422002214000000,
	0x2844004428000000,
	0x5088008850000000,
	0xa0100010a0000000,
	0x4020002040000000,
	0x400040200000000,
	0x800080500000000,
	0x1100110a00000000,
	0x2200221400000000,
	0x4400442800000000,
	0x8800885000000000,
	0x100010a000000000,
	0x2000204000000000,
	0x4020000000000,
	0x8050000000000,
	0x110a0000000000,
	0x22140000000000,
	0x44280000000000,
	0x88500000000000,
	0x10a00000000000,
	0x20400000000000
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

/* these functions are only used for attack table initialization
 * and should not be visible anywhere except in this file
 */
uint64_t genindex(int index, int bits, uint64_t mask)
{
	int i, j;
	uint64_t result = 0ull;
	/* map an index to a possible occupancy */
	for (i = 0; i < bits; ++i) {
		j = popfirst(&mask);
		if (index & (1 << i))
			result |= (1ull << j);
	}
	return result;
}

uint64_t ratt(int sq, uint64_t block)
{
	uint64_t result = 0ull;
	int rank = sq / 8, file = sq % 8, r = 0, f = 0;
	for (r = rank + 1; r <= 7; ++r) {
		result |= (1ull << (file + r * 8));
		if (block & (1ull << (file + r * 8))) break;
	}
	for (r = rank - 1; r >= 0; --r) {
		result |= (1ull << (file + r * 8));
		if (block & (1ull << (file + r * 8))) break;
	}
	for (f = file + 1; f <= 7; ++f) {
		result |= (1ull << (f + rank * 8));
		if (block & (1ull << (f + rank * 8))) break;
	}
	for (f = file - 1; f >= 0; --f) {
		result |= (1ull << (f + rank * 8));
		if (block & (1ull << (f + rank * 8))) break;
	}
	return result;
}

uint64_t batt(int sq, uint64_t block)
{
	uint64_t result = 0ull;
	int rank = sq / 8, file = sq % 8, r = 0, f = 0;
	for (r = rank + 1, f = file + 1; r <= 7 && f <= 7; ++r, ++f) {
		result |= (1ull << (f + r * 8));
		if (block & (1ull << (f + r * 8))) break;
	}
	for (r = rank + 1, f = file - 1; r <= 7 && f >= 0; ++r, --f) {
		result |= (1ull << (f + r * 8));
		if (block & (1ull << (f + r * 8))) break;
	}
	for (r = rank - 1, f = file + 1; r >= 0 && f <= 7; --r, ++f) {
		result |= (1ull << (f + r * 8));
		if (block & (1ull << (f + r * 8))) break;
	}
	for (r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f) {
		result |= (1ull << (f + r * 8));
		if (block & (1ull << (f + r * 8))) break;
	}
	return result;
}
/***/

void init_attack_table(void)
{
	uint64_t mask, block[4096], magic;
	uint64_t *attack;
	unsigned int i, j;
	for (i = 0; i < 82688; ++i)
		attack_table[i] = 0;
	for (j = 0; j < 64; ++j) {
		mask = rook_masks[j];
		mbits = popcount(mask);
		attack = attack_table + rook_index[j];
		for (i = 0; i < (1 << mbits); ++i) 
			block[i] = genindex(i, mbits, mask);
		for (i = 0; i < (1 << mbits); ++i) 
			attack[i] = ratt(j, block[i]);
	}
	for (j = 0; j < 64; ++j) {
		mask = bishop_masks[j];
		mbits = popcount(mask);
		attack = attack_table + bishop_index[j];
		for (i = 0; i < (1 << mbits); ++i) 
			block[i] = genindex(i, mbits, mask);
		for (i = 0; i < (1 << mbits); ++i) 
			attack[i] = batt(j, block[i]);
	}
	
}

uint64_t pawn_moves(uint64_t enemy, uint64_t empty, enum PIECES color, enum SQUARES sq)
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

inline uint64_t bishop_moves(uint64_t occupied, enum SQUARES sq)
{
	return attack_table[(((occupied & bishop_masks[sq]) * bishop_magics[sq]) >> 55) + bishop_index[sq]];
}

inline uint64_t rook_moves(uint64_t occupied, enum SQUARES sq)
{
	return attack_table[(((occupied & rook_masks[sq]) * rook_magics[sq]) >> 52) + rook_index[sq]];
}

inline uint64_t queen_moves(uint64_t occupied, enum SQUARES sq)
{
	uint64_t r = 0ull;
	r |= bishop_moves(occupied, sq);
	r |= rook_moves(occupied, sq);
	return r;
}

uint16_t check_status(const position_t pos)
{
	const uint64_t occupied = pos.bboards[OCCUPIED];
	uint16_t ret = 0;
	int bsq = ls1bindice(pos.bboards[BLACK_KING]);
	int wsq = ls1bindice(pos.bboards[WHITE_KING]);
	if (rook_moves(occupied, wsq) & (pos.bboards[BLACK_ROOK] | pos.bboards[BLACK_QUEEN]))
		ret |= WHITE_CHECK;
	else if (bishop_moves(occupied, wsq) & (pos.bboards[BLACK_BISHOP] | pos.bboards[BLACK_QUEEN]))
		ret |= WHITE_CHECK;
	else if (knight_attacks[wsq] & pos.bboards[BLACK_KNIGHT])
		ret |= WHITE_CHECK;
	else if (pawn_attacks[WHITE][wsq] & pos.bboards[BLACK_PAWN])
		ret |= WHITE_CHECK;
	else if (king_attacks[wsq] & pos.bboards[BLACK_KING])
		return WHITE_CHECK | BLACK_CHECK;
	if (rook_moves(occupied, bsq) & (pos.bboards[WHITE_ROOK] | pos.bboards[WHITE_QUEEN]))
		ret |= BLACK_CHECK;
	else if (bishop_moves(occupied, bsq) & (pos.bboards[WHITE_BISHOP] | pos.bboards[WHITE_QUEEN]))
		ret |= BLACK_CHECK;
	else if (knight_attacks[bsq] & pos.bboards[WHITE_KNIGHT])
		ret |= BLACK_CHECK;
	else if (pawn_attacks[BLACK][bsq] & pos.bboards[WHITE_PAWN])
		ret |= BLACK_CHECK;
	else if (king_attacks[bsq] & pos.bboards[WHITE_KING])
		return WHITE_CHECK | BLACK_CHECK;
	return ret;
}

uint64_t castle_moves(position_t pos)
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

void serialize_moves(int start, uint64_t attk, position_t pos,
		movels_t *lsPtr)
{
	int color = (pos.flags & WHITE_TO_MOVE) ? WHITE : BLACK;
	uint64_t startbb = 1ull << start;
	enum SQUARES end;
	uint64_t endbb;
	enum PIECETYPES pt;
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
		++lsPtr->top;
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
				lsPtr->list[lsPtr->top++] = tmp | KNIGHT_PROMOTION;
				lsPtr->list[lsPtr->top++] = tmp | BISHOP_PROMOTION;
				lsPtr->list[lsPtr->top++] = tmp | ROOK_PROMOTION;
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
		lsPtr->list[lsPtr->top] = tmp;
	}
}

void generate_moves(const position_t pos, movels_t *lsPtr)
{
	int color = (pos.flags & WHITE_TO_MOVE) ? WHITE : BLACK;
	enum SQUARES sq = 0;
	uint64_t pbb = 0;
	uint64_t attk = 0;
	uint64_t enemy = pos.bboards[BLACK - color];
	uint64_t friendly = pos.bboards[color];
	pbb = pos.bboards[color + (PAWN * 2)];
	if (pos.flags & EN_PASSANT)
		/* temporarily place an enemy piece on the e.p. square
		 * so pawns can capture it
		 */
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
	pbb = pos.pieces[color + (BISHOP * 2)];
	while (pbb != 0) {
		sq = ls1bindice(pbb);
		attk = bishop_moves(pos.bboards[OCCUPIED], sq);
		attk &= ~friendly;
		serialize_moves(sq, attk, pos, lsPtr);
		pbb &= pbb - 1;
	}
	pbb = pos.pieces[color + (KNIGHT * 2)];
	while (pbb != 0) {
		sq = ls1bindice(pbb);
		attk = knight_attacks[sq];
		attk &= ~friendly;
		serialize_moves(sq, attk, pos, lsPtr);
		pbb &= pbb - 1;
	}
	pbb = pos.pieces[color + (ROOK * 2)];
	while (pbb != 0) {
		sq = ls1bindice(pbb);
		attk = rook_moves(pos.occupied, sq);
		attk &= ~friendly;
		serialize_moves(sq, attk, pos, lsPtr);
		pbb &= pbb - 1;
	}
	pbb = pos.pieces[color + (QUEEN * 2)];
	while (pbb != 0) {
		sq = ls1bindice(pbb);
		attk = queen_moves(pos.occupied, sq);
		attk &= ~friendly;
		serialize_moves(sq, attk, pos, lsPtr);
		pbb &= pbb - 1;
	}
	sq = ls1bindice(pos.bboards[color + (2 * KING)];
	attk = king_attack_lookups[sq];
	attk &= ~friendly;
	serialize_moves(sq, attk, pos, lsPtr);
	if (pos.flags & BOTH_BOTH_CASTLE)
		serialize_moves(sq, castle_moves(pos), pos, lsPtr);
}
