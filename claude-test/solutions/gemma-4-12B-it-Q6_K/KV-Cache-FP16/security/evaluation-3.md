## Evaluation for security test-3

### Score: 13/15

| subject | score | note |
| --- | --- | --- |
| What you found? | 4 | The agent warn about buffer overflow as a result of integer overflow at the calculation of `width * height * channel_count + height` (Line 121). Although no mention over the second (similar) calculation `channel_count * width * height + height` (Line 127), which is part of the buffer overflow too. |
| Source code lines | 4 | Line 121 is there, no mention to line 127 wich is part of the buffer overflow vulnerability too. |
| Fix | 5 | The agent suggests to limit the `password_length`, which would secure the code.  |
