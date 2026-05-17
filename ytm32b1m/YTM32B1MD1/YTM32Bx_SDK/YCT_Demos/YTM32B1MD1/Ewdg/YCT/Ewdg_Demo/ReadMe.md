# Ewdg_Demo

---

## Function Description

This example demonstrates two functionalities of the EWDG module:

- When the EWDG_IN signal does not match the configured value, the watchdog can be successfully refreshed. The EWDG_OUT pin will assert low upon timeout.
- When the EWDG_IN signal matches the configured value, the watchdog cannot be refreshed. Any refresh attempt will immediately drive EWDG_OUT low.
- Since the EWDG module can only be initialized once, the self-test procedure uses a system reset to validate the second functionality.
- For self-test, the user should short PTE17 to EWDG_OUT and PTE13 to EWDG_IN.

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