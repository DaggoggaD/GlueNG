#include "GlueNG.h"

void initialize() {
	initialize_knight_attack_squares();
	initialize_king_attack_squares();

	initialize_rook_masks();
	initialize_bishop_masks();

	initialize_magic_bitboards();
}

void make_move(Board* board) {


}

int main()
{
	initialize();
	Board board = { 0 };

	load_fen_board("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3", &board);
	debug_board_visualizer(&board);

	MoveList list = { 0 };
	white_generate_pseudo_moves(&board, &list);

	debug_move_list(&list);
}