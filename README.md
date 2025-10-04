# Chess Game in C – Data Structures Project

A **mini chess game** implemented in **C language**, enhanced with **data structures** and additional features for a better playing experience.

---

## Features

- **ANSI-colored board display** for clear visualization of pieces.
- **Captured pieces tracking** for both White and Black.
- **Undo functionality** using a **stack data structure**.
- **Full move validation**:
  - Legal moves for all pieces (Pawn, Knight, Bishop, Rook, Queen, King)
  - Check, checkmate, and stalemate detection
  - Prevention of self-check moves
- **Pawn promotion** with choice of piece (Queen, Rook, Bishop, Knight)
- Enhanced **user input handling** with validation
- Clear console instructions for easy gameplay

---

## How to Play

1. Compile and run `CHESS.C` in a C compiler/IDE.
2. Enter moves in algebraic notation, e.g., `e2e4`.
3. Commands:
   - `u` → Undo the last move
   - `e` → Exit the game
4. Captured pieces are dynamically updated.
5. Game ends on **checkmate**, **stalemate**, or **exit**.

---

## Code Structure

- `CHESS.C` → main program file containing all game logic.
- **Data Structures**:
  - **Stack** → for undo moves
  - **Arrays** → for captured pieces tracking

---

## Author
- Original Code: [Jacksonfio](https://github.com/Jacksonfio/chessgame)  
- Enhanced Version & PR: [Immanuel Thomas J](https://github.com/immanuel-thomas-j/chessgame)
