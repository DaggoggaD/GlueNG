#include "TranspTables.h"
// RKISS from chessprogramming.org, see https://www.chessprogramming.org/Bob_Jenkins#RKISS
TranspEntry* TT = NULL;
int current_age = 0;
int ttSize = 0;

U64 ran_val(ranctx* x) {
    U64 e = x->a - rot(x->b, 7);
    x->a = x->b ^ rot(x->c, 13);
    x->b = x->c + rot(x->d, 37);
    x->c = x->d + e;
    x->d = e + x->a;
    return x->d;
}

void ran_init(ranctx* x, U64 seed) {
    U64 i;
    x->a = 0xf1ea5eed, x->b = x->c = x->d = seed;
    for (i = 0; i < 20; ++i) {
        (void)ran_val(x);
    }
}

void init_random_keys() {
    for (int side = 0; side < 2; side++)
    {
        for (int piece = 0; piece < 6; piece++)
        {
			for (int square = 0; square < 64; square++) {
                pieceKeys[side][piece][square] = ran_val(&rng);
            }
        }
    }

	sideKey = ran_val(&rng);

    for (int i = 0; i < 16; i++)
    {
		castleKeys[i] = ran_val(&rng);
    }

    for (int i = 0; i < 8; i++)
    {
		enPassantKeys[i] = ran_val(&rng);
    }

}

U64 get_zobrist_code(Board* board) {
    U64 code = 0;
    // Pieces
    for (int side = 0; side < 2; side++)
    {
        for (int piece = 0; piece < 6; piece++)
        {
            U64 bitboard = board->pieceBitboards[side][piece];
            unsigned long square = 0;
            while (bitboard) {
                _BitScanForward64(&square, bitboard);
                code ^= pieceKeys[side][piece][square];
                bitboard &= bitboard - 1;
            }
        }
    }

    // Side to move
    if (board->sideToMove == BLACK) {
        code ^= sideKey;
    }
    
    // Castling rights
    int castleIndex = 0;
    castleIndex |= board->castlingPerms;
    code ^= castleKeys[castleIndex];

    // En passant
    if (board->enPassant != -1) {
        int file = board->enPassant % 8;
        code ^= enPassantKeys[file];
    }

    return code;
}

void init_tt(int sizeMB) {
    ttSize = (sizeMB * 1024 * 1024) / sizeof(TranspEntry);
    
    if (TT != NULL) {
        free(TT);
    }

    TT = (TranspEntry*)calloc(ttSize, sizeof(TranspEntry));
}

void store_tt_entry(U64 key, int depth, int eval, HashFlag flag, int bestMove) {
    if (eval > 900000 || eval < -900000) return;
    TranspEntry* entry = &TT[get_hash_index(key)];
    
    // Exact match of key
    if (entry->key == key) {
        if (depth >= entry->depth) {
            entry->depth = depth;
            entry->eval = eval;
            entry->flag = flag;
            entry->bestMove = bestMove;
            entry->ancient = current_age;
        }
        else {
			entry->ancient = current_age;
        }
        return;
    }

    if (entry->key == 0 || entry->ancient != current_age || depth >= entry->depth) {
        entry->key = key;
        entry->depth = depth;
        entry->eval = eval;
        entry->flag = flag;
        entry->bestMove = bestMove;
        entry->ancient = current_age;
    }

}