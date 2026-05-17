# Linflexd_Uart_Idle_Interrupt_Demo

---

## Function Description

This project demonstrates how to use the UART module's idle interrupt mode.

- Short PTC2 to PTC3 to test the routine
- The UART receives variable-length data (where the length is smaller than the receiver's buffer capacity)
- This triggers an idle error interrupt in the UART module
- The UART aborts the current transfer and initiates a new transmission

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