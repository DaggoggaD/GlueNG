#include "FenFunctions.h"


PieceId select_fen_piece(const char piece) {
	switch (piece)
	{
	case 'P':
		return wP;
	case 'N':
		return wN;
	case 'B':
		return wB;
	case 'R':
		return wR;
	case 'Q':
		return wQ;
	case 'K':
		return wK;
	case 'p':
		return bP;
	case 'n':
		return bN;
	case 'b':
		return bB;
	case 'r':
		return bR;
	case 'q':
		return bQ;
	case 'k':
		return bK;
	case '/':
		return -1;
	default:
		return EMPTY; // Invalid piece character
		break;
	}

}

void place_fen_pieces(const char** fen, Board* board) {
	int index = 0;

	while (index < 64) {
		PieceId piece = select_fen_piece(**fen);

		if (piece == EMPTY) {
			int emptySquares = **fen - '0';
			if (emptySquares < 1 || emptySquares > 8) {
				fprintf(stderr, "Error: Invalid FEN string. Expected a digit between 1 and 8 for empty squares.\n");
				return;
			}
			index += emptySquares;
		}

		else if (piece != EMPTY && piece != -1) {
			SelectionColor color = (piece >= bP) ? BLACK : WHITE;
			PieceType type = (piece - 1) % 6;
			add_piece_to_board(type, color, index ^ 56, board);
			index++;
		}

		(*fen)++;
	}
}

void set_fen_castling_permissions(const char** fen, Board* board) {
	board->castlingPerms = 0;

	while (**fen != ' ' && *fen != '\0') {
		switch (**fen) {
		case 'K':
			board->castlingPerms |= WKCA;
			break;
		case 'Q':
			board->castlingPerms |= WQCA;
			break;
		case 'k':
			board->castlingPerms |= BKCA;
			break;
		case 'q':
			board->castlingPerms |= BQCA;
			break;
		}
		(*fen)++;
	}
}

void set_fen_en_passant(const char** fen, Board* board) {
	int index = -1;
	if (**fen == '-') {
		board->enPassant = -1; // No en passant square
		(*fen)++;
	}
	else {
		int file = **fen - 'a';
		int rank = (*fen)[1] - '1';
		if (file < 0 || file > 7 || rank < 0 || rank > 7) {
			fprintf(stderr, "Error: Invalid FEN string. En passant square must be between a1 and h8.\n");
			board->enPassant = -1;
			return;
		}

		index_from_rank_file(rank, file, &index);
		board->enPassant = index; // Convert to little endian index
		(*fen) += 2; // Move past the en passant square
	}
}

void set_fen_half_moves(const char* fen, Board* board) {
	board->halfMoves = 0;
	while (*fen >= '0' && *fen <= '9') {
		board->halfMoves = board->halfMoves * 10 + (*fen - '0');
		fen++;
	}
}

void load_fen_board(const char* fen, Board* board)
{
	place_fen_pieces(&fen, board);
	fen++; // Skip space after piece placement

	board->sideToMove = (*fen == 'w') ? WHITE : BLACK;
	fen += 2; // Skip space after side to move

	set_fen_castling_permissions(&fen, board);
	fen++; // Skip space after castling

	set_fen_en_passant(&fen, board);
	fen++; // Skip space after en passant

	set_fen_half_moves(fen, board);

}