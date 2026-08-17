# Llama.cpp Backend Proxy for Multi-Token Prediction (MTP)

## Project Role & Context
This directory contains a specialized **Llama.cpp Backend Proxy** used as part of our research into local LLM performance and benchmarking. Within the broader scope of evaluating different models, this project serves as an essential infrastructure utility: it enables the testing of **Multi-Token Prediction (MTP)** capabilities for models like Gemma-4 in environments that lack native support (specifically LM-Studio). By providing a way to inject MTP logic into standard inference pipelines, it allows us to include and accurately benchmark these advanced speculative decoding techniques.

## Abstract
The **Llama.cpp Backend Proxy** is a specialized middleware solution designed to facilitate the use of Gemma-4's **Multi-Token Prediction (MTP)** capabilities within environments that do not natively support them—specifically, LM-Studio. 

Because standard interfaces in LM-Studio may lack direct hooks for specific speculative decoding types required by Gemma-4's architecture, this project implements an **Interceptor Pattern**. The proxy sits between the UI and the inference engine (`llama.cpp`), intercepting raw command-line strings, performing semantic transformations based on a rule engine, and re-dispatching the modified commands to the underlying server to enable Multi-Token Prediction (MTP) support.

---

## Technical Architecture

### 1. Interception & Transformation Pipeline
The core logic resides in the `CommandProcessor`. When the proxy is invoked:
- **Ingestion**: The raw command line is parsed into a discrete list of key-value pairs (flags and values).
- **Rule Evaluation**: Each rule in `proxy-settings.json` is evaluated against the current argument set. This includes conditional logic (`when` blocks), allowing for context-aware transformations.
- **Reconstruction**: The modified arguments are serialized back into a command string, prefixed with the target executable path.

### 2. Process Lifecycle Management
To ensure system stability and prevent "zombie" or orphaned processes, the proxy utilizes **Windows Job Objects**. By assigning the child process (the real `llama.cpp` server) to a job object with the `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE` flag, the proxy guarantees that all resources associated with the inference engine are released immediately upon termination of the proxy process.

### 3. Configuration Schema
The proxy's behavior is governed by a JSON configuration file (`proxy-settings.json`). This allows for modular updates without recompilation:
- `forward to`: The absolute path to the target executable.
- `replace`: A list of substitution rules containing:
    - `from` / `to`: Direct string replacements.
    - `from_value` / `to_value`: Value-specific substitutions for flags.
    - `when`: A set of mandatory conditions (e.g., specific model paths) that must be met for the rule to trigger.

---

## Deployment & Integration

To integrate the proxy into LM-Studio as a backend engine, it is necessary to provide a complete set of artifacts that satisfy LM-Studio's backend requirements. 

### Required Files
The following files must exist in the `backends` directory:

| File | Description | Source/Status |
| :--- | :--- | :--- |
| **`llama-server.exe`** | **The Proxy Executable.** The core compiled binary that handles interception and forwarding. | **New / Compiled** |
| **`proxy-settings.json`** | **Proxy Configuration.** Defines the forward path and semantic replacement rules (including MTP logic). | **New** |
| `backend-manifest.json` | Metadata describing the backend engine to LM-Studio. | Copied & Modified |
| `display-data.json` | UI metadata for display names and descriptions. | Copied & Modified |
| `lmstudiocore.dll` | Core binding library for LM-Studio. | Original |
| `llm_engine_cuda12.node` | Node.js bindings for the LLM engine. | Original |
| `liblmstudio_bindings_cuda12.node` | CUDA-specific bindings. | Original |
| `engine-protocol-server-artifacts.json` | Protocol artifacts. | Original |

### Setup Procedure & Modifications
The following specific modifications are required during the setup:

1.  **Backend Placement**: Place all files listed in the table above into the LM-Studio `backends` directory.
2.  **Manifest Modification**: Update the `"name"` property in `backend-manifest.json`:
    *   *Original*: `"name": "llama.cpp-win-x86_64-nvidia-cuda12-avx2"`
    *   *Modified*: `"name": "llama.cpp-win-x86_64-nvidia-cuda12-avx2-proxy"`
3.  **Display Configuration**: Update the `displayName` and `description` properties in `display-data.json`:
    *   *Original displayName*: `"CUDA 12 llama.cpp (Windows)"`
    *   *Modified displayName*: `"CUDA 12 llama.cpp (Windows) Proxy"`
    *   *Original description*: `"Nvidia CUDA 12.8 accelerated llama.cpp engine"`
    *   *Modified description*: `"Nvidia CUDA 12.8 accelerated llama.cpp engine proxy"`
4.  **Runtime Configuration**: Configure `proxy-settings.json` to point to your local `llama.cpp` installation and define the specific model path triggers for Gemma-4.

---

## Usage Example: Multi-Token Prediction (MTP)
The primary use case involves enabling **Multi-Token Prediction (MTP)** via speculative decoding. The proxy automatically detects when the user attempts to load a Gemma-4 variant with a standard draft type and converts it to support MTP logic:

**Original Command (from LM-Studio):**
`... --model "path/to/gemma-4" --spec-type draft-simple --spec-draft-model "path/to/assistant"`

**Proxy Transformation:**
The proxy identifies the model path in the `when` block and replaces:
`--spec-type draft-simple` $\rightarrow$ `--spec-type draft-mtp`

---
