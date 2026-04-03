#ifndef CHESS_DEFINITIONS_H
#define CHESS_DEFINITIONS_H



#include <stdbool.h>
#include <stdio.h>

typedef unsigned long long U64;

// Standard piece and square definitions taken from chess programming wiki
typedef enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NONE } PieceType;

typedef enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK } PieceId;

typedef enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_NONE } FileId;

typedef enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE } RankId;

typedef enum { WHITE, BLACK, BOTH } SelectionColor;


// Square IDs for a standard 8x8 chessboard, with NO_SQ representing an invalid square
typedef enum {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NO_SQ
} SquareId;

extern const int castlingMask[64];


// Castling permissions (represented as bit flags)
typedef enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8 } CastlingPermissions;

#endif // !CHESS_DEFINITIONS_H