# srsRAN_4G Project Architecture

## 1. Project Overview
srsRAN_4G is a high-performance, open-source implementation of the LTE (Long Term Evolution) and NR (New Radio) protocol stacks. It provides a complete software-defined radio (SDR) solution, allowing users to deploy a full LTE network (UE, eNodeB, and EPC) using commodity hardware.

The project follows a **"Core Library + Application"** design pattern, where the complex protocol logic is decoupled from the specific network role.

---

## 2. Main Modules & Functions

### 📦 Core Library (`lib/`)
The `lib` directory contains the shared protocol implementation used by all applications.
- **`phy/` (Physical Layer)**: Handles the air interface. Includes synchronization, channel estimation, MIMO processing, and FEC (Forward Error Correction) like Turbo and LDPC codes.
- **`mac/` (Medium Access Control)**: Manages scheduling, HARQ (Hybrid ARQ), and resource allocation.
- **`rlc/` (Radio Link Control)**: Handles segmentation, concatenation, and ARQ for reliable data delivery.
- **`pdcp/` (Packet Data Convergence Protocol)**: Manages header compression (ROHC), ciphering, and integrity protection.
- **`rrc/` (Radio Resource Control)**: The "brain" of the control plane, managing connection states, cell reselection, and system information.
- **`asn1/`**: Provides serialization/deserialization for 3GPP protocol messages.
- **`radio/`**: Hardware Abstraction Layer (HAL) for interfacing with SDRs (UHD, SoapySDR, etc.).

### 📡 eNodeB Application (`srsenb/`)
Implements the LTE Base Station functionality.
- **Scheduler**: Implements the logic to allocate physical resources to different UEs.
- **S1AP Stack**: Manages the S1 interface for communication with the Core Network (MME).

### 📱 UE Application (`srsue/`)
Implements the LTE User Equipment (Mobile Phone) functionality.
- **Cell Search**: Implements PSS/SSS detection to find and synchronize with a cell.
- **UE State Machine**: Manages the transition between IDLE and CONNECTED states.

### ☁️ EPC Application (`srsepc/`)
Implements the Evolved Packet Core (Core Network).
- **MME (Mobility Management Entity)**: Handles UE authentication, tracking area management, and session setup.
- **HSS (Home Subscriber Server)**: Central database for user profiles and security keys.
- **SPGW (Serving/PDN Gateway)**: Routes user data packets between the eNB and the external internet.

---

## 3. Data Flow

### 🛠️ Control Plane (Signaling)
Used for connection setup, authentication, and mobility management.
**Flow**: `srsue (RRC)` $\leftrightarrow$ `srsenb (RRC)` $\leftrightarrow$ `srsenb (S1AP)` $\leftrightarrow$ `srsepc (MME)` $\leftrightarrow$ `srsepc (HSS)`

### 📦 User Plane (Data)
Used for actual internet traffic (IP packets).
**Flow**: `srsue (PDCP $\rightarrow$ RLC $\rightarrow$ MAC $\rightarrow$ PHY)` $\leftrightarrow$ `SDR Hardware` $\leftrightarrow$ `srsenb (PHY $\rightarrow$ MAC $\rightarrow$ RLC $\rightarrow$ PDCP)` $\rightarrow$ `srsenb (GTP-U)` $\rightarrow$ `srsepc (SPGW)` $\rightarrow$ `Internet`

---

## 4. Technology Stack

### 💻 Languages & Build System
- **Languages**: C and C++ (Mixed). C is primarily used for the PHY layer for performance, while C++ is used for higher layers and application logic.
- **Build System**: CMake.
- **Configuration**: Custom `.conf` files for each component.

### 📚 Key Libraries & Dependencies
- **Signal Processing**: 
    - `FFTW3` / `Intel MKL`: For Fast Fourier Transforms (FFT).
- **Security & Networking**:
    - `MbedTLS`: For encryption and integrity protection.
    - `SCTP`: For reliable signaling transport on the S1 interface.
    - `ZeroMQ`: Used for remote radio head (RRH) communication and simulation.
- **Data Handling**:
    - `RapidJSON`: For JSON-based metrics and configuration.
    - `ASN1`: For 3GPP standard message encoding.
- **SDR Hardware Interfaces**:
    - `UHD` (USRP Hardware Driver), `SoapySDR`, `LimeSDR`.

---

## 5. Component Interaction Summary

| Component | Primary Interface | Primary Responsibility |
| :--- | :--- | :--- |
| **srsue** | LTE-Uu (Air) | Synchronization, Data Access |
| **srsenb** | LTE-Uu $\leftrightarrow$ S1 | Resource Scheduling, Radio Mgmt |
| **srsepc** | S1 $\leftrightarrow$ S5/S8 | Authentication, IP Routing |
| **lib** | Internal API | Protocol Logic Implementation |