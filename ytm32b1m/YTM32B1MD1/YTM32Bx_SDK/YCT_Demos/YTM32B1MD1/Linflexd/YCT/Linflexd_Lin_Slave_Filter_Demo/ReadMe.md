# Linflexd_Lin_Slave_Filter_Demo

---

## Function Description

This example demonstrates the LIN slave filter functionality along with a self-test procedure:

- LIN1 operates as LIN Master, LIN2 operates as LIN Slave
- Short-circuit the LIN bus lines between LIN1 and LIN2 to perform self-test
- Apply 12V voltage to the VBAT pins of both LIN1 and LIN2
- Master sends frame header with ID 0x22. After transmission, the slave enables reception to receive the remaining frame data from the master
- Master sends frame header with ID 0x11. After transmission, the slave enables response transmission, and the master receives the data sent by the slave
- Master sends frame header with ID 0x33. This ID is not within the slave's ID Filter range and will be discarded by the slave
- Transmitted and received data from both master and slave are output via UART for verification

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