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

			if (type == KING) {

				if (color == WHITE) board->kingSq[WHITE] = index ^ 56;
				else board->kingSq[BLACK] = index ^ 56;

			}

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

void set_piece_on_square(Board *board) {
	U64 pawns = board->pieceBitboards[WHITE][PAWN] | board->pieceBitboards[BLACK][PAWN];
	U64 knights = board->pieceBitboards[WHITE][KNIGHT] | board->pieceBitboards[BLACK][KNIGHT];
	U64 bishops = board->pieceBitboards[WHITE][BISHOP] | board->pieceBitboards[BLACK][BISHOP];
	U64 rooks = board->pieceBitboards[WHITE][ROOK] | board->pieceBitboards[BLACK][ROOK];
	U64 queens = board->pieceBitboards[WHITE][QUEEN] | board->pieceBitboards[BLACK][QUEEN];
	U64 king = board->pieceBitboards[WHITE][KING] | board->pieceBitboards[BLACK][KING];


	for (int i = 0; i < BRD_SIZE; i++)
	{
		U64 searchedIndex = 1ULL << i;

		if (pawns & searchedIndex) {
			board->pieceOnSquare[i] = PAWN;
			continue;
		}
		else if (knights & searchedIndex) {
			board->pieceOnSquare[i] = KNIGHT;
			continue;
		}
		else if (rooks & searchedIndex) {
			board->pieceOnSquare[i] = ROOK;
			continue;
		}
		else if (queens & searchedIndex) {
			board->pieceOnSquare[i] = QUEEN;
			continue;
		}
		else if (bishops & searchedIndex) {
			board->pieceOnSquare[i] = BISHOP;
			continue;
		}
		else if (king & searchedIndex) {
			board->pieceOnSquare[i] = KING;
			continue;
		}
		else {
			board->pieceOnSquare[i] = NONE;
			continue;
		}


	}

}

void load_fen_board(const char* fen, Board* board)
{
	memset(board, 0, sizeof(Board));

	place_fen_pieces(&fen, board);
	fen++;

	board->sideToMove = (*fen == 'w') ? WHITE : BLACK;
	fen += 2;

	set_fen_castling_permissions(&fen, board);
	fen++;

	set_fen_en_passant(&fen, board);
	fen++;

	set_fen_half_moves(fen, board);

	set_piece_on_square(board);

}