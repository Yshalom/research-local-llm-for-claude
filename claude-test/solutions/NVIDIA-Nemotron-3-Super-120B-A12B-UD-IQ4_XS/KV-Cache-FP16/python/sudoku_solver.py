#!/usr/bin/env python3
import sys

class SudokuSolver:
    def __init__(self):
        self.board = [[0]*9 for _ in range(9)]
        self.rows = [0]*9      # bitmask per row
        self.cols = [0]*9      # bitmask per col
        self.boxes = [0]*9     # bitmask per 3x3 box

    def parse_line(self, line):
        """Parse a line of 9 characters, return list of ints (0 for empty)."""
        if len(line) != 9:
            return None
        row = []
        for ch in line:
            if ch in '123456789':
                row.append(int(ch))
            elif ch in '0.-_ ':
                row.append(0)
            else:
                return None
        return row

    def load_from_stdin(self):
        """Read 9 lines from stdin, validate, and initialize bitmasks."""
        print("Enter Sudoku puzzle (9 lines, use . 0 - _ for empty):")
        for i in range(9):
            try:
                line = input()
            except EOFError:
                print("Error: Expected 9 lines of input.")
                sys.exit(1)
            # Ignore leading/trailing whitespace
            line = line.strip()
            if len(line) != 9:
                print("Error: Each line must contain exactly 9 characters after stripping whitespace.")
                sys.exit(1)
            parsed = self.parse_line(line)
            if parsed is None:
                print("Error: Invalid character in line. Use digits 1-9 or . 0 - _ for empty.")
                sys.exit(1)
            self.board[i] = parsed
        # Initialize bitmasks and check for contradictions
        for r in range(9):
            for c in range(9):
                val = self.board[r][c]
                if val != 0:
                    bit = 1 << (val - 1)
                    b = (r // 3) * 3 + (c // 3)
                    if (self.rows[r] & bit) or (self.cols[c] & bit) or (self.boxes[b] & bit):
                        print("Error: Contradictory clues detected.")
                        sys.exit(1)
                    self.rows[r] |= bit
                    self.cols[c] |= bit
                    self.boxes[b] |= bit

    def candidates_mask(self, r, c):
        """Return bitmask of possible digits for cell (r,c)."""
        b = (r // 3) * 3 + (c // 3)
        used = self.rows[r] | self.cols[c] | self.boxes[b]
        # bits 0-8 correspond to digits 1-9
        return (~used) & 0x1FF  # 0x1FF = binary 111111111

    def count_bits(self, x):
        """Count set bits in integer."""
        return bin(x).count('1')

    def find_mrv(self):
        """Find empty cell with Minimum Remaining Values.
        Returns (r, c, mask) or None if no empty cells."""
        min_count = 10
        min_cell = None
        for r in range(9):
            for c in range(9):
                if self.board[r][c] == 0:
                    mask = self.candidates_mask(r, c)
                    if mask == 0:
                        # No candidates -> dead end
                        return (r, c, 0)
                    cnt = self.count_bits(mask)
                    if cnt < min_count:
                        min_count = cnt
                        min_cell = (r, c, mask)
                        if min_count == 1:
                            return min_cell
        return min_cell

    def solve(self):
        """Backtracking search with MRV and constraint propagation.
        Returns True if solved, False otherwise."""
        cell = self.find_mrv()
        if cell is None:
            # No empty cells -> solved
            return True
        r, c, mask = cell
        if mask == 0:
            return False  # contradiction
        b = (r // 3) * 3 + (c // 3)
        # Try each candidate
        m = mask
        while m:
            # isolate lowest bit
            bit = m & -m
            d = (bit.bit_length())  # because 1<<0 -> bit_length=1 => digit 1
            # Place digit
            self.board[r][c] = d
            self.rows[r] |= bit
            self.cols[c] |= bit
            self.boxes[b] |= bit
            if self.solve():
                return True
            # Undo
            self.board[r][c] = 0
            self.rows[r] &= ~bit
            self.cols[c] &= ~bit
            self.boxes[b] &= ~bit
            m -= bit
        return False

    def print_board(self):
        """Print solved board as 9 lines of digits."""
        for r in range(9):
            line = ''.join(str(self.board[r][c]) for c in range(9))
            print(line)


def main():
    solver = SudokuSolver()
    solver.load_from_stdin()
    if solver.solve():
        solver.print_board()
    else:
        print("No solution")
        sys.exit(0)


if __name__ == "__main__":
    main()