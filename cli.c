#include <stdio.h>
#include <stdlib.h>
#include "headers/chess.h"
#include "headers/search.h"
#include "headers/interface.h"

position_t internal_position;

char[MAXSTRINGLENGTH] command_string;
char[MAXTOKENS][MAXTOKENLENGTH] command_tokens;

const char empty_board[][] = {
"+---+---+---+---+---+---+---+---+\n",
"|   |   |   |   |   |   |   |   |\n",
"+---+---+---+---+---+---+---+---+\n",
"|   |   |   |   |   |   |   |   |\n",
"+---+---+---+---+---+---+---+---+\n",
"|   |   |   |   |   |   |   |   |\n",
"+---+---+---+---+---+---+---+---+\n",
"|   |   |   |   |   |   |   |   |\n",
"+---+---+---+---+---+---+---+---+\n",
"|   |   |   |   |   |   |   |   |\n",
"+---+---+---+---+---+---+---+---+\n",
"|   |   |   |   |   |   |   |   |\n",
"+---+---+---+---+---+---+---+---+\n",
"|   |   |   |   |   |   |   |   |\n",
"+---+---+---+---+---+---+---+---+\n",
"|   |   |   |   |   |   |   |   |\n",
"+---+---+---+---+---+---+---+---+\n";

/* positions to write pieces onto display board */
/* [SQUARES][0=row, 1=column] */
const int square_index[][] = {
	{15, 3}, {15, 7}, {15, 11}, {15, 15},
	{15, 19}, {15, 23}, {15, 27}, {15, 31}},
      {
	{13, 3}, {13, 7}, {13, 11}, {13, 15},
	{13, 19}, {12, 23}, {13, 27}, {13, 31}},
      {
	{11, 3}, {11, 7}, {11, 11}, {11, 15},
	{11, 19}, {11, 23}, {11, 27} {11, 31}},
      {
	{9, 3}, {9, 7}, {9, 11}, {9, 15},
	{9, 19}, {9, 23}, {9, 27}, {9, 31}},
      {
	{7, 3}, {7, 7}, {7, 11}, {7, 15},
	{7, 19}, {7, 23}, {7, 27}, {7, 31}},
      {
	{5, 3}, {5, 7}, {5, 11}, {5, 15},
	{5, 19}, {5, 23}, {5, 27}, {5, 31}},
      {
	{3, 3}, {3, 7}, {3, 11}, {3, 15},
	{3, 19}, {3, 23}, {3, 27}, {3, 31}},
      {
	{1, 3}, {1, 7}, {1, 11}, {1, 15},
	{1, 19}, {1, 23}, {1, 27}, {1, 31}};

const char piece_letters[] = {
	' ', ' ',
	'P', 'p',
	'N', 'n',
	'B', 'b',
	'R', 'r',
	'Q', 'q',
	'K', 'k',
	' ', ' '
};

void display(void)
{
	char board[17][35];
	memcpy(empty_board, board, sizeof empty_board);
	for (int j = 0; j < 64; ++j)
		for (int i = 0; i < 16; ++i)
			if (internal_position.bboards[i] & (1 << j))
				board[square_index[j][0]][square_index[j][1]] = piece_letters[i];
	write(stdout, board, sizeof board);
}
