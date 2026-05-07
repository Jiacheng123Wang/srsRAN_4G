# Technical Context: srsRAN_4G

## 1. Language & Toolchain
- **Languages**: 
    - **C**: Used primarily in the Physical Layer (`lib/src/phy`) for maximum performance and deterministic memory management.
    - **C++**: Used for higher layers (MAC, RLC, PDCP, RRC) and application logic to leverage object-oriented design for complex state management.
- **Build System**: `CMake` is used for cross-platform build configuration.
- **Compiler Requirements**: Requires a C++11 (or newer) compatible compiler (e.g., GCC, Clang).

## 2. Key Technical Dependencies

### ⚡ Signal Processing & Math
- **FFTW3 / Intel MKL**: Essential for performing Fast Fourier Transforms (FFT), which are the core of OFDM modulation/demodulation.
- **Custom FEC Implementations**: High-performance implementations of Turbo codes, LDPC, and Polar codes.

### 🔐 Security & Networking
- **MbedTLS**: Provides the cryptographic primitives required for LTE NAS/AS security (ciphering and integrity protection).
- **SCTP (Stream Control Transmission Protocol)**: Used for the S1-MME interface between the eNodeB and MME, as required by 3GPP standards.
- **ZeroMQ**: Enables the "Remote Radio Head" (RRH) mode, allowing the protocol stack to run on one machine and the radio interface on another, or to simulate the air interface via files/sockets.

### 🛠️ Hardware Abstraction
- **UHD (USRP Hardware Driver)**: Primary interface for Ettus/NI USRP devices.
- **SoapySDR**: A vendor-neutral library providing a common API for various SDR hardware (LimeSDR, BladeRF, etc.).

## 3. Performance & Real-time Constraints
- **TTI (Transmission Time Interval)**: The system is hard-constrained by the LTE TTI of **1ms**. All PHY and MAC processing for a subframe must be completed within this window.
- **Parallelism**: 
    - **Worker Pools**: The eNodeB uses a worker pool to distribute the processing of multiple Component Carriers (CC) and subframes across available CPU cores.
    - **SIMD**: Utilizes SSE/AVX instructions where possible to accelerate signal processing.

## 4. Configuration & Data Formats
- **Configuration Files**: Uses human-readable `.conf` files (e.g., `enb.conf`, `ue.conf`) to define network parameters, frequency, and hardware settings.
- **ASN.1**: All 3GPP signaling messages are defined using ASN.1 (Abstract Syntax Notation One) and are compiled into C/C++ structures for transmission.
- **JSON**: Used for exporting real-time metrics and system telemetry.

## 5. Development & Testing Environment
- **Simulation Mode**: Supports running the entire network (UE $\rightarrow$ eNB $\rightarrow$ EPC) on a single machine using ZeroMQ, removing the need for physical SDR hardware during initial development.
- **PCAP Logging**: Ability to capture and log MAC/PHY layer traffic for offline analysis.
- **Spec Interpretation**: 本地协议 Markdown 存在渲染瑕疵。Cline 应具备“源码阅读能力”，透过渲染错误识别底层的 LTE 物理层数学模型。