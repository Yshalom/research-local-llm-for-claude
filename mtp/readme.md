# Llama.cpp Backend Proxy for Multi-Token Prediction (MTP)

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

To integrate the proxy into LM-Studio as a backend engine:

1.  **Backend Placement**: Place the compiled `llama-server.exe` (the proxy) and its associated configuration files in the LM-Studio `backends` directory.
2.  **Manifest Modification**: 
    - Copy `backend-manifest.json` from the original `llama.cpp` distribution.
    - Update the `"name"` property to include a `-proxy` suffix (e.g., `llama.cpp-win-x86_64-nvidia-cuda12-avx2-proxy`).
3.  **Display Configuration**:
    - Modify `display-data.json` to update the `displayName` and `description` fields, ensuring clarity within the LM-Studio UI.
4.  **Runtime Configuration**: 
    - Configure `proxy-settings.json` to point to your local `llama.cpp` installation and define the specific model path triggers for Gemma-4.

---

## Usage Example: Multi-Token Prediction (MTP)
The primary use case involves enabling **Multi-Token Prediction (MTP)** via speculative decoding. The proxy automatically detects when the user attempts to load a Gemma-4 variant with a standard draft type and converts it to support MTP logic:

**Original Command (from LM-Studio):**
`... --model "path/to/gemma-4" --spec-type draft-simple --spec-draft-model "path/to/assistant"`

**Proxy Transformation:**
The proxy identifies the model path in the `when` block and replaces:
`--spec-type draft-simple` $\rightarrow$ `--spec-type draft-mtp`

---
