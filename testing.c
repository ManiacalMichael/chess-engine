#include <stdio.h>
#include <stdbool.h>
#include "headers/chess.h"
#include "headers/utility.h"
#include "headers/search.h"
#include "headers/testpos.h"

bool find_move(struct movelist_t, uint32_t);

void printpos(struct position_t);

unsigned long long perft(struct position_t, int);

int main(void)
{
	printf("Starting perft tests\n");
	printf("Start position:\n");
	printf("%s%d%s", "Perft(2): Expected 400 Actual ", 
			perft(START_POSITION, 2), "\n");
	printf("%s%d%s", "Perft(3): Expected 8092 Actual ", 
			perft(START_POSITION, 3), "\n");
	printf("%s%d%s", "Perft(4): Expected 197281 Actual ", 
			perft(START_POSITION, 4), "\n");
	printf("Test position 1\n");
	printpos(perft1);
	printf("%s%d%s", "Perft(2): Expected 2039 Actual ",
			perft(perft1, 2), "\n");
	printf("%s%d%s", "Perft(3): Expected 97862 Actual ",
			perft(perft1, 3), "\n");
	printf("%s%d%s", "Perft(4): Expected 4085603 Actual ",
			perft(perft1, 4), "\n");
	return 0;
}

unsigned long long perft(struct position_t pos, int depth)
{
	struct movelist_t ls = generate_moves(&pos);
	struct movenode_t *p = ls.root;
	struct position_t testpos = pos;
	int r = 0;
	if (depth == 0) {
		delete_list(&ls);
		return 1;
	}
	while (p != NULL) {
		make_move(&testpos, p->move);
		r += perft(testpos, depth - 1);
		testpos = pos;
		p = p->nxt;
	}
	delete_list(&ls);
	return r;
}

bool find_move(struct movelist_t ls, uint32_t mv)
{
	struct movenode_t *p = ls.root;
	if (p == NULL)
		return false;
	while (p != NULL) {
		if (p->move == mv)
			return true;
		p = p->nxt;
	}
	return false;
}

void printpos(struct position_t pos)
{
	int k;
	printf("%s", " ---------------\n");
	for(int i = 7; i >= 0; i--) {
		printf("%c", '|');
		for(int j = 0; j < 8; j++) {
			k = (i * 8) + j;
			if (pos.board.black_pieces & (1ull << k)) {
				if (pos.board.pawns & (1ull << k))
					printf("%c", 'p');
				else if (pos.board.knights & (1ull << k))
					printf("%c", 'n');
				else if (pos.board.bishops & (1ull << k))
					printf("%c", 'b');
				else if (pos.board.rooks & (1ull << k))
					printf("%c", 'r');
				else if (pos.board.queens & (1ull << k))
					printf("%c", 'q');
				else if (pos.board.kings & (1ull << k))
					printf("%c", 'k');
				else
					printf("%c", '%');
			} else if (pos.board.occupied & (1ull << k)) {
				if (pos.board.pawns & (1ull << k))
					printf("%c", 'P');
				else if (pos.board.knights & (1ull << k))
					printf("%c", 'N');
				else if (pos.board.bishops & (1ull << k))
					printf("%c", 'B');
				else if (pos.board.rooks & (1ull << k))
					printf("%c", 'R');
				else if (pos.board.queens & (1ull << k))
					printf("%c", 'Q');
				else if (pos.board.kings & (1ull << k))
					printf("%c", 'K');
				else
					printf("%c", '%');
			} else
				printf("%c", '.');
			printf("%c", ' ');
		}
		printf("%s", "|\n");
	}
	printf("%s", " ---------------\n");
	/*
	if (pos.flags & EN_PASSANT) {
		printf("%s", "e.p. capture available on sq#");
		printf("%d\n", (pos.flags & EP_SQUARE) >> 1);
	}
	if (pos.flags & BOTH_BOTH_CASTLE) {
		if (pos.flags & WHITE_KINGSIDE_CASTLE)
			printf("%c", 'W');
		if (pos.flags & WHITE_QUEENSIDE_CASTLE)
			printf("%c", 'w');
		if (pos.flags & BLACK_KINGSIDE_CASTLE)
			printf("%c", 'B');
		if (pos.flags & BLACK_QUEENSIDE_CASTLE)
			printf("%c", 'b');
		printf("%c", '\n');
	}
	if (pos.flags & BLACK_CHECK)
		printf("%s", "Black is in check\n");
	if (pos.flags & WHITE_CHECK)
		printf("%s", "White is in check\n");
	if (pos.flags & GAME_OVER)
		printf("%s", "Game is over\n");
	if (pos.flags & GAME_DRAWN)
		printf("%s", "Game is a draw\n");
	if (pos.flags & WHITE_TO_MOVE)
		printf("%s", "White's move\n");
	else
		printf("%s", "Black's move\n");
	*/
}

