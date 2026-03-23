#include "GlueNG.h"
#include <time.h>

void initialize() {
	// Initialize attacks for "jumping" pieces
	initialize_pawn_single_attack_squares();
	initialize_knight_attack_squares();
	initialize_king_attack_squares();

	// Initialize masks for sliding pieces
	initialize_rook_masks();
	initialize_bishop_masks();

	// Initialize "magic" bitboards for sliding pieces
	initialize_magic_bitboards();
}

#pragma warning(suppress : 6262)
int main()
{
    initialize();
    
    char fen_input[256];

    printf("GlueNG ready. Paste fen string.\n");
    printf("'Quit' to exit.\n");

    while (1) {
        Board board = { 0 };
        printf("\nFEN > ");
        if (fgets(fen_input, sizeof(fen_input), stdin) == NULL) break;

        fen_input[strcspn(fen_input, "\n")] = 0;

        if (strcmp(fen_input, "quit") == 0) break;

        load_fen_board(fen_input, &board);
        debug_board_visualizer(&board);


        int best = best_move(&board, 7);

        printf("move played: \n");
        debug_move(best);
        printf("\n");
    }

	return 0;
}