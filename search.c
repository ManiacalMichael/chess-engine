#include "headers/chess.h"
#include "headers/search.h"

unsigned long long perft(struct position_t *posPtr, int depth)
{
	uint16_t movelist[MAX_MOVES + 1];
	unsigned long long total = 0;
	int color = (posPtr->flags & WHITE_TO_MOVE) ? WHITE : BLACK;
	movelist[0] = 0;
	generate_moves(*posPtr, movelist);
	for (int i = 1; i <= movelist[0]; i++) 
		if ((movelist[i] & END_SQUARE) == posPtr->kingpos[BLACK - color])
			return 0;
	if (depth == 1)
		return movelist[0];
	for (int i = 1; i <= movelist[0]; i++) {
		make_move(posPtr, movelist[i]);
		total += perft(posPtr, depth - 1);
		unmake_move(posPtr, movelist[i]);
	}
	return total;
}
