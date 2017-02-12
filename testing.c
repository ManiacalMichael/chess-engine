#include <stdio.h>
#include "headers/chess.h"
#include "headers/search.h"
#include "headers/testpos.h"

char getpiece(const struct position_t, int, int);

void printpos(struct position_t);

int main(void)
{
	struct position_t testpos = START_POSITION;
	printf("%s", "Starting perft tests\n");
	printf("%s", "Start Position:\n");
	printpos(testpos);
	for (int i = 1; i <= 4; ++i) 
		printf("%s%d%s%d%s%d\n", "Depth(", i,
				") Expected value ", start_position_expected[i],
				" Actual value ", perft(&testpos, i));
	testpos = perft1;
	printf("%s", "Perft 1 test position:\n");
	printpos(perft1);
	for (int i = 1; i <= 4; ++i) 
		printf("%s%d%s%d%s%d\n", "Depth(", i,
				") Expected value ", perft1_expected[i],
				" Actual value ", perft(&testpos, i));
	return 0;
}

void printpos(struct position_t pos)
{
	char display[10][19] = {
		"#################\n",
		"#% % % % % % % %#\n",
		"#% % % % % % % %#\n",
		"#% % % % % % % %#\n",
		"#% % % % % % % %#\n",
		"#% % % % % % % %#\n",
		"#% % % % % % % %#\n",
		"#% % % % % % % %#\n",
		"#% % % % % % % %#\n",
		"#################\n"
	};
	uint64_t bb = 0;
	for (signed i = 7; i >= 0; --i) {
		for (int j = 0; j < 8; ++j) {
			bb = (1ull << ((i * 8) + j));
			display[(8 - i) + 1][2 + (2 * j)] = getpiece(pos, i, j);
		}
	}
	for (int i = 0; i < 10; ++i)
		printf("%s", display[i]);
}

char getpiece(const struct position_t pos, int i, int j)
{
	char whitepieces[6] = { 'P', 'N', 'B', 'R', 'Q', 'K' };
	char blackpieces[6] = { 'p', 'n', 'b', 'r', 'q', 'k' };
	uint64_t bb = 0;
	bb = 1ull << ((i * 8) + j);
	if (pos.occupied & bb) {
		if (pos.pieces[WHITE][0] & bb) {
			for (int k = PAWN; k <= KING; ++k) {
				if (pos.pieces[WHITE][k] & bb)
					return whitepieces[k];
			}
			return '%';
		} else if (pos.pieces[BLACK][0] & bb) {
			for (int k = PAWN; k <= KING; ++k) {
				if (pos.pieces[BLACK][k] & bb)
					return blackpieces[k];
			}
			return '%';
		}
		return '%';
	}
	for (int k = 0; k <= KING; ++k) {
		if (pos.pieces[WHITE][0] & bb)
			return '%';
	}
	for (int k = 0; k <= KING; ++k) {
		if (pos.pieces[BLACK][0] & bb)
			return '%';
	}
	return '.';
}
