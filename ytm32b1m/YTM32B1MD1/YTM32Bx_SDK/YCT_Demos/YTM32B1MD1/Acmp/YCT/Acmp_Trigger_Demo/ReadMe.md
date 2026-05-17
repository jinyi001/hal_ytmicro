# Acmp_Trigger_Demo

---

## Function Description

This example demonstrates the configuration and usage of the SDK ACMP module, as well as self-testing:

1. Rotate the potentiometer to adjust the voltage of Channel 0 to 4V.
2. Rotate the potentiometer to adjust the voltage of Channel 1 to 3V.
3. Configure a trigger to initiate a comparison on both channels after 1 second.
4. The program checks after 0.1 seconds and verifies that the ACMP has not yet performed the channel voltage comparison, as it has not been triggered.
5. The program checks again after 1 second and confirms that the ACMP has received the trigger and completed the channel voltage comparison.

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