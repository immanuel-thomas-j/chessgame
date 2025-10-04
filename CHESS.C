#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ------------ ANSI Color Definitions ------------
#define ANSI_COLOR_RESET    "\x1b[0m"
#define ANSI_COLOR_WHITE    "\x1b[33m" // Yellow for White pieces
#define ANSI_COLOR_BLACK    "\x1b[34m" // Blue for Black pieces
#define ANSI_COLOR_BORDER   "\x1b[90m" // Bright Black (Gray) for borders

// ------------ Captured Pieces Data Structure ------------
char captured_by_white[16] = {0};
int captured_by_white_count = 0;
char captured_by_black[16] = {0};
int captured_by_black_count = 0;

// ------------ Move Stack (DS Concept) ------------
// Used for implementing the 'undo' feature.
typedef struct {
    int r1, c1, r2, c2;
    char captured;
    char original_piece; // Stores the piece that moved (for promotion undo)
} Move;

Move stack[100]; // Array acting as the stack
int top = -1;    // Stack pointer

// ------------ Chess Board ------------
char board[8][8] = {
    {'r','n','b','q','k','b','n','r'},
    {'p','p','p','p','p','p','p','p'},
    {'.','.','.','.','.','.','.','.'},
    {'.','.','.','.','.','.','.','.'},
    {'.','.','.','.','.','.','.','.'},
    {'.','.','.','.','.','.','.','.'},
    {'P','P','P','P','P','P','P','P'},
    {'R','N','B','Q','K','B','N','R'}
};

// Pushes a move onto the stack
void pushMove(int r1, int c1, int r2, int c2, char captured) {
    if(top < 99) {
        stack[++top].r1 = r1;
        stack[top].c1 = c1;
        stack[top].r2 = r2;
        stack[top].c2 = c2;
        stack[top].captured = captured;
        stack[top].original_piece = board[r1][c1];
    }
}

// Pops the last move from the stack
Move popMove() {
    if (top >= 0) {
        return stack[top--];
    }
    // Return an invalid move to signal an empty stack
    return (Move){-1, -1, -1, -1, '.', '.'}; 
}

// ------------ Get Chess Piece Symbol ------------
const char* getPiece(char p) {
    switch(p) {
        case 'K': return "♔";
        case 'Q': return "♕";
        case 'R': return "♖";
        case 'B': return "♗";
        case 'N': return "♘";
        case 'P': return "♙";
        case 'k': return "♚";
        case 'q': return "♛";
        case 'r': return "♜";
        case 'b': return "♝";
        case 'n': return "♞";
        case 'p': return "♟";
        default: return " ";
    }
}

// ------------ Helper Functions ------------
int isWhite(char p) { return p >= 'A' && p <= 'Z'; }
int isBlack(char p) { return p >= 'a' && p <= 'z'; }

// ------------ Find King Position ------------
// Finds the position of the king for a given color.
int findKing(char color, int *kr, int *kc) {
    char king = (color == 'W') ? 'K' : 'k';
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if(board[i][j] == king) {
                *kr = i;
                *kc = j;
                return 1;
            }
        }
    }
    return 0;
}

// ------------ Piece-Specific Move Validation ------------
// Checks if the move is physically legal for the piece type (not considering checks/pins).
int isLegalPieceMove(int r1, int c1, int r2, int c2) {
    char piece = board[r1][c1];
    char target = board[r2][c2];
    int dr = abs(r2 - r1);
    int dc = abs(c2 - c1);
    char color = isWhite(piece) ? 'W' : 'B';
    
    // Check if the target square contains a friendly piece
    if (target != '.') {
        if ((color == 'W' && isWhite(target)) || (color == 'B' && isBlack(target))) {
            return 0;
        }
    }

    switch (toupper(piece)) {
        case 'P': { // Pawn movement logic
            int direction = (color == 'W') ? -1 : 1;
            int startRow = (color == 'W') ? 6 : 1;
            // Single step forward
            if (c1 == c2 && r2 - r1 == direction) {
                return board[r2][c2] == '.';
            }
            // Double step forward (only from starting row)
            if (c1 == c2 && r2 - r1 == 2 * direction && r1 == startRow) {
                return board[r2][c2] == '.' && board[r1 + direction][c1] == '.';
            }
            // Capture diagonal
            if (abs(c2 - c1) == 1 && r2 - r1 == direction) {
                return board[r2][c2] != '.'; // Must capture a piece
            }
            return 0;
        }
        case 'N': return (dr == 1 && dc == 2) || (dr == 2 && dc == 1); // Knight (L-shape)
        case 'B': { // Bishop (Diagonal movement and path clear check)
            if (dr != dc) return 0;
            int rStep = (r2 > r1) ? 1 : -1;
            int cStep = (c2 > c1) ? 1 : -1;
            for (int r = r1 + rStep, c = c1 + cStep; r != r2; r += rStep, c += cStep) {
                if (board[r][c] != '.') return 0;
            }
            return 1;
        }
        case 'R': { // Rook (Straight movement and path clear check)
            if (dr != 0 && dc != 0) return 0;
            if (dr == 0) { // Horizontal movement
                int cStep = (c2 > c1) ? 1 : -1;
                for (int c = c1 + cStep; c != c2; c += cStep) {
                    if (board[r1][c] != '.') return 0;
                }
            } else { // Vertical movement
                int rStep = (r2 > r1) ? 1 : -1;
                for (int r = r1 + rStep; r != r2; r += rStep) {
                    if (board[r][c1] != '.') return 0;
                }
            }
            return 1;
        }
        case 'Q': { // Queen (Rook + Bishop movement logic)
            if (dr == dc || dr == 0 || dc == 0) {
                if (dr == dc && dr != 0) { // Diagonal check
                    int rStep = (r2 > r1) ? 1 : -1;
                    int cStep = (c2 > c1) ? 1 : -1;
                    for (int r = r1 + rStep, c = c1 + cStep; r != r2; r += rStep, c += cStep) {
                        if (board[r][c] != '.') return 0;
                    }
                    return 1;
                }
                if (dr == 0) { // Horizontal check
                    int cStep = (c2 > c1) ? 1 : -1;
                    for (int c = c1 + cStep; c != c2; c += cStep) {
                        if (board[r1][c] != '.') return 0;
                    }
                } else if (dc == 0) { // Vertical check
                    int rStep = (r2 > r1) ? 1 : -1;
                    for (int r = r1 + rStep; r != r2; r += rStep) {
                        if (board[r][c1] != '.') return 0;
                    }
                }
                return 1;
            }
            return 0;
        }
        case 'K': return (dr <= 1 && dc <= 1); // King (One step in any direction)
        default: return 0;
    }
}

// ------------ Check if a square is attacked ------------
// Checks if the square (r, c) is attacked by any piece of 'byColor'.
int isSquareAttacked(int r, int c, char byColor) {
    for (int r_att = 0; r_att < 8; r_att++) {
        for (int c_att = 0; c_att < 8; c_att++) {
            char piece = board[r_att][c_att];
            if (piece == '.') continue;
            // Check if the piece is the correct attacking color
            if (byColor == 'W' && !isWhite(piece)) continue;
            if (byColor == 'B' && !isBlack(piece)) continue;

            // Temporarily treat the target square as empty for path checking logic
            char originalTarget = board[r][c];
            board[r][c] = '.';
            int isAttacking = 0;
            
            // Special pawn attack logic (pawns capture differently than they move)
            if (toupper(piece) == 'P') {
                int direction = (isWhite(piece)) ? -1 : 1;
                // Pawn attack is one step diagonal
                if (abs(c - c_att) == 1 && r - r_att == direction) {
                    isAttacking = 1;
                }
            } 
            // Check all other pieces using the general move logic
            else if (isLegalPieceMove(r_att, c_att, r, c)) {
                isAttacking = 1;
            }
            
            // Restore the board state
            board[r][c] = originalTarget; 
            
            if (isAttacking) return 1;
        }
    }
    return 0;
}

// Checks if the King of 'color' is currently in check.
int isInCheck(char color) {
    int kr, kc;
    if(!findKing(color, &kr, &kc)) return 0;
    char opponent = (color == 'W') ? 'B' : 'W';
    return isSquareAttacked(kr, kc, opponent);
}

// Checks if the player of 'color' has any legal moves.
int hasLegalMove(char color) {
    for(int r1 = 0; r1 < 8; r1++) {
        for(int c1 = 0; c1 < 8; c1++) {
            char piece = board[r1][c1];
            if(piece == '.') continue;
            // Only check pieces of the current player's color
            if(color == 'W' && !isWhite(piece)) continue;
            if(color == 'B' && !isBlack(piece)) continue;
            
            for(int r2 = 0; r2 < 8; r2++) {
                for(int c2 = 0; c2 < 8; c2++) {
                    if(r1 == r2 && c1 == c2) continue; // Cannot move to same square
                    
                    // 1. Check if the piece can legally move to (r2, c2)
                    if (!isLegalPieceMove(r1, c1, r2, c2)) continue;
                    
                    // 2. Test the move (Check for self-check/pins)
                    char temp = board[r2][c2];
                    board[r2][c2] = board[r1][c1];
                    board[r1][c1] = '.';
                    
                    int stillInCheck = isInCheck(color);
                    
                    // 3. Undo the test move
                    board[r1][c1] = board[r2][c2];
                    board[r2][c2] = temp;
                    
                    if(!stillInCheck) return 1; // Found at least one legal move
                }
            }
        }
    }
    return 0; // No legal moves found
}

int isCheckmate(char color) {
    return isInCheck(color) && !hasLegalMove(color);
}

int isStalemate(char color) {
    return !isInCheck(color) && !hasLegalMove(color);
}

// Prints the list of captured pieces for one side.
void printCapturedPieces(char* pieces, int count, const char* piece_color) {
    printf("%s", ANSI_COLOR_RESET);
    if (count == 0) {
        printf("[None]");
        return;
    }
    printf("%s", piece_color);
    for (int i = 0; i < count; i++) {
        printf("%s ", getPiece(pieces[i]));
    }
    printf("%s", ANSI_COLOR_RESET);
}

// Prints the entire chess board with coordinates, colors, and captured pieces.
void printBoard() {
    printf("\n%s", ANSI_COLOR_RESET);
    printf(ANSI_COLOR_BORDER "  ========================================\n" ANSI_COLOR_RESET);
    printf("  Captured by White: ");
    printCapturedPieces(captured_by_white, captured_by_white_count, ANSI_COLOR_BLACK);
    printf("\n  Captured by Black: ");
    printCapturedPieces(captured_by_black, captured_by_black_count, ANSI_COLOR_WHITE);
    printf("\n");
    printf(ANSI_COLOR_BORDER "  ========================================\n\n" ANSI_COLOR_RESET);
    
    printf("      A   B   C   D   E   F   G   H\n");
    printf(ANSI_COLOR_BORDER "    +---+---+---+---+---+---+---+---+\n");
    
    for(int i=0; i<8; i++) {
        printf(ANSI_COLOR_BORDER " %d ", 8-i);
        for(int j=0; j<8; j++) {
            char piece = board[i][j];
            printf("|");
            // Set color for the piece based on its case
            if (isWhite(piece)) {
                printf(ANSI_COLOR_WHITE);
            } else if (isBlack(piece)) {
                printf(ANSI_COLOR_BLACK);
            } else {
                printf(ANSI_COLOR_RESET);
            }
            printf(" %s ", getPiece(piece));
            printf(ANSI_COLOR_BORDER);
        }
        printf("| %d\n", 8-i);
        printf("    +---+---+---+---+---+---+---+---+\n");
    }
    printf(ANSI_COLOR_RESET "      A   B   C   D   E   F   G   H\n\n");
}

// Full move validation: combines piece logic and check/pin constraints.
int validMove(int r1,int c1,int r2,int c2,char turn) {
    // 1. Check if coordinates are within bounds
    if(r1<0||r1>7||c1<0||c1>7||r2<0||r2>7||c2<0||c2>7) return 0;
    // 2. Check if the starting square has a piece
    if(board[r1][c1]=='.') return 0;
    // 3. Check if the piece belongs to the current player
    if(turn=='W' && !isWhite(board[r1][c1])) return 0;
    if(turn=='B' && !isBlack(board[r1][c1])) return 0;
    // 4. Check if the move is physically legal for the piece
    if(!isLegalPieceMove(r1, c1, r2, c2)) return 0;
    
    // 5. Check if the move leaves the king in check (pin check)
    char temp = board[r2][c2];
    board[r2][c2] = board[r1][c1];
    board[r1][c1] = '.';
    int inCheck = isInCheck(turn);
    // Restore board state
    board[r1][c1] = board[r2][c2];
    board[r2][c2] = temp;
    
    if(inCheck) return 0; // Move is illegal if it results in self-check
    return 1;
}

// Executes the move after validation, pushing it to the stack and updating captured pieces.
void makeMove(int r1,int c1,int r2,int c2) {
    char moved_piece = board[r1][c1];
    char captured = board[r2][c2];
    
    // Push move to stack for undo feature
    pushMove(r1,c1,r2,c2,captured); 
    
    // Update captured piece lists
    if (captured != '.') {
        if (isWhite(moved_piece)) {
            captured_by_white[captured_by_white_count++] = captured;
        } else {
            captured_by_black[captured_by_black_count++] = captured;
        }
    }
    
    // Execute move on the board
    board[r2][c2] = moved_piece;
    board[r1][c1] = '.';
}

int main() {
    char input[10];
    int r1,c1,r2,c2;
    char turn='W';

    printf("\n  ----------------------------------------\n");
    printf("            MINI CHESS GAME\n");
    printf("          Data Structures Project\n");
    printf("  ----------------------------------------\n");
    printf("\n  HOW TO PLAY:\n");
    printf("  - Enter moves: e2e4 (from e2 to e4)\n");
    printf("  - Type 'u' to undo | 'e' to exit\n");
    
    printBoard();

    while(1) {
        if(isCheckmate(turn)) {
            printf("\n  ========================================\n");
            printf("      *** CHECKMATE! %s WINS! ***\n", (turn=='W')?"BLACK":"WHITE");
            printf("  ========================================\n\n");
            break;
        }
        if(isStalemate(turn)) {
            printf("\n  ========================================\n");
            printf("      *** STALEMATE! GAME IS A DRAW! ***\n");
            printf("  ========================================\n\n");
            break;
        }
        if(isInCheck(turn)) {
            printf("  ⚠️  %s IS IN CHECK! ⚠️\n\n", (turn=='W')?"WHITE":"BLACK");
        }
        
        printf("  %s's turn > ", (turn=='W')?"White":"Black");
        if (scanf("%s",input) != 1) break;

        // Handle Exit command
        if((input[0]=='e' || input[0]=='E') && strlen(input)==1) {
            printf("\n  Exiting game...\n");
            break;
        }

        // Handle Undo command (using the Move Stack)
        if(input[0]=='u' || input[0]=='U') {
            if(top>=0) {
                Move m = popMove();
                if (m.r1 != -1) {
                    // Restore board state
                    board[m.r1][m.c1] = m.original_piece;
                    board[m.r2][m.c2] = m.captured;
                    
                    // Undo captured piece update
                    if (m.captured != '.') {
                        if (isBlack(m.captured)) {
                            if (captured_by_white_count > 0) captured_by_white_count--;
                        } else if (isWhite(m.captured)) {
                            if (captured_by_black_count > 0) captured_by_black_count--;
                        }
                    }
                    turn = (turn=='W')?'B':'W'; // Switch turn back
                    printf("\n  ✓ Move undone!\n");
                    printBoard();
                }
            } else {
                printf("\n  ✗ Nothing to undo\n\n");
            }
            continue;
        }

        // Validate input length
        if(strlen(input)!=4) {
            printf("\n  ✗ Invalid format! Use: e2e4\n\n");
            continue;
        }

        // Convert algebraic notation (e.g., e2e4) to array indices
        c1 = tolower(input[0])-'a';
        r1 = 8-(input[1]-'0');
        c2 = tolower(input[2])-'a';
        r2 = 8-(input[3]-'0');
        
        // Re-validate coordinates after conversion
        if (r1 < 0 || r1 > 7 || c1 < 0 || c1 > 7 || r2 < 0 || r2 > 7 || c2 < 0 || c2 > 7) {
            printf("\n  ✗ Invalid coordinates!\n\n");
            continue;
        }

        // Full validation of the requested move
        if(!validMove(r1,c1,r2,c2,turn)) {
            printf("\n  ✗ Invalid move!\n\n");
            continue;
        }

        // Execute the move
        makeMove(r1,c1,r2,c2);
        
        // Handle Pawn Promotion
        char moved_piece = board[r2][c2];
        if (toupper(moved_piece) == 'P' && (r2 == 0 || r2 == 7)) {
            char promo_piece_char;
            char player_color = isWhite(moved_piece) ? 'W' : 'B';
            char input_promo[10];
            printf("  ♙ Pawn Promotion! Choose (Q/R/B/N): ");
            while(1) {
                // Read promotion choice
                if (scanf("%s", input_promo) != 1 || strlen(input_promo) != 1) {
                    printf("  ✗ Invalid input: ");
                    // Clear input buffer (important for robust scanf loops)
                    while (getchar() != '\n'); 
                    continue;
                }
                promo_piece_char = toupper(input_promo[0]);
                // Validate promotion choice
                if (promo_piece_char == 'Q' || promo_piece_char == 'R' || 
                    promo_piece_char == 'B' || promo_piece_char == 'N') {
                    break;
                }
                printf("  ✗ Must be Q/R/B/N: ");
            }
            // Apply promotion piece (uppercase for White, lowercase for Black)
            if (player_color == 'W') {
                board[r2][c2] = promo_piece_char;
            } else {
                board[r2][c2] = tolower(promo_piece_char);
            }
            printf("  ✓ Promoted to %s\n", getPiece(board[r2][c2]));
        }

        // Change turns
        turn = (turn=='W')?'B':'W';
        printBoard();
        
        // Check for immediate game over condition for the next player
        if(isCheckmate(turn)) {
            printf("\n  ========================================\n");
            printf("      *** CHECKMATE! %s WINS! ***\n", (turn=='W')?"BLACK":"WHITE");
            printf("  ========================================\n\n");
            break;
        }
        if(isStalemate(turn)) {
            printf("\n  ========================================\n");
            printf("      *** STALEMATE! GAME IS A DRAW! ***\n");
            printf("  ========================================\n\n");
            break;
        }
    }

    printf("\n  ----------------------------------------\n");
    printf("        Thanks for playing!\n");
    printf("  ----------------------------------------\n\n");
    return 0;
}