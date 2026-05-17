# Etmr_Pwm_Complementary_Demo

---

## Function Description

This routine demonstrates the complementary PWM output functionality of the eTMR module and performs self-test through input capture.

- Short-circuit the four channel pairs between eTMR0 and eTMR3 for self-testing
- eTMR0_CH0 and eTMR0_CH1 output complementary PWM signals with dead time, featuring 25% and 75% duty cycles respectively
- eTMR0_CH2 and eTMR0_CH3 output complementary PWM signals with dead time, featuring 62.5% and 37.5% duty cycles respectively
- Operating frequency: 10kHz
- Dead time frequency: the fast bus clock frequency
  - Even-numbered channels (e.g., CH0/CH2): Dead time counter period = 8
  - Odd-numbered channels (e.g., CH1/CH3): Dead time counter period = 10
- eTMR3 channels 0, 1, 2, 3 capture the duty cycle and frequency of eTMR0's four channels respectively, verifying results against configured values

---

## EVB Hardware Setup

The following table shows the external pin configuration used in the routine:

|  PIN        |    FUNC       |
|  ------     |    -----      |
|  PTB_12      |    eTMR0_CH0  |
|  PTB_13      |    eTMR0_CH1  |
|  PTB_14      |    eTMR0_CH2  |
|  PTB_15      |    eTMR0_CH3  |
|  PTB_8       |    eTMR3_CH0  |
|  PTB_9       |    eTMR3_CH1  |
|  PTC_6       |    eTMR3_CH2  |
|  PTC_7       |    eTMR3_CH3  |

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