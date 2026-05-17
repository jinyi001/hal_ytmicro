# Acmp_Continuous_Loop_Demo

---

## Function Description

This example demonstrates the configuration and usage of the SDK ACMP module, as well as self-testing:

1. Rotate the potentiometer to adjust the voltage of Channel 0 to 4V.
2. Rotate the potentiometer to adjust the voltage of Channel 1 to 3V.
3. The program first sets the internal DAC voltage to 2.5V. At this point, the voltages of both Channel 0 and Channel 1 are higher than the DAC voltage, causing the corresponding red and green lights to turn on.
4. Then, the internal DAC voltage is set to 3.5V. Now, only the voltage of Channel 0 is higher than the DAC voltage, so only the red light turns on.
5. Finally, the internal DAC voltage is set to 4.5V. Since the channel voltages are both lower than the DAC voltage, no lights will turn on.
6. Every time the DAC voltage is modified, the program checks whether the comparison result matches the expected outcome.

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