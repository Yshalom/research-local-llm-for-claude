## Evaluation for security test-2

### Score: 23/25

| subject | score | note |
| --- | --- | --- |
| Issue found? | 5 | It found the vulnerability |
| False security issues | 5 | No false vulnerabilities presented |
| How it could be badly used? | 4 | Mention that we can alter the stored hash with buffer overflow. Explain the how an integer overflow can be triggered, and how would it cause a buffer overflow. Mention the possibility to bypass the password check. Didn't show a flow from the buffer overflow to the password check bypass (only mention it). |
| Source code lines | 4 | Gave the exact lines of the vulnerability, though didn't show the location of the hash-buffer relative to password-buffer, which is crucial to the password bypass. |
| Fix | 5 |  Doing the suggested fixes will secure the code. |