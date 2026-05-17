# Etmr_Pwm_Demo

---

## Function Description

This routine demonstrates the PWM output and input capture functionality of the eTMR module, along with a self-test procedure:

- Short-circuit the four channel pairs between eTMR0 and eTMR3 to perform self-test
- eTMR0 channels 0, 1, 2, 3 output 5kHz PWM with 12.5%, 25.0%, 37.5%, and 50.0% duty cycles respectively
- eTMR3 channels 0, 1, 2, 3 capture the duty cycle and frequency of eTMR0's four channels, verifying results against configured values
- Update only the duty cycles of eTMR0's four channels to 50.0%, 62.5%, 75.0%, and 87.5% and repeat the above self-test
- Update only the frequency of eTMR0's four channels to 10kHz and repeat the above self-test
- Set eTMR0 channels 0 and 1 to 100% duty cycle
- Set eTMR0 channels 2 and 3 to 0% duty cycle

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