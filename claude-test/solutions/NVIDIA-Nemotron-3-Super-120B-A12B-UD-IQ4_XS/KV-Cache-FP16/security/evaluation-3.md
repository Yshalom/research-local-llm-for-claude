## Evaluation for security test-3

### Score: 14/20

| subject | score | note |
| --- | --- | --- |
| What you found? | 4 | The agent warn about out-of-bounds memory read as a result of integer overflow. Afterwards it mark the calculation of `width * height * channel_count + height` (Line 121) in risk of integer overflow. Although no mention over the second (similar) calculation `channel_count * width * height + height` (Line 127), which is part of the buffer overflow too. Also warning from DoS risk for low memory system, where big width-height may lead to huge memory allocation or allocation fail. |
| False security issues | 1 | 1: Integer overflow in bitmap buffer allocation. 2: Integer overflow in `filter_decompressed_data()`. 3: Integer overflow in the `compressed_size` variable. 4: Grey-scale images cause out-of-bounds read. 5: Potential Unicode mismatch in drag-and-drop handling at ANSI mode (The code isn't compilable at ANSI mode). | 
| Source code lines | 4 | Gave the exact lines of the vulnerability, though didn't show the location of the hash-buffer relative to password-buffer, which is crucial to the password bypass. |
| Fix | 5 | Doing the suggested fix will secure the code (testing the multiplication result for overflow). |
