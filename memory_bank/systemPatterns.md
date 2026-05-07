# System Patterns: srsRAN_4G

## 1. Architectural Patterns

### Core Library + Application Pattern
The project is split into a shared core (`lib/`) and specific network role applications (`srsue`, `srsenb`, `srsepc`). This ensures that the 3GPP protocol logic is implemented once and reused across different roles, reducing duplication and ensuring consistency.

### Layered Protocol Stack
The system strictly follows the 3GPP layered architecture:
`PHY` $\leftrightarrow$ `MAC` $\leftrightarrow$ `RLC` $\leftrightarrow$ `PDCP` $\leftrightarrow$ `RRC/Upper Layers`.
Each layer provides a specific service to the layer above it and consumes services from the layer below, allowing for modular updates to specific protocol versions.

### Hardware Abstraction Layer (HAL)
To support various SDR hardware (USRP, LimeSDR, etc.), the project uses a HAL pattern in `lib/src/radio`. This decouples the high-level signal processing from the low-level hardware-specific API calls.

---

## 2. Design Patterns

### Strategy Pattern (Scheduling)
In `srsenb`, different scheduling algorithms can be swapped. The MAC layer uses a strategy-like approach to decide how to allocate physical resources based on UE channel quality and priority.

### State Machine Pattern
Both `srsue` and `srsenb` rely heavily on state machines to manage the LTE connection lifecycle (e.g., `IDLE` $\rightarrow$ `RRC_CONNECTED` $\rightarrow$ `DISCONNECTED`).

### Observer/Callback Pattern
Used extensively for event handling between layers (e.g., the PHY layer notifying the MAC layer about a successful HARQ transmission or a synchronization event).

---

## 3. Concurrency & Performance Patterns

### Worker Pool Pattern
To handle the computationally intensive PHY layer processing in real-time, `srsenb` employs a worker pool (`srsenb/src/phy/lte/worker_pool.cc`). This allows the system to parallelize the processing of subframes across multiple CPU cores.

### Zero-Copy / Buffer Management
Given the high data rates of LTE, the system uses optimized buffer management (e.g., `base_ue_buffer_manager.cc`) to minimize memory copying between the protocol layers.

### Real-time Processing Loop
The system operates on a strict TTI (Transmission Time Interval) of 1ms. The execution flow is designed as a high-priority loop that must complete all PHY/MAC processing within this window to avoid "under-runs" or "over-runs" on the SDR hardware.