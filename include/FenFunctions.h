#ifndef FEN_FUNCTIONS_H
#define FEN_FUNCTIONS_H

#include "Headers.h"

// Translates a FEN piece character to the corresponding PieceId enum value.
// -1 : '/' (separation char), EMPTY : invalid/int character
PieceId select_fen_piece(const char piece);

// Parses the piece placement portion of a FEN string and updates the board's bitboards accordingly.
void place_fen_pieces(const char** fen, Board* board);

// Parses (max 4) char representing castling permissions in 2's power.
void set_fen_castling_permissions(const char** fen, Board* board);

// Parses the en passant square, converts it to little endian index.
void set_fen_en_passant(const char** fen, Board* board);

// Parses the halfmove clock. Full move is ignored.
void set_fen_half_moves(const char* fen, Board* board);

// Loads a FEN string into the given board structure.
// Since FEN is big_endian, it's translated to little_endian.
void load_fen_board(const char* fen, Board* board);

#endif // !FEN_FUNCTIONS_H
