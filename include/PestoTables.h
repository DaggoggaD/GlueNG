#pragma once
#ifndef PESTO_TABLES_H
#define PESTO_TABLES_H

// Values taken from standard PeSTO implementation at:
// https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function

extern int mg_pawn_table[64];
extern int eg_pawn_table[64];
extern int mg_knight_table[64];
extern int eg_knight_table[64];
extern int mg_bishop_table[64];
extern int eg_bishop_table[64];
extern int mg_rook_table[64];
extern int eg_rook_table[64];
extern int mg_queen_table[64];
extern int eg_queen_table[64];
extern int mg_king_table[64];
extern int eg_king_table[64];

extern const int* mg_pesto_table[6];
extern const int* eg_pesto_table[6];

#endif // !PESTO_TABLES_H