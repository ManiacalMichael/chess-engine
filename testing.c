#include <stdio.h>
#include "headers/chess.h"
#include "headers/utility.h"
#include "headers/search.h"

void printpos(struct position_t);

const struct position_t fvb11Bg5 = {
	{
		0x69f366445824e3b8ull,
		0x69f3660040000000ull,
		0x4000000000000010ull,
		0x0800000400000000ull,
		0x2100000000000088ull,
		0x0040004040000020ull,
		0x0000220000240000ull,
		0x00b344001800e300ull,
		0x0010af3ba7db1847ull,
		0x960c99aa15a00000ull
	},
	0x0080u,
	22,
	8
};

const struct position_t kvt5Qd2 = {
	{
		0x9ff768001814eff1ull,
		0x9ff7680000000000ull,
		0x1000000000000010ull,
		0x0800000000000800ull,
		0x8100000000000081ull,
		0x0440000000100020ull,
		0x0200200000040040ull,
		0x00b748001800e700ull,
		0x0000000000000000ull,
		0x0000000000000000ull
	},
	0x0780u,
	10,
	3
};

const struct position_t cvl56Rb8 = {
	{
		0x020000001c000300ull,
		0x0000000004000300ull,
		0x0000000014000000ull,
		0x0000000000000000ull,
		0x0200000000000100ull,
		0x0000000008000000ull,
		0x0000000000000000ull,
		0x0000000000000200ull,
		0x0000000000000000ull,
		0x0000000000000000ull
	},
	0x0000u,
	112,
	7
};

int main(void)
{
	struct position_t testpos = START_POSITION;
	struct movelist_t ls;
	struct movenode_t *p;
	printf("%s", "Start position: \n");
	printpos(testpos);
	printf("%s", "Moves: \n");
	ls = generate_moves(&testpos);
	p = ls.root;
	while (p != NULL) {
		make_move(&testpos, p->move);
		printpos(testpos);
		p = p->nxt;
		testpos = START_POSITION;
	}
	testpos = fvb11Bg5;
	printf("%s", "\nFischer v Byrne 11.Bg5\n");
	printpos(testpos);
	printf("%s", "Moves: \n");
	ls = generate_moves(&testpos);
	p = ls.root;
	while (p != NULL) {
		make_move(&testpos, p->move);
		printpos(testpos);
		p = p->nxt;
		testpos = fvb11Bg5;
	}
	testpos = kvt5Qd2;
	printf("%s", "\nKasparov v Topalov 5.Qd2\n");
	printpos(testpos);
	printf("%s", "Moves: \n");
	ls = generate_moves(&testpos);
	p = ls.root;
	while (p != NULL) {
		make_move(&testpos, p->move);
		printpos(testpos);
		p = p->nxt;
		testpos = kvt5Qd2;
	}
	testpos = cvl56Rb8;
	printf("%s", "\nCarlsen v Liren 56.Rb8\n");
	printpos(testpos);
	printf("%s", "Moves: \n");
	ls = generate_moves(&testpos);
	p = ls.root;
	while (p != NULL) {
		make_move(&testpos, p->move);
		printpos(testpos);
		p = p->nxt;
		testpos = cvl56Rb8;
	}
	return 0;
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
}

