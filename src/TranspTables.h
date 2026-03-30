#pragma once
#ifndef TRANSP_TABLES
#include "Headers.h"
#define STANDARD_HASH_SIZE_MB 64


// RKISS Implementation at https://www.chessprogramming.org/Bob_Jenkins#RKISS
// Bit rotation, making sure no left shifting losses
#define rot(x,k) (((x)<<(k))|((x)>>(64-(k))))

U64 pieceKeys[2][6][64]; // [color][piece][square]
U64 sideKey;             // SideToMove info
U64 castleKeys[16];      // 16 = 2^4 (all castle combinations)
U64 enPassantKeys[8];    // enPassant for each file

typedef struct ranctx {
	U64 a; U64 b; U64 c; U64 d; 
} ranctx;

typedef enum {
	EXACT,
	ALPHA,
	BETA
} HashFlag;

typedef struct {
	U64 key;
	int depth;
	int eval;
	HashFlag flag;
	int bestMove;
	int ancient; // see later
} TranspEntry;

extern TranspEntry* TT;
extern int current_age;
extern int ttSize;

ranctx rng;

U64 ran_val(ranctx* x);

void ran_init(ranctx* x, U64 seed);

void init_random_keys();

U64 get_zobrist_code(Board* board);

static inline int get_hash_index(U64 hashKey) {
	return hashKey % ttSize;
}

void init_tt(int sizeMB);

void store_tt_entry(U64 key, int depth, int eval, HashFlag flag, int bestMove);

#endif // !TRANSP_TABLES
