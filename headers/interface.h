#ifndef INCLUDE_INTERFACE_H
#define INCLUDE_INTERFACE_H

#include "headers/chess.h"
#include "headers/search.h"

#define MAXTOKENS 256
#define MAXTOKENLENGTH 128
#define MAXSTRINGLENGTH 33024 /* MAXTOKENS * (MAXTOKENLENGTH + 1) */

extern position_t internal_position;


/* Decodes a sieres of moves into a position */
position_t movesdecode(char**);

/* Decodes a FEN string into a position */
position_t fendecode(char**);

/* Attempts to decode a string into a move */
uint16_t movedecode(const position_t, char*);

/* Encodes a move as a string */
void moveencode(char*, uint16_t);


/* Parse a command string into seperate tokens */
void parse(char*, char**);

/* Attempts to read the first token in the array as a command and executes it,
 * otherwise, attempts to read the first token as a move to be played
 */
void execute(char**);


/* Runs divide perft to the specified depth on the internal position */
void divide(int);

/* Prints a visualization of the internal position */
void display(void);

/* Sets the internal position to the one specified in the array.
 * The zeroth string should be either "fen" or "startpos"
 */
void uci_position(char**);

#endif
