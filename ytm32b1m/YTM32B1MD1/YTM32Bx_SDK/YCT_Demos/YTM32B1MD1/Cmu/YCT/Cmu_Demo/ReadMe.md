# Cmu_Demo 

---

## Function Description

This example demonstrates the CMU functionality and self-test procedures:

- System reset upon clock loss, and interrupt generation when the clock exceeds the specified range.
- The self-test manually adjusts the frequency range of the FXOSC in the CMU to simulate an out-of-range clock scenario.
- An interrupt is triggered immediately when the clock goes out of range, followed by a software-initiated reset.
- After reset, the CMU status is verified to confirm the system state.

---

## Hardware Requirements

+ **EVB**: YTM32B1MD1-EVB-Q100
+ **Debugger**: J-LINK v9.2 or later version

---

## Software Requirements

+ **SDK**      : YTM32B1MD1_1_4_0 or later version
+ **GCC**      : GNU GCC v10.3 or later version
+ **Keil/MDK** : ARM Keil/MDK v5.36 or later version
+ **IAR***     : IAR v8.4 or later version
+ **VS Code**  : Microsoft Visual Studio Code IDE v1.76 or later version(GNU GCC toolchain, CMake v3.25, ninja, Cortex-Debug extensions installation required)
+ **J-LINK** : SEGGER JLINK v6.96 or later version, YTM32B1MD1 patch(support package) installation required.
+ **Ozone**  : SEGGER Ozone v3.28 or later version.
  
---