/**
 * @file FenFunctions.h
 * @brief Function declarations for parsing and loading FEN strings.
 */
#ifndef FEN_FUNCTIONS_H
#define FEN_FUNCTIONS_H

#include "Headers.h"

 /**
  * @brief Translates a FEN piece character to the corresponding PieceId enum value.
  * @param piece The character representing the piece in FEN format.
  * @return The corresponding PieceId. Returns -1 for '/' (separation char),
  * and EMPTY for an invalid/int character.
  */
PieceId select_fen_piece(const char piece);

/**
 * @brief Parses the piece placement portion of a FEN string and updates the board's bitboards accordingly.
 * @param fen Pointer to the FEN string pointer (advanced as it parses).
 * @param board Pointer to the board state to update.
 */
void place_fen_pieces(const char** fen, Board* board);

/**
 * @brief Parses (max 4) characters representing castling permissions in powers of 2.
 * @param fen Pointer to the FEN string pointer.
 * @param board Pointer to the board state to update.
 */
void set_fen_castling_permissions(const char** fen, Board* board);

/**
 * @brief Parses the en passant square and converts it to a little-endian index.
 * @param fen Pointer to the FEN string pointer.
 * @param board Pointer to the board state to update.
 */
void set_fen_en_passant(const char** fen, Board* board);

/**
 * @brief Parses the halfmove clock.
 * @note Full move clock is currently ignored.
 * @param fen Pointer to the FEN string.
 * @param board Pointer to the board state to update.
 */
void set_fen_half_moves(const char* fen, Board* board);

/**
 * @brief Loads a complete FEN string into the given board structure.
 * @note Since FEN is big-endian, it's translated to little-endian internally.
 * @param fen The full FEN string to parse.
 * @param board Pointer to the board state to populate.
 */
void load_fen_board(const char* fen, Board* board);

#endif // !FEN_FUNCTIONS_H
