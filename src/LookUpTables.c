#include "LookUpTables.h"

// Knight/king moves and constants (from https://www.chessprogramming.org/Knight_Pattern)
const U64 notAFile = 0xfefefefefefefefeULL; // ~0x0101010101010101
const U64 notHFile = 0x7f7f7f7f7f7f7f7fULL; // ~0x8080808080808080

const U64 notABFile = 0xfcfcfcfcfcfcfcfcULL; // ~0x0303030303030303
const U64 notGHFile = 0x3f3f3f3f3f3f3f3fULL; // ~0xc0c0c0c0c0c0c0c0

const U64 not8thRank = 0x00ffffffffffffffULL; // ~0xff00000000000000
const U64 not1stRank = 0xffffffffffffff00ULL; // ~0x00000000000000ff

U64 knight_no_No_Ea(U64 b) { return (b & notHFile) << 17; }
U64 knight_no_Ea_Ea(U64 b) { return (b & notGHFile) << 10; }
U64 knight_so_Ea_Ea(U64 b) { return (b & notGHFile) >> 6; }
U64 knight_so_So_Ea(U64 b) { return (b & notHFile) >> 15; }
U64 knight_no_No_We(U64 b) { return (b & notAFile) << 15; }
U64 knight_no_We_We(U64 b) { return (b & notABFile) << 6; }
U64 knight_so_We_We(U64 b) { return (b & notABFile) >> 10; }
U64 knight_so_So_We(U64 b) { return (b & notAFile) >> 17; }


U64 king_No(U64 b) { return b << 8; }
U64 king_So(U64 b) { return b >> 8; }
U64 king_Ea(U64 b) { return (b & notHFile) << 1; }
U64 king_We(U64 b) { return (b & notAFile) >> 1; }
U64 king_No_Ea(U64 b) { return (b & notHFile) << 9; }
U64 king_So_Ea(U64 b) { return (b & notHFile) >> 7; }
U64 king_No_We(U64 b) { return (b & notAFile) << 7; }
U64 king_So_We(U64 b) { return (b & notAFile) >> 9; }


// Rook/bishops moves, learnt from https://github.com/maksimKorzh/bbc/blob/master/src/bbc_nnue/bbc.c
U64 calc_rook_occupancies_mask_on_square(int square) {
	U64 mask = 0ULL;
	int r;
	int f;
	rank_file_from_index(square, &r, &f);

	// Numbers from 1 to 6 because we want to exclude edge squares, 
	// which are not relevant for occupancy variations.

	for (int i = r + 1; i <= 6; i++) mask |= (1ULL << (i * 8 + f)); // N
	for (int i = r - 1; i >= 1; i--) mask |= (1ULL << (i * 8 + f)); // S
	for (int i = f + 1; i <= 6; i++) mask |= (1ULL << (r * 8 + i)); // E
	for (int i = f - 1; i >= 1; i--) mask |= (1ULL << (r * 8 + i)); // O

	return mask;
}

U64 calc_bishop_occupancies_mask_on_square(int square) {
	U64 mask = 0ULL;
	int r;
	int f;
	rank_file_from_index(square, &r, &f);

	// Numbers from 1 to 6 because we want to exclude edge squares, 
	// which are not relevant for occupancy variations.

	for (int i = 1; i <= 6 && r + i <= 6 && f + i <= 6; i++) mask |= (1ULL << ((r + i) * 8 + f + i)); // NE
	for (int i = 1; i <= 6 && r - i >= 1 && f + i <= 6; i++) mask |= (1ULL << ((r - i) * 8 + f + i)); // SE
	for (int i = 1; i <= 6 && r + i <= 6 && f - i >= 1; i++) mask |= (1ULL << ((r + i) * 8 + f - i)); // NO
	for (int i = 1; i <= 6 && r - i >= 1 && f - i >= 1; i++) mask |= (1ULL << ((r - i) * 8 + f - i)); // SO
	return mask;
}

// Calculates all the squares attacked by a king on a given square bitboard.
U64 calc_king_attacks_on_square(U64 b) {
	return king_No(b) | king_So(b) | king_Ea(b) | king_We(b) |
		king_No_Ea(b) | king_So_Ea(b) | king_No_We(b) | king_So_We(b);
}

// Calculates all the squares attacked by a knight on a given square bitboard.
U64 calc_knight_attacks_on_square(U64 b) {
	return knight_no_No_Ea(b) | knight_no_Ea_Ea(b) | knight_so_Ea_Ea(b) | knight_so_So_Ea(b) |
		knight_no_No_We(b) | knight_no_We_We(b) | knight_so_We_We(b) | knight_so_So_We(b);
}

void initialize_king_attack_squares() {
	for (int square = 0; square < 64; square++) {
		U64 bitboard = 1ULL << square;
		kingAttackSquares[square] = calc_king_attacks_on_square(bitboard);
	}
}

void initialize_knight_attack_squares() {
	for (int square = 0; square < 64; square++) {
		U64 bitboard = 1ULL << square;
		knightAttackSquares[square] = calc_knight_attacks_on_square(bitboard);
	}
}

void initialize_rook_masks() {
	for (int square = 0; square < 64; square++) {
		U64 bitboard = 1ULL << square;
		rookOccupanciesMasks[square] = calc_rook_occupancies_mask_on_square(square);
	}
}

void initialize_bishop_masks() {
	for (int square = 0; square < 64; square++) {
		U64 bitboard = 1ULL << square;
		bishopOccupanciesMasks[square] = calc_bishop_occupancies_mask_on_square(square);
	}
}