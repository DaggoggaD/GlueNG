#ifndef MAGIC_BITBOARDS_H
#define MAGIC_BITBOARDS_H

#include "Headers.h"

// Magic bitboards learnt from "https://analog-hors.github.io/site/magic-bitboards/"
// and https://www.chessprogramming.org/Magic_Bitboards

typedef struct mEntry {
	U64 occupanciesMask;
	U64 magicN;
	unsigned short indexBits;
} MagicEntry;

MagicEntry RookMagics[BRD_SIZE];
MagicEntry BishopMagics[BRD_SIZE];

U64 RookMoves[BRD_SIZE][4096];
U64 BishopMoves[BRD_SIZE][512];

// Given a blockers configuration and a magic entry, it calculates the magic index 
// by applying the magic multiplication and the appropriate shift.
// [b]: blockers configuration bitboard. 
// [entry]: magic entry containing the occupancies mask, the magic number and the index bits.
int get_magic_index(U64 b, MagicEntry* entry);

// Counts the number of all active bits in a bitboard.
// Note: could use __popcnt64. Though all the time it's called
// is during initialization, so it is not a performance concern.
int count_active_bits(U64 b);

// Given a mask and a patternID, it sets the bits in the occupancies 
// bitboard according to the active bits in the mask and the patternID.
// [mask]: pre-calculated bitboard, all possible positions without blockers and edges. 
// [patternID]: number from 0 to 2^(active bits in the mask) - 1, used to set the 
// corresponding bits in the occupancies bitboard.
U64 set_occupancies_mapping(U64 mask, int patternID);

// Calculates all the attack rays for a rook, starting on the given square, 
// and with the given blockers configuration. It does count also the blockers squares as attacked, 
/// [square]: rook position, 0-63 index. [blockers]: bitboard with blockers positions.
U64 calc_rook_attack_rays(int square, U64 blockers);

// Same as for "calc_rook_attack_rays", but for bishops.
/// [square]: bishop position, 0-63 index. [blockers]: bitboard with blockers positions.
U64 calc_bishop_attack_rays(int square, U64 blockers);

// Initializes both the rook, bishop and queen magic entries and attack tables.
// NOTE: could easily be optimized by merging the loops, but it is more clear to keep them separated,
// and the initialization is only done once on startup, so it is not a problem.
void initialize_magic_bitboards();

#endif // !MAGIC_BITBOARDS_H
