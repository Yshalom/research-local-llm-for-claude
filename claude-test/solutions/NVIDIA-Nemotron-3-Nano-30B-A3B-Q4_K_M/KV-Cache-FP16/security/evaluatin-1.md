## Evaluation for test-1

### Score: 8/10

| Subject |  | Note |
| --- | --- | --- |
| What you found? | 5 | It found the vulnerability |
| How it could be badly used? | 3 | Point out that it could be used when typing more than 20 characters. No explanation over the `evaluated_password` flag corruption, that would cause authentication success. |
| Source code lines | 4 | Gave the exact vulnerable lines. However, no mention of the  `evaluated_password` flag |
| Fix | 5 | Doing the suggested fix will secure the code |