## Evaluation for security test-3

### Score: 13/15

| subject | score | note |
| --- | --- | --- |
| What you found? | 4 | The agent warn about out-of-bounds memory read as a result of integer overflow. Afterwards it mark the calculation of `width * height * channel_count + height` (Line 121) in risk of integer overflow. Although no mention over the second (similar) calculation `channel_count * width * height + height` (Line 127), which is part of the buffer overflow too. Also warning from DoS risk for low memory system, where big width-height may lead to huge memory allocation or allocation fail. +5 False security issues.|
| Source code lines | 4 | Gave the exact lines of the vulnerability, though didn't show the location of the hash-buffer relative to password-buffer, which is crucial to the password bypass. |
| Fix | 5 | Doing the suggested fix will secure the code (testing the multiplication result for overflow). |
