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


int get_magic_index(U64 b, MagicEntry* entry);

void initialize_magic_bitboards();

#endif // !MAGIC_BITBOARDS_H
