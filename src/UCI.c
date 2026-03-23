#include "UCI.h"

int parse_move_string(Board* board, char* move_string) {
	int strLenght = strlen(move_string);

	int fromSq = (move_string[0] - 'a') + (move_string[1] - '1') * 8;
	int toSq = (move_string[2] - 'a') + (move_string[3] - '1') * 8;

	PieceType promotion = 0;
	if (strLenght > 4) {
		switch (move_string[4])
		{
			case 'q':
				promotion = QUEEN;
				break;
			case 'n':
				promotion = KNIGHT;
				break;
			case 'r':
				promotion = ROOK;
				break;
			case 'b':
				promotion = BISHOP;
				break;
		default:
			break;
		}
	}

	MoveList list = { 0 };
	if (board->sideToMove == WHITE) white_generate_pseudo_moves(board, &list);
	else black_generate_pseudo_moves(board, &list);

	for (int i = 0; i < list.count; i++) {
		int move = list.moves[i];
		
		if (GET_MOVE_SOURCE(move) == fromSq && GET_MOVE_TARGET(move) == toSq && GET_PROMOTION_PIECE(move) == promotion) {
			return move;
		}
	}

	return 0;
}

void uci_parse_pos(char* cmd, Board* board) {

	// Skip "position"
	char* ptr = cmd + 9;

	// StartPos indicates the standard initial position
	if (strncmp(ptr, "startpos", 8) == 0) {
		load_fen_board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", board);
	}
	// Fen represents a custom position
	else if (strncmp(ptr, "fen", 3) == 0) {
		// Skip "moves"
		ptr += 4;

		// The fen position can be followed by a moves set.
		char* moves_ptr = strstr(ptr, "moves");

		if (moves_ptr != NULL) {
			// Terminate the string earlier, as moves will be parsed later.
			*(moves_ptr - 1) = '\0';
		}

		load_fen_board(ptr, board);
	}

	char* moves_ptr = strstr(cmd, "moves");

	if (moves_ptr != NULL) {
		moves_ptr += 6;
		char* move_string = strtok(moves_ptr, " ");

		while (move_string != NULL) {
			int move = parse_move_string(board, move_string);

			if (move == 0) {
				break;
			}

			make_move(board, move, board->sideToMove, 0);

			move_string = strtok(NULL, " ");
		}
	}
}

#pragma warning(suppress : 6262)
void uci_protocol_handler(int depth) {
	char cmd[2048];
	Board board = { 0 };

	printf("GlueNG UCI ready\n");

	while (fgets(cmd, 2048, stdin))
	{
		cmd[strcspn(cmd, "\r\n")] = 0;

		if (strncmp(cmd, "quit", 4) == 0) {
			break;
		}
		else if (strncmp(cmd, "uci", 3) == 0) {
			printf("id name GlueNG\n");
			printf("id author Daggo\n");
			printf("uciok\n");
		}
		else if (strncmp(cmd, "isready", 7) == 0) {
			printf("readyok\n");
		}
		else if (strncmp(cmd, "position", 8) == 0) {
			uci_parse_pos(cmd, &board);
		}
		else if (strncmp(cmd, "go", 2) == 0) {

			int best = best_move(&board, depth);

			printf("bestmove ");
			debug_move(best);
			printf("\n");
		}

	}

}