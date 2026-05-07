# Active Context for srsRAN_4G

## 1. Project Architecture Summary (From architecture.md)
srsRAN_4G is a high-performance, open-source implementation of the LTE and NR protocol stacks. It provides a complete SDR solution.

**Design Pattern**: "Core Library + Application"
**Main Modules**:
- **`lib/` (Core Library)**: Contains shared protocol logic (PHY, MAC, RLC, PDCP, RRC, ASN1, radio).
- **`srsenb/`**: eNodeB Application (Base Station).
- **`srsue/`**: UE Application (User Equipment).
- **`srsepc/`**: EPC Application (Core Network: MME, HSS, SPGW).

**Data Flow**:
- **Control Plane**: `srsue (RRC)` $\leftrightarrow$ `srsenb (RRC)` $\leftrightarrow$ `srsenb (S1AP)` $\leftrightarrow$ `srsepc (MME)` $\leftrightarrow$ `srsepc (HSS)`
- **User Plane**: `srsue` $\leftrightarrow$ `SDR Hardware` $\leftrightarrow$ `srsenb` $\rightarrow$ `srsepc (SPGW)` $\rightarrow$ `Internet`

**Technology Stack**:
- **Languages**: C and C++
- **Build System**: CMake
- **Key Libraries**: FFTW3/MKL, MbedTLS, SCTP, ZeroMQ, RapidJSON, UHD/SoapySDR.

## 2. Remote Compilation & Execution Environment (New Information)
The project requires compilation on an Ubuntu system, as the current machine is macOS. A remote Ubuntu machine named `nuc5GC` is available for this purpose.

**Machine Details**:
- **OS**: Ubuntu
- **Code Repository Path**: `~/work/srsRAN_4G`
- **Startup Command (EPC/eNB)**: `~/work/srsRAN_4G/start_epc_enb.sh`
- **Shutdown Command (EPC/eNB)**: `~/work/srsRAN_4G/stop_epc_enb.sh`
- **Recompile Command (Full Project)**: `~/work/srsRAN_4G/compile_epc_enb.sh`
- **EPC Terminal Output**: `/home/jwang/work/srsRAN_4G/srs_config/epc.log`
- **EPC Runtime Log**: `/tmp/epc.log`
- **eNB Terminal Output**: `/home/jwang/work/srsRAN_4G/srs_config/enb.log`
- **eNB Runtime Log**: `/tmp/enb.log`

---
*This context file is automatically updated by Cline.*