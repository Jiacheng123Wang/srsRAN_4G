# Product Context: srsRAN_4G

## Purpose
srsRAN_4G provides a complete, open-source software implementation of the LTE and partial NR protocol stacks. Its primary purpose is to democratize access to LTE/NR technology by allowing researchers, developers, and engineers to deploy a full cellular network without relying on proprietary vendor hardware and software.

## Target Audience
- **Academic Researchers**: For testing new PHY layer algorithms, MAC scheduling policies, or RRC procedures.
- **SDR Enthusiasts**: For building private LTE networks using USRP or LimeSDR hardware.
- **Telecom Engineers**: For prototyping 4G/5G features and understanding the 3GPP standard in a practical implementation.
- **Software Developers**: For integrating cellular connectivity into custom embedded systems.

## Key Use Cases
- **Private LTE Network**: Deploying a standalone eNodeB and EPC to provide private wireless coverage.
- **Protocol Testing**: Using `srsue` and `srsenb` in a simulated environment (ZeroMQ) to verify protocol compliance.
- **Algorithm Benchmarking**: Testing the performance of FEC (Turbo/LDPC) or synchronization algorithms against real-world IQ captures.
- **Education**: Learning the end-to-end flow of a cellular packet from the UE to the Core Network.

## Value Proposition
- **Open Source**: Full transparency and modifiability of the protocol stack.
- **Hardware Agnostic**: Supports a wide range of SDRs via a unified abstraction layer.
- **Complete Ecosystem**: Includes the UE, Base Station, and Core Network, eliminating the need for third-party EPCs.
- **High Performance**: Optimized C/C++ implementation capable of real-time operation.