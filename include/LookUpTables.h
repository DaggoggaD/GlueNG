#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include "Headers.h"

U64 knightAttackSquares[64];
U64 kingAttackSquares[64];
U64 pawnSingleAttacks[PlayerN][64];

U64 rookOccupanciesMasks[64];
U64 bishopOccupanciesMasks[64];

// Knight moves constants (from https://www.chessprogramming.org/Knight_Pattern)
extern const U64 notAFile;
extern const U64 notHFile;
extern const U64 notABFile;
extern const U64 notGHFile;

// Called on startup, initializes knight, king attack bitboards.
void initialize_pawn_single_attack_squares();
void initialize_knight_attack_squares();
void initialize_king_attack_squares();

// Called on startup, initializes rook and bishop occupancy masks.
void initialize_rook_masks();
void initialize_bishop_masks();


#endif // !LOOKUP_TABLES_H
