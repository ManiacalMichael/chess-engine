#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#if !defined(FIND_ITER)
    #define FIND_ITER 4096
#endif
#if !defined(ORDER_ITER)
    #define ORDER_ITER 65536
#endif

uint64_t random_u64()
{
	uint64_t u1, u2, u3, u4;
	u1 = (uint64_t)(random()) & 0xffff;
	u2 = (uint64_t)(random()) & 0xffff;
	u3 = (uint64_t)(random()) & 0xffff;
	u4 = (uint64_t)(random()) & 0xffff;
	return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

uint64_t randomfew()
{
	return random_u64() & random_u64() & random_u64();
}

int popcount(uint64_t bb)
{
	int x;
	for (x = 0; bb; ++x, bb &= bb - 1);
	return x;
}

const int index64[64] = {
	0,  1, 48,  2, 57, 49, 28,  3,
	61, 58, 50, 42, 38, 29, 17,  4,
	62, 55, 59, 36, 53, 51, 43, 22,
	45, 39, 33, 30, 24, 18, 12,  5,
	63, 47, 56, 27, 60, 41, 37, 16,
	54, 35, 52, 21, 44, 32, 23, 11,
	46, 26, 40, 15, 34, 20, 31, 10,
	25, 14, 19,  9, 13,  8,  7,  6
};

int popfirst(uint64_t *bb)
{
	const uint64_t mul = 0x03f79d71b4cb0a89;
	uint64_t b = (*bb ^ (*bb - 1)) & *bb;
	assert(*bb != 0);
	*bb &= (*bb - 1);
	return index64[(b * mul) >> 58];
}

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

uint64_t rmask(int sq)
{
	uint64_t result = 0ull;
	int rank = sq / 8, file = sq % 8, r = 0, f = 0;
	for (r = rank + 1; r <= 6; ++r)
		result |= (1ull << (file + r * 8));
	for (r = rank - 1; r >= 1; --r)
		result |= (1ull << (file + r * 8));
	for (f = file + 1; f <= 6; ++f)
		result |= (1ull << (f + rank * 8));
	for (f = file - 1; f >= 1; --f)
		result |= (1ull << (f + rank * 8));
	return result;
}

uint64_t bmask(int sq)
{
	uint64_t result = 0ull;
	int rank = sq / 8, file = sq % 8, r = 0, f = 0;
	for (r = rank + 1, f = file + 1; r <= 6 && f <= 6; ++r, ++f)
		result |= (1ull << (f + r * 8));
	for (r = rank + 1, f = file - 1; r <= 6 && f >= 1; ++r, --f)
		result |= (1ull << (f + r * 8));
	for (r = rank - 1, f = file + 1; r >= 1 && f <= 6; --r, ++f)
		result |= (1ull << (f + r * 8));
	for (r = rank - 1, f = file - 1; r >= 1 && f >= 1; --r, --f)
		result |= (1ull << (f + r * 8));
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

uint64_t used[4096];
uint64_t attacktable[2 * 64 * 4096];
uint64_t bmagic[64], rmagic[8][64];
int bcost[64], rcost[8][64];
uint64_t bused[64][4096], rused[8][64][4096];
typedef struct {
	int bindex[64];
	int rindex[64];
	int whichr[64];
} conf_t;
conf_t testconf;

uint64_t find_magic(int sq, int bishop)
{
	uint64_t mask, block[4096], attack[4096], magic;
	unsigned int i, j, k, mbits, fail;
	mask = bishop? bmask(sq) : rmask(sq);
	mbits = popcount(mask);
	for (i = 0; i < (1 << mbits); ++i) 
		block[i] = genindex(i, mbits, mask);
	for (i = 0; i < (1 << mbits); ++i) 
		attack[i] = bishop? batt(sq, block[i]) : ratt(sq, block[i]);
	for (k = 0; k < 100000000; ++k) {
		magic = randomfew();
		fail = 0;
		for (i = 0; i < 4096; ++i) used[i] = 0ull;
		for (i = 0; !fail && (i < (1 << mbits)); ++i) {
			j = (block[i] * magic) >> (bishop? 55 : 52);
			if (used[j] == 0ull) used[j] = attack[i];
			else if (used[j] != attack[i]) fail = 1;
		}
		if (!fail) 
			return magic;
	}
	fputs("/***Failed***/\n", stderr);
	return -1;
}

int calccost()
{
	int gaps = 0, open = 0, length = 0, run = 0;
	/* gaps - runs of empty indices */
	/* open - open slots */
	for (int i = 0; i < 4096; ++i) {
		if (used[i]) {
			length = i;
			run = 0;
		}
		else {
			++open;
			++run;
		}
		if (run == 1)
			++gaps;
	}
	return (2 * length) - open + gaps;
}

int insertcost(int cost, int sq)
{
	for (int i = 0; i < 8; ++i) {
		if (rcost[i][sq] > cost) {
			for (int j = 7; j > i; --j)
				rcost[j][sq] = rcost[j + 1][sq];
			rcost[i][sq] = cost;
			return i;
		}
	}
	return 8;
}

void printbb(uint64_t bb)
{
	for (int i = 7; i >= 0; --i) {
		for (int j = 0; j < 8; ++j) {
			if (bb & (1ull << ((i * 8) + j)))
				putchar('1');
			else
				putchar('0');
		}
		puts("");
	}
}

void printtable(char *name, uint64_t *table, int index)
{
	puts(name);
	for (int i = 0; i < index; ++i)
		printf("\t0x%llx,\n", table[i]);
	puts("};");
}

void order()
{
	/* select 64 magics and place them in an order */
	int num = 0;
	uint64_t rand0 = random_u64();
	uint64_t rand1 = random_u64();
	uint64_t rand2 = random_u64();
	for (int i = 0; i < 64; ++i) testconf.bindex[i] = 0;
	for (int i = 0; i < 64; ++i) {
		num = 0;
		if (rand0 & (1ull << i))
			num += 1;
		if (rand1 & (1ull << i))
			num += 2;
		if (rand2 & (1ull << i))
			num += 4;
		while (!rmagic[num][i]) --num; /*no 0s plz :(*/
		testconf.whichr[i] = num;
	}
	for (int i = 0; i < 64; ++i) {
		do {
			num = rand() % 64;
		} while (testconf.bindex[num]);
		testconf.bindex[num] = i;
		/* bindex holds the order in which the rook tables will be inserted */
	}
}

void cpused(int bishop, int sq, int index)
{
	for (int i = 0; i < 4096; ++i)
		bishop?
		(bused[sq][i] = used[i]):
		(rused[index][sq][i] = used[i]);
}

int destructive(int bishop, int sq, int index)
{
	uint64_t *arr = bishop? bused[sq] : rused[testconf.whichr[sq]][sq];
	for (int i = 0; i < 4096; ++i)
		if (attacktable[index + i] && arr[i])
			return 1;
	return 0;
}

int try()
{
	int index = 127 * 4096;
	int whichr;
	for (int i = 0; i < 2 * 64 * 4096; ++i) attacktable[i] = 0;
	for (int i = 0; i < 64; ++i) {
		whichr = testconf.whichr[testconf.bindex[i]];
		for (int k = 0; k < 127 * 4096; ++k) {
			if (!destructive(0, testconf.bindex[i], k)) {
				index = k;
				break;
			}
		}
		testconf.rindex[testconf.bindex[i]] = index;
		for (int j = 0; j < 4096; ++j)
			if (rused[whichr][i][j])
				attacktable[index + j] = 1;
	}
	for (int i = 0; i < 64; ++i) {
		for (int k = 0; k < 127 * 4096; ++k) {
			if (!destructive(1, i, k)) {
				index = k;
				break;
			}
		}
		testconf.bindex[i] = index;
		for (int j = 0; j < 4096; ++j)
			if (bused[i][j])
				attacktable[index + j] = 1;
	}
	for (int i = 0; i < 2 * 64 * 4096; ++i)
		if (attacktable[i])
			index = i;
	return index;
}

int main()
{
	int rindex = 0;
	int num;
	int bestsize = 9999999;
	uint64_t magic;
	conf_t bestconf;
	for (int i = 0; i < 64; ++i) {
		bcost[i] = 8193;
		for (int j = 0; j < 8; ++j)
			rcost[j][i] = 8193;
	}
	for (int i = 0; i < FIND_ITER; ++i) {
		for (int sq = 0; sq < 64; ++sq) {
			magic = find_magic(sq, 0);
			if ((num = insertcost(calccost(), sq)) != 8) {
				rmagic[num][sq] = magic;
				printf(
	"Improved rook magic \tcost:%-10dsq:%-5di:%d\n", rcost[num][sq], sq, i);
				cpused(0, sq, num);
			}
			magic = find_magic(sq, 1);
			if ((num = calccost()) < bcost[sq]) {
				bcost[sq] = num;
				bmagic[sq] = magic;
				printf(
	"Improved bishop magic \tcost:%-10dsq:%-5di:%d\n", num, sq, i);
				cpused(1, sq, 0);
			}
		}
	}
	for (int i = 0; i < ORDER_ITER; ++i) {
		order();
		if ((num = try()) < bestsize) {
			printf("Improved size from %-8d to %-8d i:%d\n", bestsize, num, i);
			bestsize = num;
			bestconf = testconf;
		}
	}
	for (int i = 0; i < 64; ++i)
		rmagic[0][i] = rmagic[bestconf.whichr[i]][i];
	printf("Best table size: %d KiB\n", bestsize * 8 / 1024);
	printtable("const uint64_t rmagic[64] = {", rmagic[0], 64);
	printtable("const uint64_t bmagic[64] = {", bmagic, 64);
	puts("const int rindex[64] = {");
	for (int i = 0; i < 64; ++i) printf("\t%d,\n", bestconf.rindex[i]);
	puts("};\nconst int bindex[64] = {");
	for (int i = 0; i < 64; ++i) printf("\t%d,\n", bestconf.bindex[i]);
	puts("};");
	return 0;
}
