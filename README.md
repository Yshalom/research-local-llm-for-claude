# This Project Is Not Complete
## I'll finish it soon
---
---
---

# Local LLM For Claude

## Introduction
In the raising days of AI, and its integration at the development and coding tasks opened new doors for smart development environment. In this project I compere common popular open-source models that can run on a personal computer. You can navigate the project and see for yourself.

**The project is build up in different git branches to prevent the models from cheating, the branches are separated! A model can read only the branch under checkout.**

- Speed tests are under [speed-test/](speed-test/) folder.
- The coding tests are under [claude-test/](claude-test/) **Make you are checkout the correct branch to see the tests**.
    - [claude-test/tests/](claude-test/tests/) contains the tasks which the models must fulfill.
    - [claude-test/solutions/](claude-test/solutions/) contains the models solutions to the tasks + my evaluation.

---
- Hardware
    - GPU: PNY **RTX 5080** Overclocked Triple Fan (OC: +4% for Boost Clock)
    - CPU: AMD Ryzen 9 9900X (12-cors/24-threads)
    - Memory: 2 sticks of G.Skill F5-6400J3239F48G (total 96GB, under-clocked to 6000MT/s for CPU compatibility)
- Software (host)
    - OS: Windows-11
    - LM-Studio: 0.4.20
    - CUDA 12 llama.cpp (Windows) v2.27.1
    - Driver: NVIDIA Studio Driver 610.62
    - docker sbx version: v0.38.0
    - Python: 3.14.2
- Software (sandbox)
    - OS: Ubuntu 26.04 LTS
    - Claude Code: v2.1.224
    - Browser: Google Chrome 151.0.7922.108 (running with headless mode)
    - Python: 3.14.4
- Claude Code sessions
    - Effort: max
    - Mode: auto mode on
---

### Model Inference
All the models are loaded with the default parameters, unless explicitly said otherwise in `note.txt` file that will appear in the directory of change!  
I build the project such as: when a folder has `note.txt` file, which explicitly states how the model is loaded, or how inference is configured. it recursively mark all the subdirectories too (unless another `note.txt` file state otherwise).


### Claude-Code set-up

Claude Code is running inside docker-sandbox, the sandbox runs with the following additional environment variables (connecting Claude-Code to LM-Studio's server)
```
ANTHROPIC_API_KEY=not-needed-for-local
ANTHROPIC_BASE_URL=http://host.docker.internal:1234
```

In the sandbox network policy we allow connection to `localhost:1234`.

Claude-Code settings are set to the `~/.claude/settings.json` file:
```json
{
  "env": {
    "CLAUDE_CODE_ATTRIBUTION_HEADER": "0",
    "CLAUDE_CODE_MAX_CONTEXT_TOKENS": "MAXIMUM-CONTEXT-WINDOW",
    "CLAUDE_CODE_AUTO_COMPACT_WINDOW": "MAXIMUM-CONTEXT-WINDOW * 90%"
  },
  "model": "MODEL-NAME",
  "alwaysThinkingEnabled": true,
  "skipDangerousModePermissionPrompt": true
}
```

- context length:
    - NVIDIA-Nemotron-3-Nano-30B-A3B-Q4_K_M: **512k**
    - NVIDIA-Nemotron-3-Super-120B-A12B-UD-IQ4_XS: **384k**
    - gemma-4-12B-it-Q6_K: **256k**
- QV quantization:
    - tested with half-precision only (no quantization)

## Speed test
Each model inference speed is tested (on my hardware), with different context length, and different prompt processing batch-size.
The results are in CSV files (I've written them manually).  
I wrote a script at [create-svg.py](speed-test/create-svg.py) that convert the CSV files to SVG images for better visualization.  
MTP supporting models (Gemma) also has speed tests for MTP enhanced inference.  

**Here an example** (NVIDIA-Nemotron-3-Nano-30B-A3B-Q4_K_M, KV-Cache: FP16, Context-Length: 512k)  
<img width="500" src="speed-test/NVIDIA-Nemotron-3-Nano-30B-A3B-Q4_K_M/KV-Cache-FP16/512k-speed.svg"/>
<img width="500" src="speed-test/NVIDIA-Nemotron-3-Nano-30B-A3B-Q4_K_M/KV-Cache-FP16/512k-ttft.svg"/>

## Claude Code testing

- The tests are performed in 4 fields:
    - Low-Level (CUDA/C++)
    - Security: (C++)
    - Python
    - Web

> [!IMPORTANT]
> The agent can't run C++ & CUDA code!

> [!NOTE]
> For the web tests, the agent can run a headless browser.

Each test is perfumed in auto mode, with the following request:
```
/clean
/effort max
Do the tasks in <The-Task-File.txt>
Put the solution in the folder "<Where-To-Put-The-Solution>"
```
No more information is given to the agent!
However, since the model start a task it has full control over the flow. If the model ask the user for more information, it will be provided.

## Agentic evaluation summary
Here is an evaluation summery of the agentic task tests given to each model.

### CUDA
> [!WARNING]
> Nothing here yet. This section will be filled soon

### Python

Task summary:  
*Create a command‑line program that reads one Sudoku puzzle from console, solves it using "backtracking with constraint propagation" (bitmask + MRV heuristic), and prints the solved grid to console. No GUI or external libraries are required.*

| Model                                | Input | No Solution | Solving Skill | Efficiency | Trailing Code | Code Cleanness | Summary |
| ------------------------------------ | ----- | ----------- | ------------- | ---------- | ------------- | -------------- | ------- |
| Gemma-4-12B-it                       | 3     | 3           | 0             | 0          | 3             | 4              | 13/30   |
| NVIDIA-Nemotron-3-Nano-30B-A3B       | 3     | 3           | 5             | 4          | 2             | 5              | 24/30   |
| NVIDIA-Nemotron-3-Super-120B-A12B-UD | 4     | 5           | 5             | 4          | 4             | 5              | 27/30   |


### Security

Task summary:  
*Determine whether the supplied source code (`source.cpp`) is secure.*

**Test-1**  

| Model                                | What you found? | False security issues | How could be it badly used? | Source code lines | Fix | Summary |
| ------------------------------------ | --------------- | --------------------- | --------------------------- | ----------------- | --- | ------- |
| Gemma-4-12B-it                       | 5               | 5                     | 3                           | 4                 | 5   | 22/25   |
| NVIDIA-Nemotron-3-Nano-30B-A3B       | 5               | 5                     | 3                           | 4                 | 5   | 22/25   |
| NVIDIA-Nemotron-3-Super-120B-A12B-UD | 5               | 5                     | 5                           | 5                 | 5   | 25/25   |


**Test-2**  

| Model                                | What you found? | False security issues | How could be it badly used? | Source code lines | Fix | Summary |
| ------------------------------------ | --------------- | --------------------- | --------------------------- | ----------------- | --- | ------- |
| Gemma-4-12B-it                       | 3               | 2                     | 3                           | 4                 | 5   | 17/25   |
| NVIDIA-Nemotron-3-Nano-30B-A3B       | 0               | 5                     | 0                           | 0                 | 3   | 8/25    |
| NVIDIA-Nemotron-3-Super-120B-A12B-UD | 5               | 5                     | 4                           | 4                 | 5   | 23/25   |

**Test-3**  

| Model                                | What you found? | False security issues | Source code lines | Fix | Summary |
| ------------------------------------ | --------------- | --------------------- | ----------------- | --- | ------- |
| Gemma-4-12B-it                       | 4               | 4                     | 4                 | 5   | 17/20   |
| NVIDIA-Nemotron-3-Nano-30B-A3B       | 0               | 5                     | 0                 | 0   | 5/20    |
| NVIDIA-Nemotron-3-Super-120B-A12B-UD | 4               | 1                     | 4                 | 5   | 14/20   |


### Web
> [!WARNING]
> Nothing here yet. This section will be filled soon

