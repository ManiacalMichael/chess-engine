#include <stdio.h>
#include "headers/chess.h"
#include "headers/search.h"
#include "headers/testpos.h"

const char files[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };

char getpiece(const struct position_t, int, int);

void printpos(struct position_t);

unsigned long long dperft(struct position_t *posPtr, int depth, int indent);

int main(void)
{
	struct position_t testpos = START_POSITION;
	uint16_t movelist[MAX_MOVES + 1] = { 0 };
	int start;
	int end;
	int depth = 7;
	printf("%s", "Starting perft tests\n");
	while ((depth < 1) || (depth > 6)) {
		printf("%s", "depth(1-6): ");
		scanf("%d", &depth);
	}
	printpos(testpos);
	for (int i = 1; i <= depth; ++i) 
		printf("%s%d%s%-15d%s%d\n", "Perft(", i,
				") Expected value ", start_position_expected[i],
				" Actual value ", perft(&testpos, i));
	testpos = perft1;
	printf("%s", "Perft test position 1:\n");
	printpos(perft1);
	for (int i = 1; i <= depth; ++i) 
		printf("%s%d%s%-15d%s%d\n", "Perft(", i,
				") Expected value ", perft1_expected[i],
				" Actual value ", perft(&testpos, i));
	testpos = perft1;
	generate_moves(testpos, movelist);
	for (int i = 1; i <= movelist[0]; ++i) {
		start = movelist[i] & START_SQUARE;
		end = (movelist[i] & END_SQUARE) >> 6;
		make_move(&testpos, movelist[i]);
		printf( "%c%d%c%d", files[start % 8], (start / 8) + 1,
				files[end % 8], (end / 8) + 1);
		printf( "%s%d\n", " ", perft(&testpos, depth - 1));
		unmake_move(&testpos, movelist[i]);
	}
	return 0;
}

unsigned long long dperft(struct position_t *posPtr, int depth, int indent)
{
	uint16_t movelist[MAX_MOVES + 1];
	uint64_t total = 0;
	uint64_t tmp;
	int start, end;
	int color = (posPtr->flags & WHITE_TO_MOVE) ? WHITE : BLACK;
	movelist[0] = 0;
	generate_moves(*posPtr, movelist);
	for (int i = 1; i <= movelist[0]; ++i) 
		if (((movelist[i] & END_SQUARE) >> 6) 
				== posPtr->kingpos[BLACK - color]) 
			return 0;
	if (depth == 0)
		return 1;
	for (int i = 1; i <= movelist[0]; ++i) {
		start = movelist[i] & START_SQUARE;
		end = (movelist[i] & END_SQUARE) >> 6;
		make_move(posPtr, movelist[i]);
		tmp = dperft(posPtr, depth - 1, indent + 1);
		if (!tmp) {
			unmake_move(posPtr, movelist[i]);
			continue;
		}
		total += tmp;
		putchar('\n');
		for (int j = 0; j < indent; ++j)
			putchar('\t');
		printf("%c%d%c%d%s%d", files[start % 8], (start / 8) + 1,
				files[end % 8], (end / 8) + 1, "  ", tmp);
		unmake_move(posPtr, movelist[i]);
	}
	return total;
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
			display[8 - i][1 + (2 * j)] = getpiece(pos, i, j);
		}
	}
	for (int i = 0; i < 10; ++i)
		printf("%s", display[i]);
}

char getpiece(const struct position_t pos, int i, int j)
{
	char whitepieces[7] = { '%', 'P', 'N', 'B', 'R', 'Q', 'K' };
	char blackpieces[7] = { '%', 'p', 'n', 'b', 'r', 'q', 'k' };
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
