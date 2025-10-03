#include <stdio.h>
#include <conio.h>
#include <string.h>
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

/* Print the chess board */
void printBoard() {
    int i, j;
    clrscr();
    printf("\n   a b c d e f g h\n");
    printf("  -----------------\n");
    for (i = 0; i < 8; i++) {
        printf("%d| ", 8 - i);
        for (j = 0; j < 8; j++) {
            printf("%c ", board[i][j]);
        }
        printf("|\n");
    }
    printf("  -----------------\n");
}
int validMove(int r1, int c1, int r2, int c2) {
    if (r1 < 0 || r1 > 7 || r2 < 0 || r2 > 7) return 0;
    if (c1 < 0 || c1 > 7 || c2 < 0 || c2 > 7) return 0;
    return 1;
}

void makeMove(int r1, int c1, int r2, int c2) {
    board[r2][c2] = board[r1][c1];
    board[r1][c1] = '.';
}

int main() {
    char move[10];
    int r1, c1, r2, c2;

    printBoard();

    while (1) {
        printf("\nEnter move (e.g. e2e4) or 'exit': ");
        scanf("%s", move);

        if (strcmpi(move, "exit") == 0)
            break;

        if (strlen(move) != 4) {
            printf("Invalid format! Use format like e2e4.\n");
            continue;
        }

        c1 = move[0] - 'a';
        r1 = 8 - (move[1] - '0');
        c2 = move[2] - 'a';
        r2 = 8 - (move[3] - '0');

        if (!validMove(r1, c1, r2, c2)) {
            printf("Invalid move!\n");
            continue;
        }

        makeMove(r1, c1, r2, c2);
        printBoard();
    }

    printf("\nGame Over.\n");
    getch();
    return 0;
}

