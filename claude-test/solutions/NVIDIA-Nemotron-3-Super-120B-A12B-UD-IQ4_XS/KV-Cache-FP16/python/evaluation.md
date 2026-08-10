## Evaluation for python test

### Score: 27/30

| subject | score | note |
| --- | --- | --- |
| Input | 4 | *When the space (' ') is at the beginning/ending of a line*, doesn't read it as an empty cell.  |
| No Solution | 5 | Detects Contradictory clues; Finds boards with no solution. |
| Solving Skill | 5 | Solve the board correctly; Use recursive correctly |
| Efficiency | 4 | Implement MRV (efficient search); But entering into recursive step even with 1 bit mask (unnecessary); |
| Trailing Code | 4 | `count_bits` function is 1 line of minimal calculation, and it's being called once |
| code Cleanness | 5 | The code is clean; Variable names are good; Function separation is good; Uses try-except (at input processing) and robust |
