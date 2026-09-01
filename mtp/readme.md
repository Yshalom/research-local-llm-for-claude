# Llama.cpp Backend Proxy for Multi-Token Prediction (MTP)

## Project Role & Context
This directory hosts a specialized **Llama.cpp Backend Proxy** developed as part of our ongoing research into local LLM performance and benchmarking. This project serves as critical infrastructure for evaluating advanced inference techniques; specifically, it enables the testing of **Multi-Token Prediction (MTP)** capabilities for models like Gemma-4 in environments that lack native support (such as LM-Studio). By providing a mechanism to inject MTP logic into standard inference pipelines, this proxy allows us to benchmark these advanced decoding techniques.

## Abstract
The **Llama.cpp Backend Proxy** is a custom middleware solution designed to facilitate the use of Gemma-4's **Multi-Token Prediction (MTP)** capabilities within environments that do not natively support them—specifically LM-Studio.

Because standard interfaces in LM-Studio may lack direct hooks for the specific speculative decoding types required by Gemma-4's architecture, this project implements an **Interceptor Pattern**. The proxy acts as a layer between the UI and the inference engine (`llama.cpp`), intercepting raw command-line strings, performing semantic transformations based on a custom rule engine, and re-dispatching the modified commands to the underlying server to enable Multi-Token Prediction (MTP) support.

---

## Technical Architecture

### 1. Interception & Transformation Pipeline
The core logic is driven by the `CommandProcessor`. When the proxy is invoked, it follows a three-stage pipeline:
- **Ingestion**: The raw command line is parsed into a discrete list of key-value pairs (flags and values).
- **Rule Evaluation**: Each rule defined in `proxy-settings.json` is evaluated against the current argument set. This includes conditional logic (`when` blocks), enabling context-aware transformations based on specific model paths or flags.
- **Reconstruction**: The modified arguments are serialized back into a command string, prefixed with the target executable path for final execution.

### 2. Process Lifecycle Management
To ensure system stability and prevent "zombie" or orphaned processes, the proxy utilizes **Windows Job Objects**. By assigning the child process (the actual `llama.cpp` server) to a job object with the `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE` flag, the proxy guarantees that all resources associated with the inference engine are released immediately upon the termination of the proxy process.

### 3. Configuration Schema
The proxy's behavior is governed by a JSON configuration file (`proxy-settings.json`). This architecture allows for modular updates without requiring recompilation:
- `forward to`: The absolute path to the target executable.
- `replace`: A list of substitution rules containing:
    - `from` / `to`: Direct string replacements.
    - `from_value` / `to_value`: Value-specific substitutions for specific flags.
    - `when`: A set of mandatory conditions (e.g., specific model paths) that must be met for the rule to trigger.

---

## Deployment & Integration

To integrate the proxy into LM-Studio as a backend engine, you must provide a complete set of artifacts that satisfy LM-Studio's backend requirements.

### Backend Directory Structure
The project is organized so that multiple backends can coexist in the `backends` directory (each housed in its own sub-directory). The specific implementation for this project is located at:
`backends/llama.cpp-win-x86_64-nvidia-cuda12-avx2-2.27.1-proxy`

This folder contains the proxy and its configuration, which points to the original `llama.cpp` server located in the sibling directory:
`backends/llama.cpp-win-x86_64-nvidia-cuda12-avx2-2.27.1`

### Required Files
The following files must be present within the **proxy** folder:

| File | Description | Source/Status |
| --- | --- | --- |
| **`llama-server.exe`** | **The Proxy Executable.** The core compiled binary that handles interception and forwarding. | **New / Compiled** |
| **`proxy-settings.json`** | **Proxy Configuration.** Defines the forward path to the original `llama.cpp` server and semantic replacement rules (including MTP logic). | **New** |
| `backend-manifest.json` | Metadata describing the backend engine to LM-Studio. | Copied & Modified |
| `display-data.json` | UI metadata for display names and descriptions. | Copied & Modified |
| `lmstudiocore.dll` | Core binding library for LM-Studio. | Original |
| `llm_engine_cuda12.node` | Node.js bindings for the LLM engine. | Original |
| `liblmstudio_bindings_cuda12.node` | CUDA-specific bindings. | Original |
| `engine-protocol-server-artifacts.json` | Protocol artifacts. | Original |

### Setup Procedure & Modifications
The following specific modifications are required during setup:

1. **Backend Placement**: Place all files listed in the table above into the proxy-specific backend folder.
2. **Manifest Modification**: Update the `"name"` property in `backend-manifest.json`:
    *   *Original*: `"name": "llama.cpp-win-x86_64-nvidia-cuda12-avx2"`
    *   *Modified*: `"name": "llama.cpp-win-x86_64-nvidia-cuda12-avx2-proxy"`
3. **Display Configuration**: Update the `displayName` and `description` properties in `display-data.json`:
    *   *Original displayName*: `"CUDA 12 llama.cpp (Windows)"`
    *   *Modified displayName*: `"CUDA 12 llama.cpp (Windows) Proxy"`
    *   *Original description*: `"Nvidia CUDA 12.8 accelerated llama.cpp engine"`
    *   *Modified description*: `"Nvidia CUDA 12.8 accelerated llama.cpp engine proxy"`
4. **Runtime Configuration**: Configure `proxy-settings.json` to point to the correct path of the original `llama-server.exe` and define the specific model path triggers for Gemma-4.

---

## Usage Example: Multi-Token Prediction (MTP)
The primary use case is enabling **Multi-Token Prediction (MTP)** via speculative decoding. The proxy automatically detects when a user attempts to load a Gemma-4 variant with a standard draft type and converts it to support MTP logic:

**Original Command (from LM-Studio):**
`... --model "path/to/gemma-4" --spec-type draft-simple --spec-draft-model "path/to/assistant"`

**Proxy Transformation:**
The proxy identifies the model path in the `when` block and performs the following replacement:
`--spec-type draft-simple` $\rightarrow$ `--spec-type draft-mtp`

---
