#ifndef INCLUDE_SEARCH_H
#define INCLUDE_SEARCH_H

#include <stdint.h>
#include "headers/chess.h"

/* Recursively calculates the number of movepaths from the specified position
 * to the given depth
 */
unsigned long long perft(position_t*, int);

#endif
