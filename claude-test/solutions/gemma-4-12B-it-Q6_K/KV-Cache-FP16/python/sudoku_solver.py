import sys

class SudokuSolver:
    def __init__(self):
        # 9x9 board, 0 represents empty cells
        self.board = [[0] * 9 for _ in range(9)]
        # Bitmasks for rows, columns, and 3x3 boxes
        self.rows = [0] * 9
        self.cols = [0] * 9
        self.boxes = [0] * 9

    def get_box_index(self, r, c):
        return (r // 3) * 3 + (c // 3)

    def load_board(self, grid):
        for r in range(9):
            for c in range(9):
                val = grid[r][c]
                if val != 0:
                    # Check for contradiction during initialization
                    bit = 1 << (val - 1)
                    box_idx = self.get_box_index(r, c)
                    if (self.rows[r] & bit) or (self.cols[c] & bit) or (self.boxes[box_idx] & bit):
                        return False
                    # Set bits in masks
                    self.rows[r] |= bit
                    self.cols[c] |= bit
                    self.boxes[box_idx] |= bit
        return True

    def get_candidates(self, r, c):
        # Combine all occupied positions
        occupied = self.rows[r] | self.cols[c] | self.boxes[self.get_box_index(r, c)]
        # Candidates are bits not set in the combined mask (within 1-9 range)
        return (~occupied) & 0x1FF

    def solve(self):
        # Find all empty cells and their candidate counts
        empty_cells = []
        for r in range(9):
            for c in range(9):
                if self.board[r][c] == 0:
                    mask = self.get_candidates(r, c)
                    count = bin(mask).count('1')
                    if count == 0:
                        return False  # Prune: cell has no valid moves
                    empty_cells.append((r, c, mask))

        # Sort by Minimum Remaining Value (MRV) heuristic
        empty_cells.sort(key=lambda x: bin(x[2]).count('1'))

        # If no empty cells are left, the puzzle is solved
        if not empty_cells:
            return True

        # Pick the MRV cell
        r, c, mask = empty_cells[0]

        # Try each candidate digit
        for num in range(1, 10):
            bit = 1 << (num - 1)
            if mask & bit:
                # Place number
                self.board[r][c] = num
                self.rows[r] |= bit
                self.cols[c] |= bit
                self.boxes[self.get_box_index(r, c)] |= bit

                # Recurse - note: we need to re-calculate MRV for the new state
                # Since standard backtracking with MRV usually picks a new cell
                # at every step of recursion, let's refine the recursive structure.
                if self._backtrack():
                    return True

                # Backtrack (undo)
                self.board[r][c] = 0
                self.rows[r] ^= bit
                self.cols[c] ^= bit
                self.boxes[self.get_box_index(r, c)] ^= bit

        return False

    def _backtrack(self):
        # Find best cell based on MRV
        best_cell = None
        min_count = 10

        for r in range(9):
            for c in range(9):
                if self.board[r][c] == 0:
                    mask = self.get_candidates(r, c)
                    count = bin(mask).count('1')
                    if count == 0: return False # Contradiction found
                    if count < min_count:
                        min_count = count
                        best_cell = (r, c, mask)

        if best_cell is None:
            return True  # No empty cells left

        r, c, mask = best_cell
        for num in range(1, 10):
            bit = 1 << (num - 1)
            if mask & bit:
                self.board[r][c] = num
                self.rows[r] |= bit
                self.cols[c] |= bit
                self.boxes[self.get_box_index(r, c)] |= bit

                if self._backtrack():
                    return True

                # Undo
                self.board[r][c] = 0
                self.rows[r] ^= bit
                self.cols[c] ^= bit
                self.boxes[self.get_box_index(r, c)] ^= bit

        return False

    def print_solution(self):
        for row in self.board:
            print("".join(map(str, row)))

def main():
    print("Enter Sudoku puzzle (9 lines of 9 characters):")
    grid = []
    try:
        for _ in range(9):
            line = input().strip()
            # Filter out whitespace but keep core chars
            processed_line = "".join([c for c in line if not c.isspace()])
            if len(processed_line) != 9:
                print("Error: Each line must contain exactly 9 characters.")
                sys.exit(1)
            grid.append(processed_line)

        board_data = []
        for r in range(9):
            row_data = []
            for c in range(9):
                char = grid[r][c]
                if char in ".0-_ ":
                    row_data.append(0)
                elif '1' <= char <= '9':
                    row_data.append(int(char))
                else:
                    print(f"Error: Illegal character '{char}' at row {r}, col {c}")
                    sys.exit(1)
            board_data.append(row_data)

        solver = SudokuSolver()
        if not solver.load_board(board_data):
            # Contradiction during initialization
            print("Error: Contradictory clues detected.")
            sys.exit(1)

        if solver.solve():
            solver.print_solution()
        else:
            print("No solution")
            sys.exit(0)

    except EOFError:
        pass
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
