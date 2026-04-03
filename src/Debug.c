#include "../include/Debug.h"

void debug_select_string_from_piece(const PieceId piece, char* str)
{
	switch (piece)
	{
	case wP:
		strcpy(str, "wP");
		break;
	case wN:
		strcpy(str, "wN");
		break;
	case wB:
		strcpy(str, "wB");
		break;
	case wR:
		strcpy(str, "wR");
		break;
	case wQ:
		strcpy(str, "wQ");
		break;
	case wK:
		strcpy(str, "wK");
		break;
	case bP:
		strcpy(str, "bP");
		break;
	case bN:
		strcpy(str, "bN");
		break;
	case bB:
		strcpy(str, "bB");
		break;
	case bR:
		strcpy(str, "bR");
		break;
	case bQ:
		strcpy(str, "bQ");
		break;
	case bK:
		strcpy(str, "bK");
		break;
	default:
		strcpy(str, "");
		break;
	}
}

void debug_board_visualizer(Board* board) {
	printf("+----+----+----+----+----+----+----+----+\n");

	for (int rank = 7; rank >= 0; rank--) {
		printf("|");
		for (int file = 0; file < 8; file++) {
			int index;
			index_from_rank_file(rank, file, &index);
			char pieceStr[4] = "  ";
			int pieceFound = 0;
			for (int color = WHITE; color <= BLACK; color++) {
				for (int type = PAWN; type <= KING; type++) {
					if (board->pieceBitboards[color][type] & (1ULL << index)) {
						debug_select_string_from_piece((color == WHITE ? wP : bP) + type, pieceStr);
						pieceFound = 1;
						break;
					}
				}
				if (pieceFound) break;
			}
			if (index == board->enPassant) {
				strcat(pieceStr, "*");
				printf(" %s|", pieceStr);
			}
			else printf(" %s |", pieceStr);
		}
		printf("\n+----+----+----+----+----+----+----+----+\n");
	}

	printf("Side to move: %s\n", (board->sideToMove == WHITE) ? "White" : "Black");
	printf("Castling permissions: %s%s%s%s\n",
		(board->castlingPerms & WKCA) ? "K" : "",
		(board->castlingPerms & WQCA) ? "Q" : "",
		(board->castlingPerms & BKCA) ? "k" : "",
		(board->castlingPerms & BQCA) ? "q" : ""
	);
	printf("Halfmove clock: %d\n", board->halfMoves);

}

void debug_bitmask_visualizer(U64 bitmask) {
	for (int rank = 7; rank >= 0; rank--) {
		for (int file = 0; file < 8; file++) {
			int index;
			index_from_rank_file(rank, file, &index);
			printf("%c ", (bitmask & (1ULL << index)) ? 'X' : '.');
		}
		printf("\n");
	}
}

void debug_precomputed_attack_visualizer(int square, PieceType type) {
	for (int rank = 7; rank >= 0; rank--) {
		for (int file = 0; file < 8; file++) {
			int index;
			index_from_rank_file(rank, file, &index);

			if (index == square) {
				printf("S ");
			}
			else if(type == KNIGHT) {
				 printf("%c ", (knightAttackSquares[square] & (1ULL << index)) ? 'X' : '.');
			}
			else if (type == KING) {
				printf("%c ", (kingAttackSquares[square] & (1ULL << index)) ? 'X' : '.');
			}
			else if (type == ROOK) {
				printf("%c ", (rookOccupanciesMasks[square] & (1ULL << index)) ? 'X' : '.');
			}
			else if (type == BISHOP) {
				printf("%c ", (bishopOccupanciesMasks[square] & (1ULL << index)) ? 'X' : '.');
			}
			 else {
				printf(". ");
			}

			

		}
		printf("\n");
	}
}

void debug_move(int move) {
	int from = GET_MOVE_SOURCE(move);
	int to = GET_MOVE_TARGET(move);
	int promoted = GET_PROMOTION_PIECE(move);
	int isEp = GET_MOVE_EN_PASSANT(move);

	char fromFile = (from % 8) + 'a';
	char fromRank = (from / 8) + '1';
	char toFile = (to % 8) + 'a';
	char toRank = (to / 8) + '1';

	printf("%c%c%c%c", fromFile, fromRank, toFile, toRank);
	if (promoted != 0) {
		if (promoted == QUEEN) printf("q");
		else if (promoted == ROOK) printf("r");
		else if (promoted == BISHOP) printf("b");
		else if (promoted == KNIGHT) printf("n");
	}
}

void debug_move_list(MoveList* list) {
	printf("Moves: %d\n", list->count);

	for (int i = 0; i < list->count; i++) {
		int move = list->moves[i];

		int from = GET_MOVE_SOURCE(move);
		int to = GET_MOVE_TARGET(move);
		int promoted = GET_PROMOTION_PIECE(move);
		int isEp = GET_MOVE_EN_PASSANT(move);

		char fromFile = (from % 8) + 'a';
		char fromRank = (from / 8) + '1';
		char toFile = (to % 8) + 'a';
		char toRank = (to / 8) + '1';

		printf("%d. %c%c%c%c", i + 1, fromFile, fromRank, toFile, toRank);

		if (promoted) {
			char prom_char = '?';
			if (promoted == QUEEN) prom_char = 'q';
			else if (promoted == ROOK) prom_char = 'r';
			else if (promoted == BISHOP) prom_char = 'b';
			else if (promoted == KNIGHT) prom_char = 'n';
			printf("=%c", prom_char);
		}

		if (isEp) {
			printf(" (ep)");
		}

		printf("\n");
	}
	printf("\n");
}