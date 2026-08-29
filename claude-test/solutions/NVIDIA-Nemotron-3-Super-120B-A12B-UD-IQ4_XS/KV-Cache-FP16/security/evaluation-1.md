## Evaluation for security test-1

### Score: 25/25

| subject | score | note |
| --- | --- | --- |
| Issue found? | 5 | It found the vulnerability. |
| False security issues | 5 | No false security vulnerabilities presented. |
| How could be it badly used? | 5 | Explain that a corruption of the `evaluated_password` flag, would be triggered by abusing the buffer overflow of `password`. Point out that `scanf("%s", password);` causes the overflow. *Quote: """Bypass password verification by manipulating the `evaluated_password` flag directly"""* |
| Source code lines | 5 | Gave the exact vulnerable lines. Mention of the  `evaluated_password` flag. |
| Fix | 5 | Doing the suggested fixes will secure the code. |