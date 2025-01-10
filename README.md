# GCtrl: General System Controls Engine

GCtrl is a high-level programming language and IDE designed specifically for system controls engineers. The language enables efficient modeling, simulation, and deployment of control systems with a focus on modularity and portability.

---

## Overview

GCtrl offers:

- **Full C++ code generation**: Automatically generate cross-platform, high-performance C++ code for any control system.
- **Extensibility**: Design modular, reusable components for embedded systems, microcontrollers, and Linux-based platforms.
- **Simulation-ready**: Easily simulate systems with built-in tools for debugging and live data visualization.
- **Chronological design approach**: Organize system design from functions and elements to controllers and machine definitions for seamless integration.

<img width="1271" alt="Screenshot 2024-12-13 070035" src="https://github.com/user-attachments/assets/1709ca82-60ba-4309-a234-7f8fcd8b8e68" />

---

## Example Implementation

Here’s an example implementation of a thermostat system using GCtrl:

```gctrl
namespace gctrl {
    function logistic {
        input float x;
        output float control_signal;

        operation {
            control_signal = 1.0 / (1.0 + exp(-x));
        }
    }

    element low_pass_filter {
        input float current_value;
        input float smoothing_factor;
        output float filtered;
        memory float previous_value = 0.0;

        operation {
            filtered = smoothing_factor * current_value + (1.0 - smoothing_factor) * previous_value;
            previous_value = filtered;
        }
    }

    element thermal_regulator {
        input float setpoint;
        input float temperature;
        output float heating;
        output float cooling;
        memory float error;

        operation {
            error = setpoint - temperature;
            heating = logistic(error) * max(0.0, error);
            cooling = logistic(error) * max(0.0, -error);
        }
    }

    element humidity_regulator {
        input float setpoint;
        input float humidity;
        output float command;
        memory float error;

        operation {
            error = setpoint - humidity;
            command = logistic(error) * max(0.0, error);
        }
    }

    controller thermal_controller {
        element thermal_regulator thermal;
        element low_pass_filter lpfc;

        connection thermal.setpoint -> lpfc.filtered;
        connection lpfc.current_value -> thermal.temperature;
    }

    controller humidity_controller {
        element humidity_regulator humid;
        element low_pass_filter lpfh;

        connection humid.setpoint -> lpfh.filtered;
        connection lpfh.current_value -> humid.humidity;
    }

    machine thermostat {
        controller thermal_controller tc;
        controller humidity_controller hc;

        connection tc.heating -> hc.command;
    }
}
```

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
