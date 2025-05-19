## Table of Contents

- [Project Overview](#project-overview)
- [Fundamental Concept of PMSM FOC](#fundamental-concept-of-pmsm-foc)
- [Software Stack](#software-stack)
- [Configuration](#configuration)
- [Build Instructions](#build-instructions)
- [Contributing](#contributing)
  - [New Feature](#new-feature)

# motor control firmware

A Zephyr firmware implementation for controlling a 3-phase sensorless PMSM motor with a field oriented
control algorithm approach.

## Fundamental concept of PMSM FOC

The field oriented control approach is meant to achieve high performance motor control with smooth
rotation over the entire speed range of the motor, full torque at zero speed and fast acceleration/deceleration.
The algorithm is an efficient tool for achieving high dynamic control.


## Software stack
This project uses [Zephyr](https://www.zephyrproject.org/) as the development framework.
Zephyr was chosen for it's portability between processors, and for the good built-in support for Ethernet and CAN bus.

## Configuration

The target board is the i.MX RT1060 which is a new processor family featuring NXP’s advanced implementation of the Arm Cortex®-M7 core,
which operates at speeds up to 600 MHz to provide high CPU performance and best real-time response. And is well suited for application
in motor control. It has great Zephyr support. The board is designated `mimxrt1060_evk`.

## Build instructions
At time of writing, this project is being built and tested with Zephyr v4.0

    west build -b mimxrt1060_evk --pristine
    west flash


## Contributing

Most of our contributing guidelines are taken from the Zephyr project, so please refer to the [Zephyr contributing
guidelines](https://docs.zephyrproject.org/latest/contribute/guidelines.html) for more information and help.

### New feature

Create a feature branch by prefixing with your name `feat/`.
