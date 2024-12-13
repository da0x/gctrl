# gctrl

General Controls Engine

## Overview
gctrl is a powerful integrated development environment (IDE) and framework for building modular, diagram-driven controls code in C++ for embedded systems. It serves as an engine with full C++ code generation capabilities for any control system. The generated code is cross-platform and works seamlessly on any embedded system, Linux, or microcontroller.

<img width="1271" alt="Screenshot 2024-12-13 070035" src="https://github.com/user-attachments/assets/1709ca82-60ba-4309-a234-7f8fcd8b8e68" />
---

## Core Concepts

### System
A **system** is a collection of subsystems, each with clearly defined behaviors and application programming interfaces (APIs).

### Subsystem
A **subsystem** comprises physical and logical devices, some of which:
- Run software.
- Accept settings for operation.

### Machine (Target)
A **machine**, or target, is an embedded device that hosts and executes one or more controllers and device drivers.

### Controller
A **controller** is a logical device representing software functions and their associated interfaces. It consists of:
- **Control Elements**: Algorithms with defined inputs and outputs.
- **Ports**: Objects facilitating data exchange between controllers and drivers.

Controllers are composed of interconnected control elements and ports, enabling defined signal flows:
- **Inputs** can be constants or outputs from other elements.
- **Outputs** can be signals to other elements or external interfaces.

### Element
An **element** is an algorithm with:
- A clearly defined function.
- Signal inputs and outputs that specify all required data and provided results.

### Port
A **port** is an object that facilitates data exchange between controllers and/or drivers. Ports manage two signal buses:
1. **Plug Data**
2. **Socket Data**

When instantiated within a controller, ports expose signals as follows:
- **Plug Data**: Outgoing signals, sourced from element outputs.
- **Socket Data**: Incoming signals, connected to element inputs.

Each port instance is defined as either a **plug** or a **socket**, determining which signals are considered incoming or outgoing. This definition enforces compatibility between port connections in controllers and drivers. For example:
- A **plug** port exposes its plug data as outgoing signals and socket data as incoming signals.
- A **socket** port reverses this arrangement.

### Driver
A **driver** is a module designed to:
- Interface with physical devices.
- Handle external communications.

Drivers use port instances (plug or socket) to interact with controllers on the same machine.

---

## Diagram-Driven Development
The gctrl framework leverages diagrams to visually represent and structure systems, subsystems, controllers, elements, ports, and drivers. This approach simplifies understanding and maintenance of complex embedded systems.

---

## Project Status
This project is currently in the **alpha stage**. 

To access the latest project version, please switch to the `alpha` branch.

---

## Contributing
We welcome contributions to gctrl! Please refer to the [CONTRIBUTING.md](CONTRIBUTING.md) file for details on how to get involved.

### Community Engagement
Help us build a strong community by:
- Starring this repository.
- Participating in the [Discussions](https://github.com/da0x/gctrl/discussions) section to share ideas, ask questions, and collaborate.

---

## Summary
gctrl provides a structured, modular approach to embedded system development, with clear delineations between components and robust mechanisms for data exchange and interaction. Its diagram-driven methodology enhances both clarity and maintainability of the codebase. The engine's C++ code generation ensures compatibility across platforms, including embedded systems, Linux, and microcontrollers.
