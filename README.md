# chess-engine
As my highly creative repo name explains, this is my pet project: a chess engine written in C.
The main objective is to learn things, so feel free to gently chastise me for any mistakes you may find
# todo list
* Stop testing legality of moves in move generation 
 * This can be done in search by causing an artifical beta cutoff if a king is captured
* Implement Perft testing
 * Make sure to validate moves, maybe using something simmilar to the example above
* Expand board representation to use two color boards and six piece boards
* Switch to 16-bit move encoding
* Clean up list functions
* Move ordering/Selection sort
 * PV (implement this later)
 * Fail-Higher (also later)
 * Promotions
 * Winning captures
 * Equal captures
 * Losing captures 
 * Other
* Static Exchange Evaluation
 * Add functions: Minimalistic makemove, find attackers to square
* Make functions monochrome, swap colors on makemove, all functions from white perspective
 * Keep flag on position to keep track of actual player to move
