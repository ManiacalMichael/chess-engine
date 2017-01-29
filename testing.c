#include <stdio.h>
#include <stdbool.h>
#include "headers/chess.h"
#include "headers/utility.h"
#include "headers/search.h"
#include "headers/testpos.h"

bool find_move(struct movelist_t, uint32_t);

void printpos(struct position_t);

int main(void)
{
	struct position_t testpos = START_POSITION;
	struct movelist_t ls;
	int i = 0;
	bool moves_found = true;
	bool noerr = true;
	printf("%s", "Starting move generation self-check\n\n");
	printf("%s", "Start position: \n");
	printpos(testpos);
	printf("%s", "Checking moves: \n");
	ls = generate_moves(&testpos);
	while (startpos_mv[i] != 0x0) {
		printf("%s%-15s%s%-16x%s", "Searching for move ", startpos_str[i],
				"hex: ", startpos_mv[i], "\t");
		if (find_move(ls, startpos_mv[i])) {
			printf("%s", "FOUND\n");
			i++;
			ls.nodes--;
		} else {
			printf("%s", "ERROR\n");
			moves_found = false;
			noerr = false;
			i++;
			ls.nodes--;
		}
	}
	testpos = fvb11Bg5;
	printf("%s", "\nFischer v Byrne 11.Bg5\n");
	printpos(testpos);
	printf("%s", "Checking moves: \n");
	ls = generate_moves(&testpos);
	i = 0;
	while (fvb11Bg5_mv[i] != 0x0) {
		printf("%s%-15s%s%-16x%s", "Searching for move ", fvb11Bg5_str[i],
				"hex: ", fvb11Bg5_mv[i], "\t");
		if (find_move(ls, fvb11Bg5_mv[i])) {
			printf("%s", "FOUND\n");
			i++;
			ls.nodes--;
		} else {
			moves_found = false;
			noerr = false;
			printf("%s", "ERROR\n");
			i++;
			ls.nodes--;
		}
	}
	testpos = kvt5Qd2;
	printf("%s", "\nKasparov v Topalov 5.Qd2\n");
	printpos(testpos);
	printf("%s", "Checking moves: \n");
	ls = generate_moves(&testpos);
	i = 0;
	while (kvt5Qd2_mv[i] != 0x0) {
		printf("%s%-15s%s%-16x%s", "Searching for move ", kvt5Qd2_str[i],
				"hex: ", kvt5Qd2_mv[i], "\t");
		if (find_move(ls, kvt5Qd2_mv[i])) {
			printf("%s", "FOUND\n");
			i++;
			ls.nodes--;
		} else {
			moves_found = false;
			noerr = false;
			printf("%s", "ERROR\n");
			i++;
			ls.nodes--;
		}
	}
	testpos = cvl56Rb8;
	printf("%s", "\nCarlsen v Liren 56.Rb8\n");
	printpos(testpos);
	printf("%s", "Checking moves: \n");
	ls = generate_moves(&testpos);
	i = 0;
	while (cvl56Rb8_mv[i] != 0x0) {
		printf("%s%-15s%s%-16x%s", "Searching for move ", cvl56Rb8_str[i],
				"hex: ", cvl56Rb8_mv[i], "\t");
		if (find_move(ls, cvl56Rb8_mv[i])) {
			printf("%s", "FOUND\n");
			i++;
			ls.nodes--;
		} else {
			moves_found = false;
			noerr = false;
			printf("%s", "ERROR\n");
			i++;
			ls.nodes--;
		}
	}
	testpos = lvs7Be7; 
	printf("%s", "\nLasker v Steinitz 7...Be7\n");
	printpos(testpos);
	printf("%s", "Checking moves: \n");
	ls = generate_moves(&testpos);
	i = 0;
	while (lvs7Be7_mv[i] != 0x0) {
		printf("%s%-15s%s%-16x%s", "Searching for move ", lvs7Be7_str[i],
				"hex: ", lvs7Be7_mv[i], "\t");
		if (find_move(ls, lvs7Be7_mv[i])) {
			printf("%s", "FOUND\n");
			i++;
			ls.nodes--;
		} else {
			moves_found = false;
			noerr = false;
			printf("%s", "ERROR\n");
			i++;
			ls.nodes--;
		}
	}
	if (noerr) {
		printf("%s", "Tests passed\n");
	} else if (moves_found) {
		printf("%s", "FAILED : Errors occured\n");
	} else {
		printf("%s", "FAILED : Not all moves were generated correctly\n");
	}
	return 0;
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

