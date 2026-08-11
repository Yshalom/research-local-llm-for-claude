## Evaluation for security test-2

### Score: 15/20

| subject | score | note |
| --- | --- | --- |
| What you found? | 3 | Didn't find the vulnerability. Point out that large numbers at password_length are not check out, which may cause DoS, if there's no enough memory to allocate. +3 False security issues. Mention buffer overlow risk on `PASSWORD_CHECKER` withough more detials. |
| How it could be badly used? | 3 | Mention buffer overlow risk on `PASSWORD_CHECKER` withough more detials, didn't point out that it may cause password bypass. |
| Source code lines | 4 | *Line 79: wscanf_s(L"%u", &password_length)*, the gave the bad line, but didn't found any integer overflow. +2 False security issues. |
| Fix | 5 | The agent suggests to limit the `password_length`, which would secure the code.  |
