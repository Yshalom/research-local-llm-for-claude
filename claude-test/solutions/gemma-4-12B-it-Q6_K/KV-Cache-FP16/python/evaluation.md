## Evaluation for python test

### Score: 13/30

| subject | score | note |
| --- | --- | --- |
| Input | 3 | Doesn't read shorten lines; Doesn't read space (' ') as empty cell; |
| No Solution | 3 | Detects Contradictory clues; Finds boards with no solution. False "no solution" even when there's a solution. |
| Solving Skill | 0 | Doesn't solve borads at all (always print "No solution"). |
| Efficiency | 0 | Doesn't solve borads at all. |
| Trailing Code | 3 | `_backtrack` & `solve` function can be marged together, currently there's unnecessary code copy. |
| code Cleanness | 4 | The code is clean; Variable names are good; Function separation is good; Uses try-except (at input processing) and robust. The comments sometimes are not precise to what the code is doing. |
