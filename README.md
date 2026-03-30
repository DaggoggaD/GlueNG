# GlueNG

GlueNG is a C-based chess engine developed for educational purposes to explore the classic architectures and algorithms of chess programming. It implements the UCI (Universal Chess Interface) protocol, making it compatible with major GUIs (e.g., Cutechess, Arena, MATE).

## Architecture and Representation

*   **Bitboards:** The chessboard is represented internally using 64-bit integers (U64). This approach guarantees extreme efficiency in move calculation and evaluation through bitwise operations (AND, OR, XOR).
*   **Move Generation:** Pseudo-legal move generator, with move legality validation performed contextually during the search (checking for checks after execting the move). It supports all standard rules: castling, en passant, promotions.
*   **Zobrist Hashing:** A 64-bit incremental hashing system used to uniquely identify the board state (including castling rights and en passant files). This is fundamental for the functioning of the engine's long-term memory.

## Search Algorithms

*   **NegaMax with Alpha-Beta Pruning:** The core algorithm for traversing the move tree. It utilizes a Fail-Soft logic to progressively narrow the search window.
*   **Iterative Deepening:** The search is not executed at a fixed depth in a single pass; instead, it starts at depth 1 and increments iteratively. This ensures optimal time management and provides better move ordering for shallow nodes.
*   **Quiescence Search:** To mitigate the Horizon Effect, at the end of the standard search depth, the engine continues to expand branches containing captures and promotions, stabilizing the static evaluation function.
*   **Check Extensions:** The search depth is dynamically extended by 1 ply when the engine is in check, preventing it from ignoring critical tactical sequences or imminent checkmates.

## Move Ordering

The efficiency of Alpha-Beta pruning heavily relies on move ordering. GlueNG orders generated moves by assigning pre-search scores:
1.  **TT Move:** The best move previously found in the Transposition Table for the current position is given absolute priority.
2.  **Promotions:** Moves leading to a promotion receive a bonus proportional to the promoted piece.
3.  **MVV-LVA (Most Valuable Victim - Least Valuable Attacker):** Captures are preferentially ordered by evaluating the difference in value between the prey and the attacker (e.g., a Pawn capturing a Queen is evaluated first).

## Transposition Table (TT)

The engine implements a hash table to store evaluations of previously explored positions, eliminating recalculation times in the event of move transpositions.
*   Nodes are saved as EXACT (true value), ALPHA (Fail-Low, Upper Bound), or BETA (Fail-High, Lower Bound).
*   Replacement strategy is based on the saved search depth compared to the requested depth.
*   Age management is used to distinguish and overwrite positions from previous games.
*   Supports the `ucinewgame` command to safely reset the table during automated tournaments.

## Evaluation Function

The static board evaluation returns a score in centipawns (cp) based on the perspective of the side to move.
*   **Material:** Standard piece weight calculation.
*   **Piece-Square Tables (PST):** Encourages the development of pieces towards ideal squares (e.g., knights in the center, controlling the center with pawns in the opening).
*   **Endgame Recognition:** Dynamic interpolation between opening/middlegame evaluations and endgame evaluations based on remaining material.
*   **Mating Net (Edge Attraction):** In the endgame, the engine calculates the Manhattan distance between the Kings and assigns bonuses to force the opponent's King towards the edge of the board, facilitating checkmate.

## UCI Protocol and Usage

GlueNG communicates via standard input/output using the UCI standard.
Supported commands include:
*   `uci`, `isready`
*   `position startpos [moves ...]`, `position fen ... [moves ...]`
*   `go wtime [ms] btime [ms] winc [ms] binc [ms]`
*   `ucinewgame`
*   `d` (Debug command to print the visual board representation to the terminal)

Note: this .md file was ai-generated. It'll be fixed when the project releases.
