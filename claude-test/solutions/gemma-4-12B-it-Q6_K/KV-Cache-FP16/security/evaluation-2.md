## Evaluation for security test-2

### Score: 17/25

| subject | score | note |
| --- | --- | --- |
| What you found? | 3 | Didn't find the vulnerability. Point out that large numbers at password_length are not check out, which may cause DoS. Mention buffer overlow risk on `PASSWORD_CHECKER` withough more detials. |
| False security issues | 2 | 1: Buffer overflow in the `bytes2hexstr` function. 2: Incorrect buffer length for `memcmp(hash, tmp_hash, HASH_STRING_LENGTH)`. 3: Timing attack on hash compere. |
| How it could be badly used? | 3 | Mention buffer overlow risk on `PASSWORD_CHECKER` withough more detials, didn't point out that it may cause password bypass. |
| Source code lines | 4 | *Line 79: wscanf_s(L"%u", &password_length)*, the gave the bad line, but didn't found any integer overflow. +2 False security issues. |
| Fix | 5 | The agent suggests to limit the `password_length`, which would secure the code.  |
