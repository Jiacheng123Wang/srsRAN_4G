# Project Brief: srsRAN_4G

## Overview
srsRAN_4G is a comprehensive, open-source software implementation of the LTE (Long Term Evolution) and NR (New Radio) protocol stacks. It enables the creation of a complete LTE network using Software Defined Radio (SDR) hardware.

## Core Components
- **`srsue`**: The User Equipment (UE) implementation, simulating a mobile device.
- **`srsenb`**: The eNodeB implementation, simulating a LTE base station.
- **`srsepc`**: The Evolved Packet Core (EPC) implementation, simulating the core network (MME, HSS, SPGW).
- **`lib`**: The shared core library containing the 3GPP protocol stack implementations (PHY, MAC, RLC, PDCP, RRC).

## Primary Objectives
- Provide a fully functional, open-source LTE network stack.
- Ensure high performance and 3GPP compliance.
- Maintain hardware independence through a robust SDR abstraction layer.
- Support both real-time hardware deployment and simulated environments (e.g., via ZeroMQ).