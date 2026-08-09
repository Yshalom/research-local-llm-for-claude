#!/usr/bin/env python3
import sys

# ---------- Helper functions ----------
def char_to_digit(ch):
    if ch.isdigit():
        return int(ch)
    # treat '.', '0', '-', '_', ' ' as empty
    if ch in '.0-_ ':
        return 0
    return None

def digit_to_char(d):
    return str(d)

# ---------- Sudoku Solver ----------
class SudokuSolver:
    def __init__(self, board):
        self.board = board  # 9x9 list of ints (0-9)
        self.rows = [0] * 9
        self.cols = [0] * 9
        self.boxes = [0] * 9
        self._init_masks()

    def _init_masks(self):
        for r in range(9):
            for c in range(9):
                val = self.board[r][c]
                if val == 0:
                    continue
                bit = 1 << (val - 1)
                b = (r // 3) * 3 + (c // 3)
                if (self.rows[r] & bit) or (self.cols[c] & bit) or (self.boxes[b] & bit):
                    raise ValueError("Contradictory clues detected")
                self.rows[r] |= bit
                self.cols[c] |= bit
                self.boxes[b] |= bit

    def _candidate_mask(self, r, c):
        b = (r // 3) * 3 + (c // 3)
        used = self.rows[r] | self.cols[c] | self.boxes[b]
        # allowed digits are those bits not set in used
        return ~used & 0x1FF  # 9 bits set

    def _mrv_cell(self):
        min_candidates = 10
        best = None
        for r in range(9):
            for c in range(9):
                if self.board[r][c] == 0:
                    mask = self._candidate_mask(r, c)
                    cnt = mask.bit_count()
                    if cnt == 0:
                        return r, c, 0  # dead end
                    if cnt < min_candidates:
                        min_candidates = cnt
                        best = (r, c, mask)
                        if cnt == 1:
                            return best
        return best

    def solve_recursive(self):
        # if solved
        if self._mrv_cell() is None:
            return True

        r, c, mask = self._mrv_cell()
        if mask == 0:
            return False

        # try each possible digit
        for d in range(1, 10):
            bit = 1 << (d - 1)
            if mask & bit:
                # place digit
                self.board[r][c] = d
                self.rows[r] |= bit
                self.cols[c] |= bit
                self.boxes[(r // 3) * 3 + (c // 3)] |= bit

                if self.solve_recursive():
                    return True

                # undo
                self.board[r][c] = 0
                self.rows[r] ^= bit
                self.cols[c] ^= bit
                self.boxes[(r // 3) * 3 + (c // 3)] ^= bit
        return False

    def solve(self):
        return self.solve_recursive()

    def formatted_board(self):
        return [''.join(str(cell) for cell in row) for row in self.board]

# ---------- Input parsing ----------
def read_puzzle():
    print("Enter Sudoku puzzle, 9 lines of 9 characters each.")
    print("Use digits 1‑9 for clues, and any of '.', '0', '-', '_', ' ' for empty cells.")
    lines = []
    for i in range(1, 10):
        raw = input(f"Line {i}: ")
        # remove any whitespace
        raw = ''.join(raw.split())
        if len(raw) != 9:
            print("Error: each line must contain exactly 9 non‑whitespace characters.", file=sys.stderr)
            sys.exit(1)
        board_row = []
        for ch in raw:
            d = char_to_digit(ch)
            if d is None:
                print(f"Error: invalid character '{ch}' in input.", file=sys.stderr)
                sys.exit(1)
            board_row.append(d)
        lines.append(board_row)
    return lines

# ---------- Main ----------
def main():
    try:
        board = read_puzzle()
    except SystemExit:
        sys.exit(1)

    try:
        solver = SudokuSolver(board)
    except ValueError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

    if solver.solve():
        for line in solver.formatted_board():
            print(line)
    else:
        print("No solution")

if __name__ == "__main__":
    main()