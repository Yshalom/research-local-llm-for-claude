## Evaluation for python test

### Score: 24/30

| subject | score | note |
| --- | --- | --- |
| Input | 3 | Doesn't read shorten lines; Doesn't read space (' ') as empty cell;  |
| No Solution | 5 | Detects Contradictory clues; Finds boards with no solution. |
| Solving Skill | 5 | Solve the board correctly; Use recursive correctly |
| Efficiency | 4 | Implement MRV (efficient search); Call recursive even with 1 bit mask; |
| Trailing Code | 2 | `digit_to_char` function; `self._mrv_cell()` called twice with the same state (lines: 64, 67) |
| code Cleanness | 5 | The code is clean; Variable names are good; Function separation is good; Uses try-except and robust |
